#ifndef SEARCH_H
#define SEARCH_H

/* Include Necessary Library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/*Define Some Macros*/
#define Page_Step 30 /*the number of rows in a page*/
#define MAXLENGTH 64/*the max length of the str to save a word*/
#define SSTableFile_FILENAME "SSTableFile.txt"
#define BSTreeFile_FILENAME "BSTreeFile.txt"
#define BUFFER_FILENAME "Buffer.txt"
#define LIST_INIT_SIZE 1000 /*the initial length of the SSTable*/
#define LISTINCREMENT  100  /*the increment step every time the SSTable need to enlarge*/
#define STACK_INIT_SIZE 400 /*the initial length of the SqStack*/
#define STACK_INCREMENT 50  /*the increment step every time the SqStack need to enlarge*/
#define TEXT_FILENAME "textFile.txt"

/* Typedef of Enumeration */
typedef enum Status {NULL_POINTER = -1,ERROR=0,OK=1, MEMORY_OVERFLOW, UNDER_FLOW, OUT_OF_RANGE,FILE_ERROR}Status;
typedef enum Bool {FALSE = 0, TRUE}Bool;

/* Definition and Typedef of Data and Structure and typedef of Function Pointers */
/*Linklist struct  which saves the place information(page and row) where the word appears*/
typedef struct word_place
{
    unsigned int page;  
    unsigned char row;
    struct word_place* next;
}word_place;
/*struct which saves the name of the word, and time the word appears, the head pointer of the word's word_place Linklist*/
typedef struct word
{
    char* word_name;
    word_place* flag;
    unsigned int count;
}word;
/*struct which saves the length, size , head pointer of the word struct linearlist*/
typedef struct SSTable
{
    word *head;
    unsigned length;
    unsigned listsize;
}SSTable;
/*Binary Sorted Tree Node has a word struct member and two BSTree pointers pointing to its two sons*/
typedef struct BSTree
{
    word head;
    struct BSTree *lp;
    struct BSTree *rp;
}BSTree;
/*Sequence Stack of BSTree* type, for storing the BSTree pointers*/
typedef struct SqStack
{
    BSTree **base;
    BSTree **top;
    unsigned stacksize;
}SqStack;

/*Function Declaration*/
/*SSTable part*/
Status SaveWord(char *newword,int current_row);
Status Destroy_SST();
Status Search_SST(char *str);
Status SSearch_SST(char *str);
Status Creat_SST(char *bookname);
Status Traver_SST();
Status QuickSort_SST(word *R,int low,int high);
int Partition(word* a,int left,int right);
int BinSearch(word* R, char* K);
/*BSTree part*/
Status InitBSTree(char *bookname);
Status FreeSubTree(BSTree* bstp);
Status TraverSubTree(BSTree* bstp);
Status SaveWordToBSTree(char* newword,unsigned int current_row,int mode);
Status DestroyBSTree();
Status TraverseBSTree();
BSTree *SearchSubTree(BSTree* T, char* key);
Status SearchBST(char* key);
Status InsertBSTree(char *str, unsigned int page, unsigned int row);
Status DeleteBSTree(char *str);
/*File part, Free-room part, and auxiliary part*/
Status WriteSSTableFile();
Status ReadSSTableFile();
Status WriteBSTreeFile();
Status PreOrderPrint(FILE * fp);
Status ReadBSTreeFile();
Status ReCreatBiTree(BSTree** bstp, FILE *fp);
Status FreeAll();
Status saveTextFile();
Status creatText();
Status creatTextlist(char *textfile, unsigned rownum);
Status creatWpplist(word* wp);
/*Assistant Data struct part*/
Status InitStack(SqStack **S);
Status Push(SqStack *S,BSTree* tnp);
Status GetTop(SqStack *S,BSTree** p);
Status Pop(SqStack *S,BSTree** p);
Bool StackEmpty(SqStack *S);
Status FreeStack(SqStack *S);

/* Global Variables */
extern unsigned int current_row;/*exact row number in the whole book*/
extern char current_str[MAXLENGTH];/*var to save the string just now got*/
extern SSTable *SST_head; /*global var to point to the SSTable in the program*/
extern BSTree *BST_head ;/*root of the BSTree*/
extern unsigned int wordnum;    /*var to count the number of different words in the BSTree*/
extern char strtest[64];   /*var for searching a word in both ADT,and also for inserting a word into the BSTree*/
extern unsigned int pagetest, rowtest; /*var for inserting a word into the BSTree*/
extern char errbuff[64];   /*buffer to store the function returned information*/
extern QString text;   /*buffer to store the function returned information*/
extern word * Global_wp;  /*var to store the word pointer of the word which is found*/
extern word_place * Global_wpp;/*var to store the word_place pointer of the word which is found*/
extern word *Global_arr;/*var to store the original unsorted word struct array*/
extern char **global_textarr;/*array of strings which store the full text of the book file*/
extern word_place **global_wpparr;/*array of word_place pointers which save the information where the word appears*/
extern char *filepath;/*full path of the book file to read*/
extern word_place **global_wppp;/*var to locate the word_place pointers in the @globar_wpparr*/
/* Definition of File Operations*/
#endif // SEARCH_H
