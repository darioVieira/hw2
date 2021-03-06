#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <pthread.h>
#include <stdbool.h>
#define MAXWORD 1024

typedef struct dict//can either declare a new instance as dict_t name; OR struct dict name;
{
  char *word;//the word
  int cnt;//num of the word
  struct dict *next;//pointer to the next word
} dict_t;

typedef struct charQueue
{
	char *word;
	struct charQueue *next;
} charQueue_t;

typedef struct SharedObject
{
  charQueue_t *charQueueHead;//MAKE CHARQUEUE TAIL - will also have to have a doubly linked queue
  bool queueBusy;//if queue is being used by a thread (primitive LOCK)
  bool dictionaryBusy;
  char *wordBuf;
  bool *prodDone;
  bool *consDone;
  FILE *infile;
} so_t;

dict_t *firstWord; 


char * make_word( char *word ) 
{
  return strcpy( malloc( strlen( word )+1 ), word );//strcopy(char *dest,const char *src)- copies a string from source to destination,and returns a pointer to destination
}

dict_t * make_dict(char *word) 
{
  dict_t *newDic = (dict_t *) malloc( sizeof(dict_t) );
  newDic->word = make_word( word );
  newDic->cnt = 1;
  newDic->next = NULL;
  return newDic;
}

void insert_word( dict_t *firstDic, char *word, so_t *share) 
{
  printf("6a\n");
  //   Insert word into dict or increment count if already there
  //   return pointer to the updated dict
  
  dict_t *newDic;
  dict_t *prevDic = NULL;		// prior to insertion point 
  dict_t *dic = firstDic;		// following insertion point
  // Search down list to find if present or point of insertion
  printf("6b\n");
  while(dic && ( strcmp(word, dic->word ) >= 0) )// 
  { 
  	printf("6d\n");
    if( strcmp( word, dic->word ) == 0 ) 
    { 
      dic->cnt++;		// increment count 
      return firstDic;			// return head 
    }
    prevDic = dic;			// advance ptr pair
    dic = dic->next;
  }
  printf("6e\n");
  while(share->dictionaryBusy)
  {
	  pthread_yield();
  }
  printf("6f\n");
  share->dictionaryBusy=true;
  newDic = make_dict(word);		// not found, make entry 
  newDic->next = dic;		// entry bigger than word or tail 
  printf("6g\n");
  if (prevDic) //is not NULL... i.e. there exists at least 1 element ahead of the new item (while loop was entered)
  {
    prevDic->next = newDic;
    //return firstDic;			// insert beond head 
  }
  //return newDic;
  share->dictionaryBusy=false;
  printf("6h\n");
}

void print_dict(dict_t *d) {
  while (d) {
    printf("[%d]\t%s\n", d->cnt, d->word);
    d = d->next;
  }
}

void get_word(char* wordBuf, FILE *infile) 
{
	printf("5a\n");
  int inword = 0;
  int c;  
  while( (c = fgetc(infile)) != EOF ) 
  {
  	printf("5b-%c\n",c);
    if (inword && !isalpha(c)) 
    {
      wordBuf[inword] = '\0';	// terminate the word string
      printf("5c-%s\n",wordBuf);
      return;
    } 
    if (isalpha(c)) 
    {
      wordBuf[inword++] = c;
    }
  }
  printf("5d\n");
  return;			// no more words
}

void *consumer(void *arg)
{
	printf("4a\n");
	so_t *share=(so_t*) arg;
	while(!share->prodDone || !share->charQueueHead->word)//while producer is not done OR there are existing words in the queue
	{
		if(share->charQueueHead->word==NULL || share->queueBusy==true)//if theres nothing to do (i.e. waiting for producer) OR the queue is being used by a different process - relinquish the processor
		{
				pthread_yield();
		}
		else
		{
			share->queueBusy=true;
			char *word=share->charQueueHead->word;
			share->charQueueHead=share->charQueueHead->next;
			share->queueBusy=false;
			printf("4b\n");
			insert_word(share->charQueueHead, share->wordBuf,share);
			printf("4c\n");
			printf("#####################\n%d\n#####################\n",share->dictionaryBusy);
			while(share->dictionaryBusy)
			{
				printf("insideLoop\n");
				pthread_yield();
			}
			
			//must now be added to the Dictionary
		}
		//share->wordBuf=malloc(sizeof(char*));
		printf("4e\n");
	}
	printf("4d\n");
	share->consDone=true;
}

