#include "stdafx.h"

// Compression LZSS , stealed from internet. 

char *CompressionName = "Compressor LZSS by unknown author";
//char *Usage = "[p]ack|[u]npack input_file output_file\n\n";


void fatal_error ( char *str, ...)
{
	deb("lzss fatal error: %s", str);
}


BFILE *OpenOutputBFILE ( char *name)
{
	BFILE *bfile;
	
	bfile = (BFILE *) calloc ( 1, sizeof (BFILE));
	bfile -> file = fopen ( name, "wb");
	bfile -> rack = 0;
	bfile -> mask = 0x80;
	bfile -> pacifier_counter = 0;
	return bfile;
}

BFILE *OpenInputBFile ( char *name)
{
	BFILE *bfile;
	
	bfile = (BFILE *) calloc ( 1, sizeof (BFILE));
	bfile -> file = fopen ( name, "rb");
	bfile -> rack = 0;
	bfile -> mask = 0x80;
	bfile -> pacifier_counter = 0;
	return bfile;
}

void CloseOutputBFile ( BFILE *bfile)
{
	if ( bfile -> mask != 0x80 )
		putc  ( bfile -> rack, bfile -> file);
	fclose ( bfile -> file);
	free ((char *) bfile);
}


void CloseInputBFile ( BFILE *bfile)
{
	fclose ( bfile -> file);
	free ((char *) bfile);
}

void WriteBit ( BFILE *bfile, int bit)
{
	if (bit)
		bfile -> rack |= bfile -> mask;
	bfile -> mask >>= 1;
	if (bfile -> mask == 0)
	{
		putc ( bfile -> rack, bfile -> file);
		if ( (bfile -> pacifier_counter++ & PACIFIER_COUNT) == 0)
			putc ( '.', stdout);
		bfile -> rack = 0;
		bfile -> mask = 0x80;
	}
}

void WriteBits ( BFILE *bfile, ulong code, int count)
{
	ulong mask;
	
	mask = 1L << (count - 1);
	while (mask != 0)
	{
		if (mask & code)
			bfile -> rack |= bfile -> mask;
		bfile -> mask >>= 1;
		if (bfile -> mask == 0)
		{
			putc ( bfile -> rack, bfile -> file);
			if ( (bfile -> pacifier_counter++ & PACIFIER_COUNT) == 0)
				putc ( '.', stdout);
			bfile -> rack = 0;
			bfile -> mask = 0x80;
		}
		mask >>= 1;
	}
}

int ReadBit ( BFILE *bfile)
{
	int value;
	
	if (bfile -> mask == 0x80)
	{
		bfile -> rack = getc ( bfile -> file);
		if (bfile -> rack == EOF)
		{
			fatal_error ("Error in ReadBit!\n");
			return -1;
		}
		if ( (bfile -> pacifier_counter++ & PACIFIER_COUNT) == 0 )
			putc ( '.', stdout);
	}
	value = bfile -> rack & bfile -> mask;
	bfile -> mask >>= 1;
	if (bfile -> mask == 0)
		bfile -> mask = 0x80;
	return ( value ? 1 : 0);
}


