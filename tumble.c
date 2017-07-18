/*
 * Tumble a piece within a box.
 * (c) 2003 duane a. bailey
 *
 * This program takes an object in a larger box and finds all the different
 * orientations of the piece within the box.  The dimensions of the box are
 * provided on the command line.  The shape of the piece is provided 
 * as a flattened input (see flatten), specified on one line.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*
 * Suppose we have a box that is 2x3x4, then
 * dimension = 3
 * size = 24
 * dalloc (actual allocated dimension of arrays) is 3
 * dims  = [2 3 4]
 * mults = [12 4 1]
 * Basically, cell [i j k] = mults[0]*i+mults[1]*j+mults[2]*k in order
 */
int dimension; /* number of dimensions of box */
int size;      /* total number of units in box */
int dalloc;	/* number of dimensions actually alloced to dims and mults */
int *dims;     /* array of dimensions */
int *mults;    /* multipliers for dimension<->index conversion */

void Usage(char *progName)
{
  fprintf(stderr,"Usage: %s [-h] size_0 size_1 ... size_n\n",progName);
  fprintf(stderr,"\t-h\tprint help\n");
  fprintf(stderr,"\tsize_i\tsize of containing box in dimension i\n");
  exit(1);
}

void parseArgs(int argc, char **argv)
{
  char *progName = argv[0];
  int i,j;


    dalloc = 3;		/* who does >3D puzzles? */
    dimension = 0;
    dims = (int *)malloc(dalloc*sizeof(int));
    mults = (int *)malloc(dalloc*sizeof(int));
    while (argv++, --argc) {
      char *arg = *argv;
      if (arg[0] == '-') {
	while (*++arg) {
	  switch (*arg) {
	  default:
	    fprintf(stderr,"Don't understand option -%c\n",*arg);
	  case 'h':
	    Usage(progName);
	    break;
	    /* add other options here: */
	  }
	}
      } else {
	if ((dimension+1) > dalloc) {
	  dalloc *= 2;
	  dims = (int*)realloc(dims,dalloc*sizeof(int));
	  mults = (int*)realloc(mults,dalloc*sizeof(int));
	}
	dims[dimension] = atoi(arg);
	mults[dimension] = 1;
	for (j = 0; j < dimension; j++)
	  {	
	    mults[j] = mults[j]*dims[dimension];
	  }
	dimension++;
      }
    }
    size = mults[0] * dims[0];
}

/*
 * Read in a flattened piece into buffer, padding to size with spaces, no nl.
 */
char *readPiece(FILE *f, char *buffer,int size)
{
    int l;
    // attempt read
    if (fgets(buffer,BUFSIZ,f) == 0) {
      return 0;
    }
    // trim
    l = strlen(buffer);
    if ((l != 0) && (buffer[l-1] == '\n')) l--;
    while (l < size)
    {
	buffer[l] = ' ';
	l++;
    }
    buffer[size] = '\0';
    return buffer;
}


/*
 * Take a piece and move it to its canonical position:
 * abutting each 0-index in each dimension
 */
void justify(char *buffer)
{
    int *corner;
    int i,j,k,kk,offset;

    if (*buffer) return;
    // First, find the lower bound index of the piece
    corner = (int*)calloc(dimension,sizeof(int));
    for (i = 0; i < dimension; i++)
    {
	corner[i] = dims[i];
    }
    // for each piece character, check against minimum "corner" indicies
    for (i = 0; i < size; i++)
    {
	if (!isspace(buffer[i]))
	{
	    k = i;
	    for (j = 0; j < dimension; j++)
	    {
		kk = k/mults[j];
		if (kk < corner[j]) corner[j] = kk;
		k = k%mults[j];
	    }
	}
    }	

    // compute translation to corner [0,0,...,0]: offset
    offset = 0;
    for (i = 0; i < dimension; i++)
    {
	offset += corner[i]*mults[i];
    }
    // non-zero offsets requires a shift; fill with spaces
    char *in = buffer+offset;
    char *out = buffer;
    char *end = buffer+size;
    while ((*out++ = *in++));
    while (out < end) *out++ = ' ';
}

/*
 * Determine the dimensions of the convex hull of a piece.
 * Returns a vector of the hull edge lengths.
 */
int *dimensions(char *buffer)
{
    int *result = (int*)calloc(dimension,sizeof(int));
    int i, j, k, kk;

    for (i = 0; i < size; i++)
    {
	if (!isspace(buffer[i]))
	{
	    k = i;
	    for (j = 0; j < dimension; j++)
	    {
		kk = k/mults[j];
		if (kk >= result[j]) result[j] = kk+1;
		k = k%mults[j];
	    }
	}
    }
    return result;
}

/*
 * Construct a vector of shapes that contains all the translations of a
 * piece within the containing box.
 * Sufficiently large vector should be passed to routine; individual pieces
 * are allocated.  Return value is number of elements of vector used.
 */
