
#include <memory>
#ifndef _SHARED_PTR_H
#include <google/protobuf/stubs/shared_ptr.h>
#endif

#include <google/protobuf/compiler/python/python_generator.h>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/printer.h>

#include <google/protobuf/testing/file.h>
#include <google/protobuf/testing/file.h>
#include <google/protobuf/stubs/strutil.h>
#include <google/protobuf/testing/googletest.h>
#include <gtest/gtest.h>

namespace google {
namespace protobuf {
namespace compiler {
namespace python {
namespace {

class TestGenerator : public CodeGenerator {
 public:
  TestGenerator() {}
  ~TestGenerator() {}

  virtual bool Generate(const FileDescriptor* file,
                        const string& parameter,
                        GeneratorContext* context,
                        string* error) const {
    TryInsert("test_pb2.py", "imports", context);
    TryInsert("test_pb2.py", "module_scope", context);
    TryInsert("test_pb2.py", "class_scope:foo.Bar", context);
    TryInsert("test_pb2.py", "class_scope:foo.Bar.Baz", context);
    return true;
  }

  void TryInsert(const string& filename, const string& insertion_point,
                 GeneratorContext* context) const {
    google::protobuf::scoped_ptr<io::ZeroCopyOutputStream> output(
        context->OpenForInsert(filename, insertion_point));
    io::Printer printer(output.get(), '$');
    printer.Print("// inserted $name$\n", "name", insertion_point);
  }
};

// This test verifies that all the expected insertion points exist.  It does
// not verify that they are correctly-placed; that would require actually
// compiling the output which is a bit more than I care to do for this test.
TEST(PythonPluginTest, PluginTest) {
  GOOGLE_CHECK_OK(File::SetContents(TestTempDir() + "/test.proto",
                             "syntax = \"proto2\";\n"
                             "package foo;\n"
                             "message Bar {\n"
                             "  message Baz {}\n"
                             "}\n",
                             true));

  google::protobuf::compiler::CommandLineInterface cli;
  cli.SetInputsAreProtoPathRelative(true);

  python::Generator python_generator;
  TestGenerator test_generator;
  cli.RegisterGenerator("--python_out", &python_generator, "");
  cli.RegisterGenerator("--test_out", &test_generator, "");

  string proto_path = "-I" + TestTempDir();
  string python_out = "--python_out=" + TestTempDir();
  string test_out = "--test_out=" + TestTempDir();

  const char* argv[] = {
    "protoc",
    proto_path.c_str(),
    python_out.c_str(),
    test_out.c_str(),
    "test.proto"
  };

  EXPECT_EQ(0, cli.Run(5, argv));
}

// This test verifies that the generated Python output uses regular imports (as
// opposed to importlib) in the usual case where the .proto file paths do not
// not contain any Python keywords.
TEST(PythonPluginTest, ImportTest) {
  // Create files test1.proto and test2.proto with the former importing the
  // latter.
  GOOGLE_CHECK_OK(File::SetContents(TestTempDir() + "/test1.proto",
                             "syntax = \"proto3\";\n"
                             "package foo;\n"
                             "import \"test2.proto\";"
                             "message Message1 {\n"
                             "  Message2 message_2 = 1;\n"
                             "}\n",
                             true));
  GOOGLE_CHECK_OK(File::SetContents(TestTempDir() + "/test2.proto",
                             "syntax = \"proto3\";\n"
                             "package foo;\n"
                             "message Message2 {}\n",
                             true));

  google::protobuf::compiler::CommandLineInterface cli;
  cli.SetInputsAreProtoPathRelative(true);
  python::Generator python_generator;
  cli.RegisterGenerator("--python_out", &python_generator, "");
  string proto_path = "-I" + TestTempDir();
  string python_out = "--python_out=" + TestTempDir();
  const char* argv[] = {"protoc", proto_path.c_str(), "-I.", python_out.c_str(),
                        "test1.proto"};
  ASSERT_EQ(0, cli.Run(5, argv));

  // Loop over the lines of the generated code and verify that we find an
  // ordinary Python import but do not find the string "importlib".
  string output;
  GOOGLE_CHECK_OK(File::GetContents(TestTempDir() + "/test1_pb2.py", &output,
                             true));
  std::vector<string> lines = Split(output, "\n");
  string expected_import = "import test2_pb2";
  bool found_expected_import = false;
  for (int i = 0; i < lines.size(); ++i) {
    if (lines[i].find(expected_import) != string::npos) {
      found_expected_import = true;
    }
    EXPECT_EQ(string::npos, lines[i].find("importlib"));
  }
  EXPECT_TRUE(found_expected_import);
}

}  // namespace
}  // namespace python
}  // namespace compiler
}  // namespace protobuf
}  // namespace google