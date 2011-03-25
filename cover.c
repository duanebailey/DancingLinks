/* -*- compile-command: "gcc -O3 -o cover cover.c" -*-
 * This program takes selected lines from the input, and generates
 * a single line composed of the characters of several lines of input,
 * all superimposed.  Two input lines cannot be merged if they both
 * contain a non-whitespace character in some column.
 *
 * (c) 2003 duane a. bailey
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/time.h>
#include <signal.h>
#include <assert.h>

#define NONZERO(c) if ((c) == 0) { perror(""); exit(1); }
int wantsUpdate = 0;
struct timeval start[1];
int verbose = 0;
int sheuristic = 0;
int depth = -1;
int maxDepth = -1;
int solutions = 0;
int *covers;

char *ProgName = "cover";

typedef struct obj
{
    struct obj *left,*right,*up,*down,*top;
    struct obj **output;
    int size;
    int width;
    int name;
    char *line;
    char data;
} obj;

typedef struct cell
{
    char *data;
    struct cell *prior;
} cell;

obj *head;

void verify(obj *head);
obj *readProblem(FILE *f);
void printsol(FILE *f, obj *head);
void cover(obj*c);
void uncover(obj*c);
void stats();
void search(int k, obj *head);
void printProblem(FILE *f, obj *head);
void siginfo(int i);
void sigint(int i);
char *curtime();
long runtime();
void usage();
void parseArgs(int argc, char **argv);
int main(int argc, char **argv);

int maxLineSize = 1024; /* maximum number of characters on input buffer */

obj *head;

obj *readProblem(FILE *f)
{
    static char *buffer;
    cell *lst = 0;
    cell *c;
    int l;
    int count = 0;
    int maxlen = 0;
    int i;
    char *bp;
    obj *oldp,*firstp,*temp,*p;

    if (buffer == 0) buffer = (char *)calloc(maxLineSize,1);
    NONZERO(buffer);
    while (fgets(buffer,maxLineSize,f))
    {
	l = strlen(buffer);
	if (l != 0 && buffer[l-1] == '\n') { buffer[l-1] = '\0'; l--; }
	if (l > maxlen) maxlen = l;
	c = (cell*)calloc(1,sizeof(cell));
	NONZERO(c);
	c->data = strdup(buffer);
	NONZERO(c->data);
	c->prior = lst;
	lst = c;
	count++;
    }
    /* at this point, all of the data has been read in, we'll
     * transfer it to an array of strings.
     */
    head = (obj*)calloc(1,sizeof(obj));
    NONZERO(head);
    head->size = head->width = maxlen;
    head->right = (obj*)calloc(maxlen,sizeof(obj));
    NONZERO(head->right);
    head->output = (obj**)calloc(maxlen,sizeof(obj*));
    NONZERO(head->output);
    covers = (int*)calloc(maxlen+10,sizeof(int));
    NONZERO(covers);
    /* build the headers for each of the columns */
    oldp = head;
    for (i = 0; i < maxlen; i++)
    {
	p = head->right+i;
	p->left = oldp;
	p->name = i;
	p->down = p->up = p;
	p->top = p;
	oldp->right = p;
	oldp = p;
    }
    oldp->right = head;  /* wraparound connections */
    head->left = oldp;

    /* now add tabular data */
    for (i = count-1; i >= 0; i--)
    {
	/* bp is the buffer pointer, points to characters in line */
	bp = lst->data;
	/* p is the pointer to the column header */
	p = head->right;
	firstp = 0;
	/* zip across line */
	while (*bp)
	{
	    if (!isspace(*bp))
	    {
		/* add a cell in current row */
		temp = (obj*)calloc(1,sizeof(obj));
		NONZERO(temp);
		temp->size = bp-(lst->data);
		temp->top = p;			/* point to column header */
		temp->top->size++;		/* count as part of column */
		temp->up = p;			/* make it top */
		temp->down = p->down;		/* link item below */
		temp->up->down = temp;		/* make others point to me */
		temp->down->up = temp;
		temp->line = lst->data;		/* point to orig data */
		if (firstp == 0)		/* first in line? */
		{   /* circular connect */
		    firstp = temp->left = temp->right = temp;
		} else {
		    temp->right = firstp;	/* point to first */
		    temp->left = firstp->left;  /*    and last */
		    temp->left->right = temp;
		    temp->right->left = temp;
		}
		temp->data = *bp;
	    }
	    /* move to next column */
	    bp++;
	    p = p->right;
	}

	/* deallocate the linked list element */
	c = lst;
	lst = lst->prior;
	c->data = 0; c->prior = 0;
	free(c);
    }
    return head;
}

