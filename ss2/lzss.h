typedef unsigned char uchar;
typedef unsigned long ulong;

typedef struct bfile
{
	FILE *file;
	uchar mask;
	int rack;
	int pacifier_counter;
} BFILE;

BFILE *OpenInputBFile ( char *name);
BFILE *OpenOutputBFile ( char *name);
void  WriteBit ( BFILE *bfile, int bit);
void  WriteBits ( BFILE *bfile, ulong code, int count);
int   ReadBit ( BFILE *bfile);
ulong ReadBits ( BFILE *bfile, int bit_count);
void  CloseInputBFile ( BFILE *bfile);
void  CloseOutputBFile ( BFILE *bfile);


void CompressFile ( FILE *input, BFILE *output);
void ExpandFile ( BFILE *input, FILE *output);
void usage_exit ( char *prog_name);
void print_ratios ( char *input, char *output);
long file_size ( char *name);


void InitTree ( int r);
void ContractNode ( int old_node, int new_node);
void ReplaceNode ( int old_node, int new_node);
int  FindNextNode ( int node);
void DeleteString ( int p);
int  AddString ( int new_node, int *match_position);



#define PACIFIER_COUNT 2047

#define INDEX_BITS  12
#define LENGTH_BITS  4
#define WINDOW_SIZE  ( 1 << INDEX_BITS)  //4096 i?e INDEX_BITS = 12
#define RAW_LOOK_AHEAD_SIZE  ( 1 << LENGTH_BITS)
#define BREAK_EVEN  (( 1 + INDEX_BITS + LENGTH_BITS) / 9)
#define LOOK_AHEAD_SIZE  ( RAW_LOOK_AHEAD_SIZE + BREAK_EVEN)
#define TREE_ROOT  WINDOW_SIZE
#define END_OF_STREAM  0
#define UNUSED  0
#define MODULO(a)  ( (a) & (WINDOW_SIZE - 1) )
