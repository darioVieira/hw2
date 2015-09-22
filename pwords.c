#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>

void *producer(void *arg);
void *consumer(void *arg);

typedef struct dict{//can either declare a new instance as dict_T name; OR struct dict name;
  char *word;
  int count;
  struct dict *next;
} dict_t;

typedef struct wordLink
{
	char *word;
	struct wordLink *next;
	struct wordLink *prev
} wordLink;

typedef struct sharedObj
{
	wordLink *wordLinkHead;
	wordLink *wordLinkTail;
	bool flag;
	bool eof;
	dict_t *firstDict;
	FILE *inFile;
	pthread_mutex_t wordsLock;
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
  printf("4a-%i\n",strcmp(word, di->word ));
  // Search down list to find if present or point of insertion
  while(di && ( strcmp(word, di->word ) >= 0) ) 
  { 
  	printf("4f\n");
    if( strcmp( word, di->word ) == 0 ) 
    { 
      di->count++;		// increment count 
      return d;			// return head 
    }
    pd = di;			// advance ptr pair
    di = di->next;
  }
  printf("4b\n");
  nd = make_dict(word);		// not found, make entry 
  printf("4c\n");
  nd->next = di;		// entry bigger than word or tail 
  if (pd) 
  {
  	printf("4d\n");
    pd->next = nd;
    return d;			// insert beond head 
  }
  printf("4e\n");
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
words( FILE *infile ) {
  dict_t *wd = NULL;
  char wordbuf[MAXWORD];
  while( get_word( wordbuf, MAXWORD, infile ) ) 
  {
  	
    wd = insert_word(wd, wordbuf); // add to dict
    printf("inside while:%s\n",wordbuf);
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
  pthread_t cons;
  void *ret;
  so_t *share=malloc(sizeof(so_t));
  share->inFile=infile;
  share->flag=false;
  share->wordLinkHead=malloc(sizeof(wordLink));
  share->wordLinkHead->next=NULL;
  share->wordLinkHead->word=NULL;
  share->wordLinkHead->prev=NULL;
  share->wordLinkTail=share->wordLinkHead;
  share->firstDict=malloc(sizeof(dict_t));
  share->firstDict->word=NULL;
  share->eof=false;
  
  printf("1a\n");
  pthread_create(&prod,NULL,producer,(void*) share);
  //pthread_create(&cons,NULL,consumer,(void*) share);
  printf("1b\n");
  pthread_join(prod,&ret);
  printf("1c\n");
  //d = words( infile );
  //print_dict( d );
  fclose( infile );
}

void *producer(void * arg)//stores word in shared obj
{
	so_t *share=(so_t *)arg;
	char wordbuf[MAXWORD];
	int cnt=0;
	
	while(get_word( wordbuf, MAXWORD, share->inFile ) ) 
  {
  	wordLink *temp=malloc(sizeof(wordLink));
  	
		printf("temp loc-\t%i\n",&temp);
		temp->word=wordbuf;
		temp->next=&share->wordLinkHead;
		temp->prev=NULL;
		share->wordLinkHead->prev=&temp;
		share->wordLinkHead = temp;
		//wordLink *first=share->wordLinkHead;
		//printf("here1\n");
		//free(temp);
		//printf("here2\n");
		//printf("done printing...\n");
		//printf("done printing...\n");
		printNowBiatch(share->wordLinkTail);
		
		//printf("here5\t%s\tthen\t%s\n",share->wordLinkHead->word,share->wordLinkHead->next->word);
  }
  //printf("here3\n");
  //printf("%s\n",share->wordLinkHead->word);
  //printf("here4\n");
  share->wordLinkHead->word="POEPOL";
  share->eof=true;
  
  print_dict(share->firstDict);
  pthread_exit(NULL);
}

void printNowBiatch(wordLink *last)
{
	while(last)
	{
		printf("word:\t%s-\t%i\n",last->word,&last);
		last=last->prev;
	}
	printf("done printing...\n");
}

/*void *consumer(void *arg)
{
	so_t *share=(so_t*)arg;
	char *tempWord;
	while(!share->eof)
	{
		wordLink temp;
		insert_word(share->firstDict,tempWord);
	}
	pthread_exit(NULL);
}*/


