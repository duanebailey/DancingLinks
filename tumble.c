/*
 * Tumble a piece within a box.
 * (c) 2003 duane a. bailey
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

int dimension; /* number of dimensions of box */
int size;      /* total number of units in box */
int *dims;     /* array of dimensions */
int *mults;    /* multipliers for dimension<->index conversion */

void parseArgs(int argc, char **argv)
{
    int i,j;

    dimension = argc-1;
    dims = (int *)calloc(dimension,sizeof(int));
    mults = (int *)calloc(dimension,sizeof(int));
    for (i = 0; i < dimension; i++)
    {
	dims[i] = atoi(argv[i+1]);
	mults[i] = 1;
	for (j = 0; j < i; j++)
	{
	    mults[j] = mults[j]*dims[i];
	}
    }
    size = mults[0] * dims[0];
}

char *readPiece(FILE *f, char *buffer,int size)
{
    int l;
    if (fgets(buffer,BUFSIZ,f) == 0) return 0;
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

void justify(char *buffer)
{
    int *corner;
    int i,j,k,kk,offset;

    corner = (int*)calloc(dimension,sizeof(int));
    for (i = 0; i < dimension; i++)
    {
	corner[i] = dims[i];
    }
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

    offset = 0;
    for (i = 0; i < dimension; i++)
    {
	offset += corner[i]*mults[i];
    }
    if (offset != 0)
    {
	for (i = 0; i < size; i++)
	{
	    if (i+offset < size)
	    {
		buffer[i] = buffer[i+offset];
	    } else {
		buffer[i] = ' ';
	    }
	}
    }
}

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

int translates(char *buffer, char **results)
{
    int i, j, k;
    int *shape = dimensions(buffer);
    int count = 0;

    for (i = 0; i < size; i++)
    {
	k = i;
	for (j = 0; j < dimension; j++)
	{
	    if ((k/mults[j])+shape[j] > dims[j]) break;
	    k = k%mults[j];
	}
	if (j == dimension)
	{
	    count++;
	    *results = (char*)calloc(size+1,1);
	    for (k = 0; k < size; k++)
	    {
		results[0][k] = (k < i)?' ':buffer[k-i];
	    }
	    results++;
	}
    }
    return count;
}

int rotate(char *piece, int dim1, int dim2)
{
    /* rotate the piece so that dimension dim1 becomes dim2, and
     * dim2 becomes -dim1
     */
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

int dorotates(char *piece, char **orientations)
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
	total = dorotates(piece, orientations);
	for (i = 0; i < total; i++)
	{
	    printf("%s\n",orientations[i]);
	}
    }
    return 0;
}