ulong ReadBits ( BFILE *bfile, int bit_count)
{
	ulong mask;
	ulong return_value;
	
	mask = 1L << (bit_count - 1);
	return_value = 0;
	while (mask != 0)
	{
		if (bfile -> mask == 0x80)
		{
			bfile -> rack = getc (bfile -> file);
			if (bfile -> rack == EOF)
			{
				fatal_error ("Error in ReadBits!\n");
				return 0;
			}
			if ( (bfile -> pacifier_counter++ & PACIFIER_COUNT) == 0 )
				putc ( '.', stdout);
		}
		if (bfile -> rack & bfile -> mask)
			return_value |= mask;
		mask >>= 1;
		bfile -> mask >>= 1;
		if (bfile -> mask == 0)
			bfile -> mask = 0x80;
	}
	return return_value;
}
/*
int main ( int argc, char *argv [])
{	
	setbuf ( stdout, NULL);
	if (argc < 4)
		usage_exit (argv [0]);
	if(strcmp(argv[1], "p") == 0)
	{
		BFILE *output;
		FILE  *input;
		input = fopen ( argv [2], "rb");
		if (input == NULL)
			fatal_error("Error %s for output\n", argv [2]);
		output = OpenOutputBFILE(argv [3]);
		if (output == NULL)
			fatal_error ( "Error while opening %s to\n", argv [3]);
		printf ( "\nCompressing %s to %s\n", argv [2], argv [3]);
		printf ( "Using %s\n", CompressionName);
		CompressFile ( input, output);
		CloseOutputBFile (output);
		fclose (input);
		print_ratios ( argv [2], argv [3]);
	}
	else
	{
		FILE *output;
		BFILE  *input;
		input = OpenInputBFile(argv [2]);
		if (input == NULL)
			fatal_error("Error %s for output\n", argv [2]);
		output = fopen (argv [3], "w+b");
		if (output == NULL)
			fatal_error ( "Error while opening %s to\n", argv [3]);
		printf ( "\nDecompressing %s to %s\n", argv[2], argv [3]);
		printf ( "Using %s\n", CompressionName);
		ExpandFile ( input, output);
		fclose(output);
		CloseOutputBFile(input);
		print_ratios ( argv [2], argv [3]);
	}
	return 0;
}

*/

long file_size ( char *name)
{
	long eof_ftell;
	FILE *file;
	
	file = fopen ( name, "r");
	if (file == NULL)
		return (0L);
	fseek ( file, 0L, SEEK_END);
	eof_ftell = ftell (file);
	fclose (file);
	return eof_ftell;
}


void print_ratios ( char *input, char *output)
{
	long input_size;
	long output_size;
	int ratio;
	
	input_size = file_size (input);
	if (input_size == 0)
		input_size = 1;
	output_size = file_size (output);
	ratio = (int) (output_size * 100L / input_size);
	deb ("\nlzss: Size of input file (bytes): %ld\n",
		input_size);
	deb ("lzss: Size of output file (bytes): %ld\n",
		output_size);
	if (output_size == 0)
		output_size = 1;
	printf ("lzss: Compression rate: %d%%\n", ratio);
}

uchar window [WINDOW_SIZE];

struct
{
	int parent;
	int smaller_child;
	int larger_child;
}
tree [WINDOW_SIZE + 1];

void InitTree ( int r)
{
	tree [TREE_ROOT].larger_child = r;
	tree [r].parent               = TREE_ROOT;
	tree [r].larger_child		= UNUSED;
	tree [r].smaller_child	= UNUSED;
}

void ContractNode ( int old_node, int new_node)
{
	tree [new_node] .parent = tree [old_node].parent;
	if (tree [tree [old_node].parent].larger_child == old_node)
		tree [tree [old_node].parent].larger_child = new_node;
	else
		tree [tree [old_node].parent].smaller_child = new_node;
	tree [old_node].parent = UNUSED;
}


void ReplaceNode ( int old_node, int new_node)
{
	int parent;
	
	parent = tree[old_node].parent;
	if (tree[parent].smaller_child == old_node)
		tree[parent].smaller_child = new_node;
	else
		tree[parent].larger_child = new_node;
	tree[new_node] = tree[old_node];
	tree[tree [new_node].smaller_child].parent = new_node;
	tree[tree [new_node].larger_child].parent = new_node;
	tree[old_node].parent = UNUSED;
}

int FindNextNode ( int node)
{
	int next;
	
	next = tree[node].smaller_child;
	while (tree[next].larger_child != UNUSED)
		next = tree[next].larger_child;
	return next;
}


void DeleteString ( int p)
{
	int replacement;
	
	if (tree [p].parent == UNUSED)
		return;
	if (tree [p].larger_child == UNUSED)
		ContractNode ( p, tree [p].smaller_child);
	else
	{
		if (tree [p].smaller_child == UNUSED)
			ContractNode ( p, tree [p].larger_child);
		else
		{
			replacement = FindNextNode (p);
			DeleteString (replacement);
			ReplaceNode ( p, replacement);
		}
	}
}

