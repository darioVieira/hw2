#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#define MAXWORD 1024
void *producer(void *arg);
void *consumer(void *arg);

typedef struct dict{//can either declare a new instance as dict_T name; OR struct dict name;
  char *word;
  int count;
  struct dict *next;
} dict_t;

typedef struct sharedObj
{
	char *word1;
	char *word2;
	bool eof;
	dict_t *firstDict;
	FILE *inFile;
	pthread_mutex_t dictLock;
	pthread_mutex_t wordLock;
} so_t;

char *
make_word( char *word ) {
  return strcpy( malloc( strlen( word )+1 ), word );//strcopy(char *dest,const char *src)- copies a string from source to destination,and returns a pointer to destination
}

dict_t *
make_dict(char *word) {
  dict_t *nd = (dict_t *) malloc( sizeof(dict_t) );
  nd->word = make_word( word );
  nd->count = 1;
  nd->next = NULL;
  return nd;
}

dict_t *
insert_word( dict_t *d, char *word ) {
  
  //   Insert word into dict or increment count if already there
  //   return pointer to the updated dict
  dict_t *nd;
  dict_t *pd = NULL;		// prior to insertion point 
  dict_t *di = d;		// following insertion point
  // Search down list to find if present or point of insertion
  while(di && ( strcmp(word, di->word ) >= 0) ) 
  { 
    if( strcmp( word, di->word ) == 0 ) 
    { 
      di->count++;		// increment count 
      return d;			// return head 
    }
    pd = di;			// advance ptr pair
    di = di->next;
  }
  nd = make_dict(word);		// not found, make entry 
  nd->next = di;		// entry bigger than word or tail 
  if (pd) 
  {
    pd->next = nd;
    return d;			// insert beond head 
  }
  return nd;
}

void print_dict(dict_t *d) {
  while (d) {
    printf("[%d]\t%s\n", d->count, d->word);
    d = d->next;
  }
}

int 
get_word( char *buf, int n, FILE *infile) {
  int inword = 0;
  int c;  
  while( (c = fgetc(infile)) != EOF ) {
    if (inword && !isalpha(c)) {
      buf[inword] = '\0';	// terminate the word string
      return 1;
    } 
    if (isalpha(c)) {
      buf[inword++] = c;
    }
  }
  return 0;			// no more words
}


dict_t *
words( FILE *infile ) {
  dict_t *wd = NULL;
  char wordbuf[MAXWORD];
  while( get_word( wordbuf, MAXWORD, infile ) ) 
  {
  	
    wd = insert_word(wd, wordbuf); // add to dict
  }
  return wd;
}

int
main( int argc, char *argv[] ) 
{
  dict_t *d = NULL;
  FILE *infile = stdin;
  if (argc >= 2) {
    infile = fopen (argv[1],"r");
  }
  if( !infile ) {
    printf("Unable to open %s\n",argv[1]);
    exit( EXIT_FAILURE );
  }
  
  pthread_t prod;
  pthread_t cons1;
  pthread_t cons2;
  
  void *ret;
  so_t *share=malloc(sizeof(so_t));
  
  share->inFile=infile;
  share->word1=NULL;
  share->word2=NULL;
  share->firstDict=malloc(sizeof(dict_t));
  share->firstDict=NULL;
  share->eof=false;
  pthread_create(&prod,NULL,producer,(void*) share);
  pthread_create(&cons1,NULL,consumer,(void*) share);
  pthread_create(&cons2,NULL,consumer,(void*) share);
  pthread_join(prod,&ret);
  pthread_join(cons1,&ret);
  pthread_join(cons2,&ret);// returns not important
  
  print_dict(share->firstDict);
  fclose( infile );
}

void *producer(void * arg)//stores word in shared obj
{
	so_t *share=(so_t *)arg;
	char wordbuf[MAXWORD];
	while(get_word( wordbuf, MAXWORD, share->inFile ) ) 
  {
  	while(share->word1!=NULL || share->word2!=NULL)
  	{}
    if(share->word1==NULL)
    {
    	share->word1=malloc(MAXWORD);
    	strcpy(share->word1,wordbuf); // let cons use
    }
    else
    {
    	share->word2=malloc(MAXWORD);
    	strcpy(share->word2,wordbuf);
    }
  }
  share->eof=true;
  pthread_exit(NULL);
}

void *consumer(void *arg)
{
	so_t *share=(so_t*)arg;
	char *tempWord;
	pthread_mutex_init( &share->wordLock, NULL );
	pthread_mutex_init( &share->dictLock, NULL );
	while(!share->eof || share->word1!=NULL || share->word2!=NULL)
	{
		int cnt=0;
		while(share->word1==NULL && share->word2==NULL && cnt<1000)
  	{
  		cnt++;
  	}
  	if(cnt<1000)
  	{
			pthread_mutex_lock(&share->wordLock);
			if(share->word1!=NULL)
			{
				tempWord=share->word1;
				share->word1=NULL;
			}
			else if(share->word2!=NULL)
			{
				tempWord=share->word2;
				share->word2=NULL;
			}
			pthread_mutex_unlock(&share->wordLock);
			pthread_mutex_lock(&share->dictLock);
			share->firstDict=insert_word(share->firstDict,tempWord);
			pthread_mutex_unlock(&share->dictLock);
		}
	}
	
	pthread_exit(NULL);
}
