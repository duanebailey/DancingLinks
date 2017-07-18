/*
 * Determine orienation of shikaku pieces for value n in a x b grid.
 * (c) 2010 duane a. bailey
 */
#include <stdio.h>
#include <stdlib.h>

int generate = 0;
int debug = 0;
char *buffer = 0;
int **board = 0;
void Usage(char *progName)
{
  fprintf(stderr,"Usage: %s d0 d1\n",progName);
  fprintf(stderr,"\tPrint flattened orienations of pieces in (d0 x d1) grid.\n");
  fprintf(stderr,"Input:\n n x y\nwhere size n appears at (x,y)\n");
  exit(1);
}

int min(int a, int b)
{
  return a<b?a:b;
}

int max(int a, int b)
{
  return a>b?a:b;
}

int between(int a, int b, int c) {
  return (a <= b) && (b <= c);
}

void printPiece(char c, int x0, int x1, int p0, int p1, int d0, int d1)
{
  int s = d0*d1;
  int i, j;
  int q0 = p0+x0-1;
  int q1 = p1+x1-1;
  char *bp = buffer;
  for (i = 0; i < d0; i++) {
    for (j = 0; j < d1; j++) {
      *bp++ = (between(p0,i,q0) && between(p1,j,q1)) ? c : ' ';
    }
  }
  *bp++ = 0;
  printf("%s\n",buffer);
  return;
}

void orientations(char c, int n, int x0, int x1, int d0, int d1)
{
  int f0; // factor; one side of block
  int f1; // other factor
  int min0, max0, min1, max1, p0, p1;
  for (f0 = 1; f0 <= n; f0++) {
    if (n%f0 == 0) {
      // consider f0 by f1 blocks
      f1 = n/f0;
      min0 = max(x0-f0+1,0);
      max0 = min(x0+f0-1,d0-1)-f0+1;
      min1 = max(x1-f1+1,0);
      max1 = min(x1+f1-1,d1-1)-f1+1;
      for (p0 = min0; p0 <= max0; p0++) {
	for (p1 = min1; p1 <= max1; p1++) { 
	  if (validatePiece(f0,f1,p0,p1,d0,d1) && debug) {
	    printf("%dx%d '%c' @ (%d,%d)\n",f0,f1,c,p0,p1);
	  } else {
	    printPiece(c,f0,f1,p0,p1,d0,d1);
	  }
	}
      }
    }
  }
}

int validatePiece(int f0, int f1, int p0, int p1, int d0, int d1) {
  int row, col;
  int e0 = p0+f0-1;
  int e1 = p1+f1-1;
  int found = 0;
  for (row = p0; row < e0; row++) {
    for (col = p1; col < e1; col++) {
      if (board[row][col] > 0) found++;
    }
  }
  return found == 1;
}

int main(int argc, char **argv)
{
  char *progName = argv[0];
  if (argc > 1 && 0 == strcmp("-d",argv[1])) {
    argc--; argv++;
    debug = 1;
  } else if (argc > 1 && 0 == strcmp("-g",argv[1])) {
    argc--; argv++;
    generate = 1;
  }
  int n;
  int x,y;
  int total = 0;
  int d0 = atoi(argv[1]),d1 = atoi(argv[2]);
  buffer = (char*)malloc(1+d0*d1);
  char line[80];
  char c = '0';
  int row, col;
  board = (int**)malloc(d0*sizeof(int*));
  for (row = 0; row < d0; row++) {
    board[row] = (int*)calloc(d1,sizeof(int));
  }
  for (row = 0; row < d0; row++) {
    if (fgets(line,80,stdin)) {
      for (col = 0; col < d1; col++) {
	if (line[col] < ' ') break;
	if (line[col] == ' ') continue;
	n = line[col];
	if (n >='a') n = n - 'a'+10;
	if (n >= 'A') n = n - 'A'+10;
	if (n >= '0') n = n - '0';
	if (n > 12) fprintf(stderr,"problem at (%d,%d); value too large.\n",
			    row,col);
	total += n;
	board[row][col] = n;
      }
    } else {
      break;
    }
  }
  for (row = 0; row < d0; row++) {
    for (col = 0; col < d1; col++) {
      if (board[row][col] > 0) {
	n = board[row][col];
	if (generate) {
	  printf("%d %d %d\n",n,row,col);
	} else { 
	  orientations(c,n,row,col,d0,d1);	
	}
	if (c == '9') c = 'a';
	else if (c == 'z') c = 'A';
	else c++;
      }
    }
  }
  if (total != d0*d1) {
    fprintf(stderr,"Piece area (%d) does not total grid area (%d).\n",
	    total,d0*d1);
  }
}