void *producer(void *arg)
{
	printf("3a\n");
	so_t* share = (so_t*) arg;
	char *newWord;
	
	while( newWord ) 
  {
  	
  	printf("3b%s\n",share->charQueueHead->word);
  	get_word(share->wordBuf,share->infile);
  	char *newWord;
  	strcpy(newWord,share->wordBuf);
  	//printf("3b-%s\n",share->wordBuf);
  	
  	
		if((share->charQueueHead->word)==NULL)//if there is nothing in the queue
		{
			printf("3e-%s\n",newWord);
			
			share->charQueueHead->word=newWord;
			//printf("\n**********************\n");
			printf("**********************\n%s\n**********************\n",share->charQueueHead->word);
		}
		else
		{
			printf("3f-%s\n",share->charQueueHead->word);
			charQueue_t *curr=share->charQueueHead;
			while(curr!=NULL)//end of queue
			{
				curr=curr->next;
			}
			charQueue_t *nextEntry=malloc(sizeof(charQueue_t));
			printf("here\n");
			nextEntry->word=newWord;
			printf("here\n");
			printf("&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n%s\n&&&&&&&&&&&&&&&&&&&&&&&&&&&&\n",newWord);
			nextEntry->next=NULL;
			curr=nextEntry;
			//printf("\nNONONONONONONONONONON\n");
		}
		printQueue(share->charQueueHead);
		char blah=getchar();
		printf("3c\n");
	}
  share->prodDone=true;
  while(!share->consDone)
  {
  	pthread_yield();
  }
  //print_dict(firstWord);
  printf("3d\n");
}

void printQueue(charQueue_t *first)
{
	int i=0;
	charQueue_t *curr=first;
	while(curr)
	{
		printf("%i)%s\n",i,first->word);
		curr=curr->next;
		i++;
	}
}


void words( FILE *infile ) 
{
	pthread_t prod;
	pthread_t cons;
	printf("2a\n");
	char wordbuf[MAXWORD];
	
	so_t * share;
	share->charQueueHead=malloc(sizeof(charQueue_t));
	share->queueBusy=false;
	share->dictionaryBusy=false;
	share->charQueueHead->word=NULL;
	share->charQueueHead->next=malloc(sizeof(charQueue_t));
	share->charQueueHead->next=NULL;
	share->wordBuf=wordbuf;
	share->prodDone=false;
	share->consDone=false;
	share->infile=infile;
	
	int status;
	void *ret;
	if( (status = pthread_create(&prod,NULL,producer,(void*)share)) != 0 )
    printf("create producer thread\n" );
	/*if( (status=pthread_create(&cons,NULL,consumer,(void*)share)) !=0 )
		printf("create consumer thread\n");*/
		
		
	if( (status = pthread_join( prod, &ret )) != 0)
    printf("join producer thread\n" );
	
	
  printf("2b\n");
  /*while( get_word(wordbuf,infile) ) //prod
  {
    wordDic = insert_word(wordDic, wordbuf); // add to dict- Cons
  }*/
  
  //pthread_exit(NULL);
  //return first;
}

int main( int argc, char *argv[] ) 
{
  dict_t *d = NULL;
  firstWord= malloc(sizeof(dict_t));
  firstWord=NULL;
  FILE *infile = stdin;
  if (argc >= 2) 
  {
    infile = fopen (argv[1],"r");
  }
  if( !infile ) 
  {
    printf("Unable to open %s\n",argv[1]);
    exit( EXIT_FAILURE );
  }
  printf("1\n");
  words(infile);
  fclose( infile );
}