void printsol(FILE *f, obj *head)
{
    int i,j,l;
    obj *o,**output;
    char *buffer = (char*)calloc(1+head->width,1);
    NONZERO(buffer);
    if (verbose) {
	fprintf(stderr,"solution %d: ",++solutions);
	fflush(stderr);
    }
    output = head->output;
    for (i = 0; i < head->width; i++) buffer[i] = '?';
    buffer[head->width] = '\0';
    for (i = 0; i < head->width; i++)
    {
	o = output[i];
	if (o == 0) break;
	l = strlen(o->line);
	for (j = 0; j < l; j++)
	{
	    if (!isspace(o->line[j]))
	    {
		buffer[j] = o->line[j];
	    }
	}
    }
    printf("%s\n",buffer);
    if (verbose) stats();
}

void stats()
{
    int total = 0;
    int i;
    fprintf(stderr,"%s: %d covers found, maximum depth reached: %d\n",
	    curtime(),
	    solutions,maxDepth);
    fprintf(stderr,"depth     attempts\n");
    fprintf(stderr,"-----     --------\n");
    for (i = 0; i < maxDepth; i++)
    {
	fprintf(stderr,"%5d %12d\n",i+1,covers[i]);
	total += covers[i];
    }
    fprintf(stderr,"-----     --------\n");
    fprintf(stderr,"total %12d (%.2f tests/sec)\n",total,total/(runtime()/1000.0));
}

void search(int k,obj *head)
{
    obj *col,*row,*ent;
    if (wantsUpdate)
    {
	stats();
	wantsUpdate = 0;
    }
    if (head->right == head) {
	printsol(stdout,head);
	return;
    }
    depth++;
    if (depth > maxDepth)
    {
	maxDepth=depth;
    }
    /* choose a column: */
    if (sheuristic)
    {
	obj *mincol = head->right;
	col = mincol->right;
	while (col != head)
	{
	    if (col->size < mincol->size) {
		mincol = col;
	    }
	    col = col->right;
	}
	col = mincol;
    } else col = head->right;
    if (depth > head->width+1) stats();
    covers[depth]++;
    cover(col);
    for (row = col->down; col != row; row = row->down)
    {
	head->output[k] = row;
	for (ent = row->right; ent != row; ent = ent->right)
	{
	    cover(ent->top);
	}
	search(k+1,head);
	assert(row == head->output[k]);
	assert(col == row->top);
	/*row = head->output[k]; col = row->top;*/
	for (ent = row->left; row != ent; ent = ent->left)
	{
	    uncover(ent->top);
	}
    }
    uncover(col);
    depth--;
    return;
}

void cover(obj *col)
{
    obj *row,*ent;
    head->size--;
    col->right->left = col->left;
    col->left->right = col->right;
    for (row = col->down; row != col; row = row->down)
    {
	for (ent = row->right; ent != row; ent = ent->right)
	{
	    ent->down->up = ent->up;
	    ent->up->down = ent->down;
	    ent->top->size--;
	}
    }
}

void uncover(obj *col)
{
    obj *row, *ent;
    head->size++;
    for (row = col->up; row != col; row = row->up)
    {
	for (ent = row->left; ent != row; ent = ent->left)
	{
	    ent->top->size++;
	    ent->down->up = ent;
	    ent->up->down = ent;
	}
    }
    col->right->left = col;
    col->left->right = col;
}

void printProblem(FILE *f, obj *head)
{
    int i,j;
    obj *q;
    fprintf(f,"Problem has maximum width %d.\n",
	    head->width);
    for (i = 0; i < head->width; i++)
    {
	fprintf(f," column %d has %d entries: ",
		i,head->right[i].size);
	q = head->right[i].down;
	for (j = 0; j < head->right[i].size; j++)
	{
	    fprintf(f," [%c]",q->data);
	    q = q->down;
	}
	fputc('\n',f);
    }
}

