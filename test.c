#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <stdbool.h>

#define MAXLINE 1000

typedef struct sharedobject 
{
	FILE *rfile;
	int linenum;
	char *line;
} so_t;

char *readline( FILE *rfile );

void *producer(void *arg) 
{
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	FILE *rfile = so->rfile;
	int i;
	char *line;
	for (i = 0; (line = readline(rfile)); i++) 
	{
		so->linenum = i;
		so->line = line;      /* share the line */
		fprintf(stdout, "Prod: [%d] %s", i, line);
	}
	printf("Prod: %d lines\n", i);
	*ret = i;
	pthread_exit(ret);
}

void *consumer(void *arg) 
{
	so_t *so = arg;
	int *ret = malloc(sizeof(int));
	int i = 0;
	int len;
	char *line;
	while ((line = so->line)) 
	{
		len = strlen(line);
		printf("Cons: [%d:%d] %s", i++, so->linenum, line);
	}
	printf("Cons: %d lines\n", i);
	*ret = i;
	pthread_exit(ret);
}

char *
readline( FILE *rfile ) {
  /* Read a line from a file and return a new line string object */
  char buf[MAXLINE];
  int len;
  char *result = NULL;
  char *line = fgets( buf, MAXLINE, rfile );
  if( line ) {
    len = strnlen( buf, MAXLINE );
    result = strncpy( malloc( len+1 ), buf , len+1 );
  }
  return result;
}

int main (int argc, char *argv[])
{
	pthread_t prod;
	pthread_t cons;
	int *ret;
	FILE *rfile;
	so_t *share = malloc(sizeof(so_t));
	rfile = fopen((char *) argv[1], "r");
	share->rfile = rfile;
	share->line = NULL;
	pthread_create(&prod, NULL, producer, share);
	pthread_create(&cons, NULL, consumer, share);
	printf("main continuing\n");
	int rc = pthread_join(prod, (void **) &ret);
	printf("main: producer joined with %d\n", *ret);
	rc = pthread_join(cons, (void **) &ret);
	printf("main: consumer joined with %d\n", *ret);
	pthread_exit(NULL);
	exit(0);
}