int AddString ( int new_node, int *match_pos)
{
	int i;
	int test_node;
	int delta;
	int match_len;
	int *child;
	
	if (new_node == END_OF_STREAM)
		return (0);
	
	test_node = tree [TREE_ROOT].larger_child;
	match_len = 0;
	
	for (;;)
	{
		for ( i = 0; i < LOOK_AHEAD_SIZE; i++)
		{
			delta = window [MODULO (new_node + i)] -
				window [MODULO (test_node + i)];
			if (delta != 0)
				break;
		}
		
		if (i >= match_len)
		{
			match_len = i;
			*match_pos = test_node;
			if (match_len >= LOOK_AHEAD_SIZE)
			{
				ReplaceNode ( test_node, new_node);
				return match_len;
			}
		}
		
		if (delta >= 0)
			child = &tree [test_node].larger_child;
		else
			child = &tree [test_node].smaller_child;
		
		if (*child == UNUSED)
		{
			*child = new_node;
			tree [new_node].parent = test_node;
			tree [new_node].larger_child = UNUSED;
			tree [new_node].smaller_child = UNUSED;
			return match_len;
		}
		test_node = *child;
	}
}

void CompressFile ( FILE *input, BFILE *output)
{
	int i;
	int c;
	int look_ahead_bytes;
	int current_pos;
	int replace_count;
	int match_len;
	int match_pos;
	
	current_pos = 1;
	
	for ( i = 0; i < LOOK_AHEAD_SIZE; i++)
	{
		if ( (c = getc (input)) == EOF )
			break;
		window [current_pos + i] = (uchar) c;
	}
	
	look_ahead_bytes = i;
	InitTree (current_pos);
	match_len = 0;
	match_pos = 0;
	
	while (look_ahead_bytes > 0)
	{
		if (match_len > look_ahead_bytes)
			match_len = look_ahead_bytes;
		if (match_len <= BREAK_EVEN)
		{
			replace_count = 1;
			WriteBit ( output, 1);
			WriteBits ( output, (ulong) window [current_pos], 8);
		}
		else
		{
			WriteBit  ( output, 0);
			WriteBits ( output, (ulong) match_pos, INDEX_BITS);
			WriteBits ( output, (ulong) ( match_len - (BREAK_EVEN + 1)),
				LENGTH_BITS);
			replace_count = match_len;
		}
		
		for ( i = 0; i < replace_count; i++)
		{
			DeleteString ( MODULO (current_pos + LOOK_AHEAD_SIZE));
			if ( (c = getc (input)) == EOF )
				look_ahead_bytes--;
			else
				window [MODULO (current_pos + LOOK_AHEAD_SIZE)] = (uchar) c;
			current_pos = MODULO (current_pos + 1);
			if (look_ahead_bytes)
				match_len = AddString ( current_pos, &match_pos);
		}
	}
	
	WriteBit  ( output, 0);
	WriteBits ( output, (ulong) END_OF_STREAM, INDEX_BITS);
}

void ExpandFile ( BFILE *input, FILE *output)
{
	int i;
	int current_pos;
	int c;
	int match_len;
	int match_pos;
	
	current_pos = 1;
	
	for (;;)
	{
		if (ReadBit (input))
		{
			c = (int) ReadBits ( input, 8);
			putc ( c, output);
			window [current_pos] = (uchar) c;
			current_pos = MODULO (current_pos + 1);
		}
		else
		{
			match_pos = (int) ReadBits ( input, INDEX_BITS);
			if (match_pos == END_OF_STREAM)
				break;
			match_len = (int) ReadBits ( input, LENGTH_BITS);
			match_len += BREAK_EVEN;
			for ( i = 0; i <= match_len; i++)
			{
				c = window [MODULO (match_pos + i)];
				putc ( c, output);
				window [current_pos] = (uchar) c;
				current_pos = MODULO (current_pos + 1);
			}
		}
	}
}
