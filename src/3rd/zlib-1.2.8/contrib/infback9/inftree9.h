
typedef struct {
    unsigned char op;           /* operation, extra bits, table bits */
    unsigned char bits;         /* bits in this part of the code */
    unsigned short val;         /* offset in table or code value */
} code;

/* op values as set by inflate_table():
    00000000 - literal
    0000tttt - table link, tttt != 0 is the number of table index bits
    100eeeee - length or distance, eeee is the number of extra bits
    01100000 - end of block
    01000000 - invalid code
 */

/* Maximum size of the dynamic table.  The maximum number of code structures is
   1446, which is the sum of 852 for literal/length codes and 594 for distance
   codes.  These values were found by exhaustive searches using the program
   examples/enough.c found in the zlib distribtution.  The arguments to that
   program are the number of symbols, the initial root table size, and the
   maximum bit length of a code.  "enough 286 9 15" for literal/length codes
   returns returns 852, and "enough 32 6 15" for distance codes returns 594.
   The initial root table size (9 or 6) is found in the fifth argument of the
   inflate_table() calls in infback9.c.  If the root table size is changed,
   then these maximum sizes would be need to be recalculated and updated. */
#define ENOUGH_LENS 852
#define ENOUGH_DISTS 594
#define ENOUGH (ENOUGH_LENS+ENOUGH_DISTS)

/* Type of code to build for inflate_table9() */
typedef enum {
    CODES,
    LENS,
    DISTS
} codetype;

extern int inflate_table9 OF((codetype type, unsigned short FAR *lens,
                             unsigned codes, code FAR * FAR *table,
                             unsigned FAR *bits, unsigned short FAR *work));