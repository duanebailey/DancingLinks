// An optimized version of cover that makes use of bits. -*- compile-command: "gcc -g -o cover2 cover2.c" -*-
// (c) 2003 duane a. bailey 

#include <stdio.h>
#include <sys/signal.h>

typedef struct prob
{
} prob;

int printUpdate = 0;
char *ProgName = 0;

void parseArgs(int argc, char **argv);
void usage();

prob *readProblem();
int search(prob *problem, int level);

void sig_info(int signum);

/* run through, and parse the arguments of the command line */
void parseArgs(int argc, char **argv)
{
    char *arg;
    ProgName = *argv;
    argv++; argc--;
    while (argc)
    {
	arg = *++argv;
	argc--;
	usage();
    }
}

/* print usage */
void usage()
{
    fprintf(stderr,"Usage: %s\n",ProgName);
    exit(1);
}

/* signal handler for ^\ */
void sig_info(int signum)
{
    printUpdate = 1;
}

/* read in a problem from standard input */
prob *readProblem()
{
    return 0;
}

/* search for cover solution, after having placed depth pieces */
int search(prob *p, int depth)
{
    return 0;
}
ffs
int main(int argc, char **argv)
{
    prob *problem = 0;
    signal(SIGQUIT,sig_info);
    parseArgs(argc,argv);
    problem = readProblem();
    search(problem,0);
}