int translates(char *buffer, char **results)
{
    int i, j, k;
    int *shape = dimensions(buffer);
    int count = 0;

    for (i = 0; i < size; i++)
    {
	k = i;
	// check to see if an offset of i would cause the shape to
	// exceed the box size in any dimension
	for (j = 0; j < dimension; j++)
	{
	    if ((k/mults[j])+shape[j] > dims[j]) break;
	    k = k%mults[j];
	}
	// if j < dimension; piece shift by i would poke through dimension j
	if (j == dimension)
	{
	    count++;
	    // we use calloc to terminate string
	    char *rp;
	    *results++ = rp = (char*)calloc(size+1,1);
	    for (k = 0; k < size; k++)
	    {
		rp[k] = (k < i)?' ':buffer[k-i];
	    }
	}
    }
    return count;
}

/*
 * (Destructively) rotate the piece so that dimension dim1 becomes dim2, and
 * dim2 becomes -dim1
 */
int rotate(char *piece, int dim1, int dim2)
{
    int *shape = dimensions(piece);
    int i,j,k;
    int *idx;
    char *copy;
    if (shape[dim1] > dims[dim2] ||
	shape[dim2] > dims[dim1]) return 0;
    copy = strdup(piece);
    idx = (int*)calloc(dimension,sizeof(int));
    for (i = 0; i < size; i++) piece[i] = ' ';
    for (i = 0; i < size; i++)
    {
	if (!isspace(copy[i]))
	{
	    k = i;
	    for (j = 0; j < dimension; j++)
	    {
		idx[j] = k/mults[j];
		k %= mults[j];
	    }
	    j = i - idx[dim1]*mults[dim1] + idx[dim1]*mults[dim2]
		- idx[dim2]*mults[dim2] + (shape[dim2]-idx[dim2]-1)*mults[dim1];
	    piece[j] = copy[i];
	}
    }
    free(idx);
    free(copy);
    return 1;
}

/*
 * (Destructively) rotate the piece so that dimension dim1 becomes -dim1, and
 * dim2 becomes -dim2
 */
int rotate180(char *piece, int dim1,int dim2)
{
    /* rotate the piece so that dimension dim1 becomes -dim1, and
     * dim2 becomes -dim2
     */
    int *shape = dimensions(piece);
    int i,j,k;
    int *idx;
    char *copy;
    copy = strdup(piece);
    idx = (int*)calloc(dimension,sizeof(int));
    for (i = 0; i < size; i++) piece[i] = ' ';
    for (i = 0; i < size; i++)
    {
	if (!isspace(copy[i]))
	{
	    k = i;
	    for (j = 0; j < dimension; j++)
	    {
		idx[j] = k/mults[j];
		k %= mults[j];
	    }
	    j = i + (shape[dim1]-2*idx[dim1]-1)*mults[dim1]
		  + (shape[dim2]-2*idx[dim2]-1)*mults[dim2];
	    piece[j] = copy[i];
	}
    }
    free(idx);
    free(copy);
    return 1;
}

/*
 * Perform uniq-style reduction of vector, s, of n strings.  Return number
 * of uniq values
 */
int uniq(char **s, int n)
{
    int i,j;
    for (i = 0; i < n; i++)
    {
	if (s[i] == 0) continue;
	for (j = i+1; j < n; j++)
	{
	    if (s[j] == 0) continue;
	    if (strcmp(s[i],s[j]) == 0) { free(s[j]); s[j] = 0; }
	}
    }
    for (i = j = 0; j < n; j++)
    {
	if (s[j]) { if (i != j) { s[i] = s[j]; s[j] = 0;} i++; } 
    }
    return i;
}

/*
 * Perform (up to 3 dimensional) all oritientations of pieces
 */
int reorient(char *piece, char **orientations)
{
    char **os = orientations;
    int count, total, i,j,k;
    total = 0;

    count = translates(piece,orientations);
    orientations += count;
    total += count;
    if (dimension == 1)
    {
	count = translates(piece,orientations);
	orientations += count;
	total += count;
	rotate180(piece,0,1);
	count = translates(piece,orientations);
	orientations += count;
	total += count;
	rotate180(piece,0,1);
    } else if (dimension == 2)
    {
	for (i = 0; i < 4; i++)
	{
	    count = translates(piece,orientations);
	    orientations += count;
	    total += count;
	    if (!rotate(piece,0,1)) { rotate180(piece,0,1); i++; }
	}
    } else if (dimension == 3)
    {
	for (i = 0; i < 4; i++)
	{
	    for (j = 0; j < 4; j++)
	    {
		for (k = 0; k < 4; k++)
		{
		    count = translates(piece,orientations);
		    orientations += count;
		    total += count;
		    if (!rotate(piece,0,1)) { rotate180(piece,0,1); k++; }
		}
		if (!rotate(piece,1,2)) { rotate180(piece,0,1); j++; }
	    }
	    if (!rotate(piece,0,2)) { rotate180(piece,0,1); i++; }
	}
    }
    return uniq(os,total);
}

int main(int argc, char **argv)
{
    char *piece;
    char **orientations = (char **)calloc(10000,sizeof(char*));
    int total;
    FILE *f = stdin;
    int i;

    parseArgs(argc,argv);
    piece = (char*)malloc(BUFSIZ);
    while (readPiece(f,piece,size))
    {
	justify(piece);
	total = reorient(piece, orientations);
	for (i = 0; i < total; i++)
	{
	    printf("%s\n",orientations[i]);
	}
    }
    return 0;
}
