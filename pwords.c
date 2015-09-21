#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>

typedef struct dict{//can either declare a new instance as dict_T name; OR struct dict name;
  char *word;
  int count;
  struct dict *next;
} dict_t;

typedef struct sharedObj
{
	int threadCnt;
	bool *getFlag;
	bool *insertFlag;
	dict_t *firstDict;
	FILE *inFile;
	char *wordBuf;
} so_t;

void *threadMethod(void* arg);

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

#define MAXWORD 1024
dict_t *
words( FILE *infile ) 
{
  dict_t *wd = NULL;
  char wordbuf[MAXWORD];
  
  pthread_t thread1;
  pthread_t thread2;
  
  void *ret1,*ret2;
  so_t *share=malloc(sizeof(so_t));
  
  share->inFile=infile;
  share->getFlag=false;
  share->insertFlag=false;
  share->wordBuf=wordbuf;
  share->firstDict=wd;
  share->threadCnt=1;
  printf("here\n");
  pthread_create(&thread1,NULL,threadMethod,(void*) share);
  printf("here\n");
  pthread_create(&thread2,NULL,threadMethod,(void*) share);
  printf("here\n");
  pthread_join(thread1,&ret1);
  pthread_join(thread2,&ret2);
  dict_t *finalRet = (dict_t*)ret1;
  
  /*while( get_word( wordbuf, MAXWORD, infile ) ) 
  {
  	
    wd = insert_word(wd, wordbuf); // add to dict
    printf("inside while:%s\n",wordbuf);
  }*/
  return finalRet;
}

void *threadMethod(void * arg)
{
	so_t *share=(so_t *)arg;
	int threadNum=share->threadCnt++;
	printf("here\n");
	while(share->getFlag==true){
		printf("%i,",threadNum);
	}
	share->getFlag=true;
	
	while( get_word( share->wordBuf, MAXWORD, share->inFile ) ) 
  {
		share->getFlag=false;
		while(share->insertFlag==true){}
		share->insertFlag=true;
		
    share->firstDict = insert_word(share->firstDict, share->wordBuf); // add to dict
    
		share->insertFlag=false;
		while(share->getFlag==true){}
		share->getFlag=true;
		
  }	
  return share->firstDict;
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
  
  d = words( infile );
  print_dict( d );
  fclose( infile );
}





















