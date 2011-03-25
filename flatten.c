/* -*- compile-command: "gcc -g -o flatten flatten.c" -*- */
#include <stdio.h>
#include <stdlib.h>

void parseArgs(int argc, char **argv);
#define MAX_DIM  10
int dimensions[MAX_DIM];
int mults[MAX_DIM];
int dimension = 0;
int size = 0;
char *ProgName;

void usage()
{
    fprintf(stderr,"Usage: %s [-h] dimension-list\n",ProgName);
    fprintf(stderr,"\t-h\tprint this message\n");
    fprintf(stderr,"\tdimension-list\n\t\ta list of dimensions of input object\n");
    exit(1);
}

void parseArgs(int argc, char **argv)
{
    int i,n;
    ProgName = *argv++; argc--;
    while (argc)
    {
	char *arg = *argv++;
	if (*arg == '-')
	{
	    arg++;
	    switch (*arg)
	    {
	      case 'h': usage(); break;
	    }
	} else {
	    mults[dimension] = 1;
	    dimensions[dimension] = n = atoi(arg);
	    for (i = 0; i < dimension; i++) mults[i] *= n;
	    dimension++;
	}
	argc--;
    }
    if (dimension == 0)
    {
	usage();
    }
    size = mults[0]*dimensions[0];
}

int idx2off(int idx[])
{
    int i,offset = 0;
    for (i = 0; i < dimension; i++)
    {
	offset += mults[i]*idx[i];
    }
    return offset;
}

void off2idx(int offset, int idx[])
{
    int i;
    for (i = 0; i < dimension; i++)
    {
	idx[i] = offset/mults[i];
	offset %= mults[i];
    }
}

char *readPiece()
{
    char *p = (char*)calloc(size+1,1);
    char c;
    int i;
    static int *idx = 0;
    int offset = 0;
    int curdim = 0;
    if (idx == 0) idx = (int*)calloc(dimension,sizeof(int));
    off2idx(offset,idx);

    for (i = 0; i < size; i++)
    {
	p[i] = ' ';
    }
    while (EOF != (c = fgetc(stdin)))
    {
	if (c == '.') {
	    do { c = fgetc(stdin); }
	    while (c != EOF && (c != '\n' && isspace(c)));
	    if (!isspace(c) && c != EOF) ungetc(c,stdin);
	    break;
	}
	if (c != '\n') {
	    if (curdim > 0)
	    {
		for (i = 0; i < curdim; i++)
		{
		    idx[dimension-1-i] = 0;
		}
		idx[dimension-1-curdim]++;
		curdim = 0;
		offset = idx2off(idx);
	    }
	    p[offset] = c;
	    offset++;
	}
	else if (c == '\n') {
	    curdim++;
	}
    }
    if (offset == 0) return 0;
    return p;
}

void writePiece(char *p)
{
    printf("%s\n",p);
}
int main(int argc, char **argv)
{
    char *p;
    parseArgs(argc, argv);
    while (p = readPiece()) writePiece(p);
    return 0;
}