void siginfo(int i)
{
    wantsUpdate = 1;
}

void sigint(int i)
{
    fprintf(stderr,"time %s: halted by user.\n",curtime());
    exit(1);
}

long runtime()
{
    struct timeval now[1];
    gettimeofday(now,0);
    now->tv_sec -= start->tv_sec;
    now->tv_usec -= start->tv_usec;
    if (now->tv_usec < 0)
    {
	now->tv_usec += 1000000;
	now->tv_sec--;
    }
    return now->tv_sec*1000+now->tv_usec/1000;
}

char *curtime()
{
    static int initMe = 1;
    struct timeval now[1];
    static char buffer[512];
    if (initMe)
    {
	initMe=0;
	gettimeofday(start,0);
    }
    gettimeofday(now,0);
    now->tv_sec -= start->tv_sec;
    now->tv_usec -= start->tv_usec;
    if (now->tv_usec < 0)
    {
	now->tv_usec += 1000000;
	now->tv_sec--;
    }
    if (now->tv_sec > 3600)
	sprintf(buffer,"%ld:%02ld:%02ld.%03d",
	    now->tv_sec/3600,
	    (now->tv_sec%3600)/60,
	    now->tv_sec%60,
	    now->tv_usec/1000);
    else if (now->tv_sec > 60)
	sprintf(buffer,"%ld:%02ld.%03d",
	    now->tv_sec/60,
	    now->tv_sec%60,
	    now->tv_usec/1000);
    else sprintf(buffer,"%ld.%03d",
	    now->tv_sec,
	    now->tv_usec/1000);
    return buffer;
}

void usage()
{
    fprintf(stderr,"Usage: %s +/-hsv\n",ProgName);
    fprintf(stderr,"\th\tprint this help\n");
    fprintf(stderr,"\ts\tuse Knuth's S heuristic %s\n",sheuristic?"[on]":"[off]");
    fprintf(stderr,"\tv\tprint verbose messages %s\n",verbose?"[on]":"[off]");
    fprintf(stderr,"Hitting sigquit (usually ^\\) prints status.\n");
    exit(1);
}

void parseArgs(int argc, char **argv)
{
    char *arg;
    ProgName = *argv++;  argc--;
    while (argc)
    {
	arg = *argv;
	if (*arg == '-' || *arg == '+')
	{
	    int value = *arg == '+';
	    switch (*++arg)
	    {
	      case 'h': usage(); break;
	      case 's': sheuristic = value; break;
	      case 'v': verbose = value; break;
	    }
	} else {
	    usage();
	}
	argc--;
    }
}

void verify(obj *head)
{
    int n,i,j,k,l,m;
    obj *p,*c,*r;
    if (head == 0)
    {
	printf("head is null!");
	return;
    }
    n = head->size;
    /* check for correct size */
    for (p = head->right, i = 0; i < n; p = p->right, i++)
    {
	assert(p != 0);
	assert(p != head);
	m = p->size;
	for (c = p->down, j = 0; j < m; c = c->down, j++)
	{
	    assert(c != 0);
	    assert(c != p);
	    assert(c->data == c->line[c->size]);
	    assert(c->top == p);
	    k = 1;
	    for (r = c->right; r != c; r = r->right)
	    {
		k++;
		assert(r->line == c->line);
	    }
	    for (r = c->left, l = 1; l < k; r = r->left, l++)
	    {
		assert(r != c);
	    }
	}
	assert(c == p);
	for (c = p->up, j = 0; j < m; c = c->up, j++)
	{
	    assert(c != 0);
	    assert(c != p);
	}
	assert(c == p);
    }
    assert(p == head);
}

int main(int argc, char **argv)
{
    signal(SIGQUIT,siginfo); /* control-backslash gets info */
    signal(SIGINT,sigint);  /* make sure control-c works */
    curtime();
    parseArgs(argc,argv);
    head = readProblem(stdin);
    if (verbose) fprintf(stderr,"%s: problem loaded.\n",curtime());
    search(0,head);
    if (verbose) {
	fprintf(stderr,"%s: finished.\n",curtime());
	stats();
    }
    return 0;
}
