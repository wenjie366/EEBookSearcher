#include <QApplication>
#include <QMainWindow>
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "search.h"

/*Global var*/
unsigned int current_row = 0;/*exact row number in the whole book*/
char current_str[MAXLENGTH];/*var to save the string just now got*/
SSTable *SST_head = NULL; /*global var to point to the SSTable in the program*/
BSTree *BST_head = NULL;/*root of the BSTree*/
unsigned int wordnum = 0;	/*var to count the number of different words in the BSTree*/
char strtest[64];   /*var for searching a word in both ADT,and also for inserting a word into the BSTree*/
unsigned int pagetest, rowtest; /*var for inserting a word into the BSTree*/
char errbuff[64];   /*buffer to store the function returned information*/
QString text;   /*buffer to store the function returned information*/
word* Global_wp = NULL; /*var to store the word pointer of the word which is found*/
word_place* Global_wpp = NULL;/*var to store the word_place pointer of the word which is found*/
word* Global_arr = NULL;/*var to store the original unsorted word struct array*/
char **global_textarr = NULL;/*array of strings which store the full text of the book file*/
word_place **global_wpparr = NULL;/*array of word_place pointers which save the information where the word appears*/
word_place **global_wppp = NULL;/*var to locate the word_place pointers in the @globar_wpparr
*/

/**
 * Creat the Staic Search Table of the book file named @bookname and sort the SST,
 * before sorting the original SST, a new word struct array @Global_arr is dublicated from the original SST's @head member.
 * @param    bookname                 the book file's name
 * @return                            the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:09:17+0800
 */
Status Creat_SST(char *bookname)
{
    current_row = 1;
    if(SST_head != NULL)
    {
        strcpy(errbuff,"SSTable already created!");
        return ERROR;
    }
    else
    {
        if((SST_head = (SSTable *)malloc(sizeof(SSTable))) == NULL)
            return(MEMORY_OVERFLOW);
        SST_head->length = 0;
        SST_head->listsize = LIST_INIT_SIZE;
        if((SST_head->head = (word *)malloc(sizeof(word)*LIST_INIT_SIZE)) == NULL)
            return(MEMORY_OVERFLOW);
        //*initial the pointers and word counter to NULL and 0*/
        memset(SST_head->head,0,LIST_INIT_SIZE*sizeof(word));
    }
    FILE *fp;
    char c;
    char *newword;
    int flag = 0;/*when flag==1, meaning it's reading a word*/
    int count=0;/*the reading place in the current_str*/
    if((fp=fopen(bookname,"r")) == NULL)
    {
        strcpy(errbuff,"Read Book File Failed!");
        return(ERROR);
    }
    while(fscanf(fp,"%c",&c) != EOF)
    {
        if(c == '\n')
        {
            current_row++;
            continue;
        }
        if(flag==0)
        {
            if((c>='a'&&c<='z') || (c>='A'&&c<='Z'))
            {
                flag = 1;
                current_str[count] = c;
                count++;
            }
            else
                continue;
        }
        else if(flag==1)
        {
            if((c>='a'&&c<='z') || (c>='A'&&c<='Z') || (c=='-'&&(current_str[count-1]!='-')))
            {
                current_str[count] = c;
                count++;
            }
            else
            {
                if(current_str[count-1]=='-')/*double constant '-' occur*/
                {
                    current_str[count-1] = '\0';
                    count--;
                }
                else
                    current_str[count] = '\0';

                if((newword = (char *)malloc((count+1)*sizeof(char))) == NULL)
                    return(MEMORY_OVERFLOW);
                strcpy(newword,current_str);
                /*Refresh the flag and counter*/
                flag = count = 0;
                if(SaveWord(newword,current_row) != OK)
                {
                    strcpy(errbuff,"SaveWord ERROR!");
                    return ERROR;
                }
            }
        }
    }
    /*When EOF comes*/
    if(flag == 1 && (count>=1))
    {
        if(current_str[count-1]=='-')/*the last char is a '-'*/
        {
            current_str[count-1] = '\0';
            count--;
        }
        else
            current_str[count] = '\0';
        if((newword = (char *)malloc((count+1)*sizeof(char))) == NULL)
            return(MEMORY_OVERFLOW);
        strcpy(newword,current_str);
        /*Refresh the flag and counter*/
        flag = count = 0;
    }
    fclose(fp);
    /*duplicate the unsorted SSTable*/
    if((Global_arr=(word*)malloc((SST_head->length)*sizeof(word))) == NULL)
        return(MEMORY_OVERFLOW);
    memcpy(Global_arr,SST_head->head,(SST_head->length)*sizeof(word));

    QuickSort_SST(SST_head->head,0,SST_head->length-1);
    text.sprintf("total row number:%d\ntotal word number:%d\n",current_row,SST_head->length);
    return OK;
}
/**
 * Save the word @newword appear in the @current_row in the book file to the SST
 * @param    newword                  the name of the word to save
 * @param    current_row              the number of the row in which this newword appears 
 * @return                            the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:11:57+0800
 */
Status SaveWord(char *newword,int current_row)
{
    word* wp;
    word_place* wpp, *temp;
    char* cp;
    int i;

    if(SST_head->length == SST_head->listsize)
    {
        if(!(SST_head->head=(word*)realloc(SST_head->head,(SST_head->length+LISTINCREMENT)*sizeof(word))))
        {
            printf("Memory realloc failed!\n");
            return MEMORY_OVERFLOW;
        }
        SST_head->listsize += LISTINCREMENT;
        memset(SST_head->head+SST_head->length,0,LISTINCREMENT*sizeof(word));/*make the pointers in the structs all NULL, make counter 0*/
    }

    for(i=0,wp=SST_head->head,cp=wp->word_name;i<SST_head->length;i++)
    {
        if(strcmp(cp,newword) == 0)/*if the word already saved in the SSTable, save its place to the word_place List*/
        {
            if(cp != newword)/*when (cp == newword): ReadFile Mode, don't need to free the same string pointer*/
                free(newword);
            break;
        }
        wp++;
        cp=wp->word_name;
    }
    if(i==SST_head->length)/*if the word not already saved in the SSTable, it's new here*/
    {
        SST_head->length++;
        wp->word_name = newword;
    }
    if((wpp = (word_place*)malloc(sizeof(word_place))) == NULL)
        return(MEMORY_OVERFLOW);
    /*caculate the page and row number of the new wordplace*/
    wpp->page = (current_row-1)/Page_Step + 1;
    wpp->row = current_row - (wpp->page-1) * Page_Step;
    /*Insert the new word_place in the corresponded list*/
    temp = wp->flag;
    if(wp->flag == NULL)
        wp->flag = wpp;
    else
    {
        while(temp->next != NULL)
            temp = temp->next;
        temp->next = wpp;
    }
    wpp->next=NULL;
    wp->count++;
    return OK;
}
/**
 * Destroy the SST struct
 * @return   the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:20:17+0800
 */
Status Destroy_SST()
{
    printf("SSTable is released!\n");
    if(!SST_head)
        return ERROR;
    word* wp = SST_head->head;
    word_place* wpp = wp->flag;
    word_place* temp;
    int i;
    for(i=0;i<SST_head->length;i++)
    {
        wpp=wp->flag;
        while(wpp != NULL)
        {
            temp = wpp;
            wpp = wpp->next;
            free(temp);
        }
        free(wp->word_name);
        wp++;
    }
    free(SST_head->head);
    free(SST_head);
    SST_head = NULL;
    current_row = 1;
    return OK;
}
/**
 * Search the word named @str in the sorted word struct array @SST.head    
 * @param    str                      the name of the word to search
 * @return                            the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:24:02+0800
 */
Status SSearch_SST(char *str)
{
    if(!SST_head)
    {
        strcpy(errbuff,"SSTable not exists!\n");
        return ERROR;
    }
    int i = BinSearch(SST_head->head,str);
    if(i == -1)/*Not found*/
    {
        strcpy(errbuff,"Word Not Founded!\n");
        return ERROR;
    }
    Global_wp = SST_head->head + i;
    Global_wpp = Global_wp->flag;
    return OK;
}
/**
 * Search the new @str in the unsorted word struct array @Global_arr
 * @param    str                      the name of word to search  
 * @return                            the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:29:52+0800
 */
Status Search_SST(char *str)
{
    if(!SST_head)
    {
        strcpy(errbuff,"SSTable not exists!\n");
        return ERROR;
    }
    word* wp = Global_arr;
    word_place* wpp = wp->flag;
    int i;
    for(i=0;i<SST_head->length;i++)
    {
        if(!strcmp(wp->word_name,str))
            break;
        wp++;
        wpp=wp->flag;
    }
    if(i == SST_head->length)
    {
        strcpy(errbuff,"Word Not Founded!\n");
        return ERROR;
    }
    Global_wp = wp;
    Global_wpp = wp->flag;
    return OK;
}
/**
 * Travel the sorted word struct @SST.head by the linear sequence, print words and all the place where they appear
 * @return   the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:33:26+0800
 */
Status Traver_SST()
{
    if(!SST_head)
    {
        strcpy(errbuff,"SSTable not exists!\n");
        return ERROR;
    }
    FILE *fp;
    if((fp=fopen(BUFFER_FILENAME,"w")) == NULL)
    {
        strcpy(errbuff,"File open failed!");
        return(ERROR);
    }
    word* wp = SST_head->head;
    word_place* wpp = wp->flag;
    int i, count;
    for(i=0;i<SST_head->length;i++)
    {
        count = 1;
        fprintf(fp,"Word  : %s\n",wp->word_name);
        while(wpp != NULL)
        {
            fprintf(fp,"[%d].  page:%d row:%d\n",count++,wpp->page,wpp->row);
            wpp = wpp->next;
        }
        wp++;
        wpp=wp->flag;
        fputc('\n',fp);
    }
    fclose(fp);
    return OK;
}
/**
 * Find the pivotpos of array @a[left,left+1,..,right] 
 * @param    a                        word struct array
 * @param    left                     the left boundary of @a
 * @param    right                    the right boundary of @a
 * @return                            the subscript of the pivotpos
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:42:51+0800
 */
int Partition(word* a,int left,int right)
{
    int i=left;
    int j=right;
    word temp=a[i];
    while(i<j)
    {
        while(i<j && (strcmp(a[j].word_name,temp.word_name)>0))
            j--;
        if(i<j)
            a[i]=a[j];
        while(i<j && (strcmp(a[i].word_name,temp.word_name)<0))
            i++;
        if(i<j)
            a[j]=a[i];
    }
    a[i]=temp;
    return i;
}
/**
 * Quick sort the word struct array @R recursively 
 * @param    R                        the array to sort
 * @param    low                      the left boundary of @R
 * @param    high                     the right boundary of @R
 * @return                            the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:50:54+0800
 */
Status QuickSort_SST(word* R,int low,int high)
{
    int pivotpos;
    if(low<high)
    {
        pivotpos=Partition(R,low,high);
        QuickSort_SST(R,low,pivotpos-1);
        QuickSort_SST(R,pivotpos+1,high);
    }
    return OK;
}
/**
 * binary search the word named @K in the sorted table @R[1..n], if found return the place number, or return -1
 * @param    R                        the word struct array to search
 * @param    K                        name of the word to find
 * @return                            the subscript of the word to find
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:54:01+0800
 */
int BinSearch(word* R, char* K)
{
    int low=0,high=SST_head->length-1,mid; //set the initial value of low and high
    while(low<=high)
    {
        mid=(low+high)/2;
        if(!strcmp(R[mid].word_name,K))
            return mid;
        if(strcmp(R[mid].word_name,K)>0)
            high=mid-1;
        else
            low=mid+1;
     }
    return 0;
} 
/**
 * Save the word @newword to the Binary Sorted Tree @BST_head
 * @param    newword                  the name of the word to save
 * @param    current_row              the number of row in which @newword appears
 * @param    mode                     default value of mode is 0, to make sure the word_place struct linklist
 *                                    a sorted list, need to set the mode to 1, indicating the insert mode
 * @return                            the running status of this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T08:57:55+0800
 */
Status SaveWordToBSTree(char* newword,unsigned int current_row,int mode = 0)
{
    word* wp;
    word_place* wpp, *t;
    int i;
    BSTree *bstp = BST_head;
    BSTree *temp;
    if((wpp = (word_place*)malloc(sizeof(word_place))) == NULL)
        return(MEMORY_OVERFLOW);
    /*the newword is the first word to save*/
    if(bstp->head.word_name == NULL)
    {
        BST_head->head.word_name = newword;
        wp = &(BST_head->head);
        wordnum++;
    }
    /*search the newword in the BSTree*/
    else
    {
        while(bstp != NULL)
        {
            i = strcmp(newword,bstp->head.word_name);
            /*if the newword already saved in the BSTree*/
            if(i == 0)
            {
                wp = &(bstp->head);
                if(newword != bstp->head.word_name)/*when (bstp->head.word_name == newword): ReadFile Mode, don't need to free the same string pointer*/
                    free(newword);
                break;
            }
            else
            {
                /*if the newword is new here in the BSTree*/
                if(i < 0)
                {
                    if(bstp->lp != NULL)
                        bstp = bstp->lp;
                    else
                    {
                        if((temp = (BSTree*)malloc(sizeof(BSTree))) == NULL)
                            return(MEMORY_OVERFLOW);
                        temp->head.word_name = newword;
                        temp->head.flag = NULL;
                        temp->head.count = 0;
                        temp->lp = temp->rp = NULL;
                        wp = &(temp->head);
                        /*insert the new BSTree Node to the Tree*/
                        bstp->lp = temp;
                        wordnum++;
                        break;
                    }
                }
                else if(i > 0)
                {
                    if(bstp->rp != NULL)
                        bstp = bstp->rp;
                    else
                    {
                        if((temp = (BSTree*)malloc(sizeof(BSTree))) == NULL)
                            return(MEMORY_OVERFLOW);
                        temp->head.word_name = newword;
                        temp->head.flag = NULL;
                        temp->head.count = 0;
                        temp->lp = temp->rp = NULL;
                        wp = &(temp->head);
                        /*insert the new BSTree Node to the Tree*/
                        bstp->rp = temp;
                        wordnum++;
                        break;
                    }
                }
            }
        }
    }
    /*caculate the page and row number of the new wordplace*/
    wpp->page = (current_row-1)/Page_Step + 1;
    wpp->row = current_row - (wpp->page-1) * Page_Step;
    /*Insert the new word_place in the corresponded list*/
    t = wp->flag;
    if(wp->flag == NULL)
    {
        wp->flag = wpp;
        wpp->next =NULL;
    }
    else
    {
        if(mode == 0)/*Normal Mode*/
        {
            while(t->next != NULL)
                t = t->next;
            t->next = wpp;
            wpp->next=NULL;
        }
        else if(mode == 1)/*Insert Mode*/
        {
            word_place *pri = wp->flag;
            while(t != NULL && current_row > (t->page-1)*Page_Step + t->row)
            {
                pri = t;
                t = t->next;
            }
            if(t == wp->flag)
                wp->flag = wpp;
            else
                pri->next = wpp;
            wpp->next = t;
        }
        
    }
    wp->count++;
    return OK;
}
/**
 * Initial the Binary Sorted Tree of book file @bookname
 * @param    bookname                 the name of the book file
 * @return                            the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:11:22+0800
 */
Status InitBSTree(char *bookname)
{
    current_row = 1;
    if(BST_head != NULL)
    {
        strcpy(errbuff,"BSTable already created!\n");
        return ERROR;
    }
    else
    {
        if((BST_head = (BSTree *)malloc(sizeof(BSTree))) == NULL)
            return(MEMORY_OVERFLOW);
        BST_head->head.word_name = NULL;
        BST_head->head.flag = NULL;
        BST_head->head.count = 0;
        BST_head->lp = NULL;
        BST_head->rp = NULL;
    }
    FILE *fp;
    char c;
    char *newword;
    int flag = 0;/*when flag==1, meaning it's reading a word*/
    int count = 0;/*the reading place in the current_str*/
    if((fp=fopen(bookname,"r")) == NULL)
    {
        strcpy(errbuff,"Read Book File Failed!\n");
        return(ERROR);
    }
    while(fscanf(fp,"%c",&c) != EOF)
    {
        if(c == '\n')
        {
            current_row++;
            continue;
        }
        if(flag==0)
        {
            if((c>='a'&&c<='z') || (c>='A'&&c<='Z'))
            {
                flag = 1;
                current_str[count] = c;
                count++;
            }
            else
                continue;
        }
        else if(flag==1)
        {
            if((c>='a'&&c<='z') || (c>='A'&&c<='Z') || (c=='-'&&(current_str[count-1]!='-')))
            {
                current_str[count] = c;
                count++;
            }
            else
            {
                if(current_str[count-1]=='-')/*double constant '-' occur*/
                {
                    current_str[count-1] = '\0';
                    count--;
                }
                else
                    current_str[count] = '\0';

                if((newword = (char *)malloc((count+1)*sizeof(char))) == NULL)
                    return(MEMORY_OVERFLOW);
                strcpy(newword,current_str);
                /*Refresh the flag and counter*/
                flag = count = 0;
                if(SaveWordToBSTree(newword,current_row) != OK)
                {
                    strcpy(errbuff,"SaveWord ERROR!");
                    return ERROR;
                }
            }
        }
    }
    /*EOF comes*/
    if(flag == 1 && (count>=1))
    {
        if(current_str[count-1]=='-')/*the last char is a '-'*/
        {
            current_str[count-1] = '\0';
            count--;
        }
        else
            current_str[count] = '\0';
        if((newword = (char *)malloc((count+1)*sizeof(char))) == NULL)
            return(MEMORY_OVERFLOW);
        strcpy(newword,current_str);
        /*Refresh the flag and counter*/
        flag = count = 0;
        if(SaveWordToBSTree(newword,current_row) != OK)
        {
            strcpy(errbuff,"SaveWord ERROR! ");
            return ERROR;
        }
    }
    fclose(fp);
    return OK;
}
/**
 * Destroy the BSTree @BST_head
 * @return   the status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:13:31+0800
 */
Status DestroyBSTree()
{
    printf("BSTable is released!\n");
    if(BST_head == NULL)
        return ERROR;
    FreeSubTree(BST_head);
    wordnum = 0;
    current_row = 1;
    BST_head = NULL;
    return OK;
}

/*destroy subtree bstp*/
/**
 * Free the subtree @bstp recursivly
 * @param    bstp                     the subtree
 * @return                            running status of this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:15:05+0800
 */
Status FreeSubTree(BSTree* bstp)
{
    word_place* wpp;
    word_place* temp;
    if(bstp != NULL)
    {
        free(bstp->head.word_name);
        wpp = bstp->head.flag;
        while(wpp != NULL)
        {
            temp = wpp;
            wpp = wpp->next;
            free(temp);
        }
        FreeSubTree(bstp->lp);
        FreeSubTree(bstp->rp);
        free(bstp);
    }
    return OK;
}
/**
 * Traver the BSTree recursivly by inorder 
 * @return   running status of this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:18:24+0800
 */
Status TraverseBSTree()
{
    if(BST_head == NULL)
    {
        strcpy(errbuff,"BSTable hasn't been created!\n");
        return ERROR;
    }
    TraverSubTree(BST_head);
    return OK;
}


/**
 * visit subtree bstp by inorder    
 * @param    bstp                     subtree
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:19:20+0800
 */
Status TraverSubTree(BSTree* bstp)
{
    SqStack *S;
    BSTree* p;
    word* wp;
    word_place* wpp;
    int count = 1;
    FILE *fp;
    if((fp=fopen(BUFFER_FILENAME,"w")) == NULL)
    {
        strcpy(errbuff,"File open failed!");
        return(ERROR);
    }
    InitStack(&S);
    Push(S,bstp);
    while(!StackEmpty(S))
    {
        while(GetTop(S,&p) && p)
        {
            Push(S,p->lp);
        }
        Pop(S,&p);
        if(!StackEmpty(S))
        {
            Pop(S,&p);
            count = 1;
            wp = &(p->head);
            wpp = wp->flag;
            fprintf(fp,"Word  : %s\n",wp->word_name);
            while(wpp != NULL)
            {
                fprintf(fp,"[%d].  page:%d row:%d\n",count++,wpp->page,wpp->row);
                wpp = wpp->next;
            }
            fputc('\n',fp);
            Push(S,p->rp);
        }
    }
    fclose(fp);
    FreeStack(S);
    return OK;
}
/**
 * Initial the SqStack @S
 * @param    S                        SqStack which need to initiald   
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:20:37+0800
 */
Status InitStack(SqStack **S)
{
    *S=(SqStack*)malloc(sizeof(SqStack));
    (*S)->base = (BSTree**)malloc(STACK_INIT_SIZE*sizeof(BSTree*));
    (*S)->stacksize = STACK_INIT_SIZE;
    (*S)->top = (*S)->base;
    return OK;
}
/**
 * Push the BSTree Node @bstp into the SqStack @S 
 * @param    S                        SqStack
 * @param    tnp                      BSTree Node which need to push into the SqStack @S
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:23:31+0800
 */
Status Push(SqStack *S,BSTree* tnp)
{
    if((S->top - S->base) == S->stacksize)
    {
        if(!(S->base = (BSTree**)realloc(S->base,(S->stacksize+STACK_INCREMENT)*sizeof(BSTree*))))
            return(MEMORY_OVERFLOW);
        S->top = S->base + S->stacksize; 	//updating the top is very important
        S->stacksize += STACK_INCREMENT;
        *S->top = tnp;
    }
    *S->top = tnp;
    S->top++;
    return OK;
}
/**
 * Get the top element of SqStack @S
 * @param    S                        SqStack
 * @param    p                        the address of BSTree node to store the top element  
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:29:10+0800
 */
Status GetTop(SqStack *S,BSTree** p)
{
    S->top--;
    *p = *(S->top);
    S->top++;
    return OK;
}
/**
 * pop out the top element of SqStack @S,in the meantime make the top pointer of @S move down a unit step
 * @param    S                        SqStack
 * @param    p                        the address of BSTree node to store the top element 
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:32:13+0800
 */
Status Pop(SqStack *S,BSTree** p)
{
    S->top--;
    *p = *(S->top);
    return OK;
}
/**
 * to judge that if the SqStack @S is empty
 * @param    S                        SqStack
 * @return                            if @S is empty, return TRUE, else return ERROR    
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:36:31+0800
 */
Bool StackEmpty(SqStack* S)
{
    if((S->top == S->base))
        return TRUE;
    else
        return FALSE;
}
/**
 * Free(Destroy) the SqStack @S
 * @param    S                        SqStack   
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:37:48+0800
 */
Status FreeStack(SqStack *S)
{
    free(S->base);
    free(S);
    return OK;
}
/**
 * Search the word named @key in the Binary Sorted Tree
 * @param    key                      name of the word to search
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:38:39+0800
 */
Status SearchBST(char* key)
{
    if(BST_head == NULL)
    {
        strcpy(errbuff,"The BSTree doesn't exists!\n");
        return ERROR;
    }
    BSTree *bstp;
    bstp=SearchSubTree(BST_head,key);
    if((bstp=SearchSubTree(BST_head,key)) != NULL)
    {
        Global_wp = &(bstp->head);
        Global_wpp = Global_wp->flag;
    }
    else
    {
        strcpy(errbuff,"Word not Founded!\n");
        return ERROR;
    }
    return OK;
}
/**
 * Searching the word named @key in the Subtree @T recursivly
 * @param    T                        Subtree
 * @param    key                      name of the word to search
 * @return                            if find the word in the SubTree return the BSTree Node which store the word, else return NULL
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:40:03+0800
 */
BSTree *SearchSubTree(BSTree* T, char* key)
{
    if(T == NULL)
        return NULL;
    int i = strcmp(key,T->head.word_name);
    if(i==0)
        return T;
    else if(i<0)
        return SearchSubTree(T->lp,key);
    else
        return SearchSubTree(T->rp,key);
}
/**
 * Insert the word which is named @str and appears in the page @page and row @row into the BSTree @BST_head
 * @param    str                      name of the word to search
 * @param    page                     number of the page where this word appears 
 * @param    row                      number of the row where this word appears
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:42:58+0800
 */
Status InsertBSTree(char *str, unsigned int page, unsigned int row)
{
    if(BST_head == NULL)
    {
        if((BST_head = (BSTree *)malloc(sizeof(BSTree))) == NULL)
            return(MEMORY_OVERFLOW);
        BST_head->head.word_name = NULL;
        BST_head->head.flag = NULL;
        BST_head->lp = NULL;
        BST_head->rp = NULL;
    }
    char* newword = (char *)malloc(sizeof(char) * (strlen(str)+1));
    strcpy(newword,str);
    /*InsertBSTree mode*/
    SaveWordToBSTree(newword,(page-1)*Page_Step + row, 1);
    return OK;
}
/**
 * Delete the word named @str in the BSTree @BST_head
 * @param    str                      name of the word to delete
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:45:34+0800
 */
Status DeleteBSTree(char *str)
{
    if(BST_head == NULL)
    {
        strcpy(errbuff,"The BSTree doesn't exists!\n");
        return ERROR;
    }
    BSTree* p = BST_head, *f = BST_head, *q, *s;
    word_place *wpp, *temp;
    int ret;
    while(p)
    {
        ret = strcmp(str,p->head.word_name);
        if(ret == 0)
            break;
        f = p;					//save the keynode's father
        p = (ret<0)?p->lp:p->rp;
    }
    if(p == NULL)
    {
        strcpy(errbuff,"This word doesn't exist in this text!\n");
        return ERROR;
    }
    if(p->lp==NULL && p->rp==NULL)/*if the node to delete has no subtree*/
    {
        if(p == BST_head)	/*only the root in the tree and going to be deleted*/
        {
            free(p->head.word_name);
            wpp = BST_head->head.flag;
            free(BST_head);
            BST_head = NULL;
        }
        else
        {
            if(p == f->lp)
                f->lp = NULL;
            else if(p == f->rp)
                f->rp = NULL;
            free(p->head.word_name);
            wpp = p->head.flag;
            free(p);
        }
    }
    else if(p->lp==NULL && p->rp!=NULL)/*if the node to delete just has right_child*/
    {
        if(p == BST_head)/*if BST_head is the node to delete*/
        {
            free(p->head.word_name);
            f = BST_head;
            wpp = BST_head->head.flag;
            BST_head = BST_head->rp;
            free(f);
        }
        else
        {
            if(p == f->lp)
                f->lp = p->rp;
            else if(p == f->rp)
                f->rp = p->rp;
            free(p->head.word_name);
            wpp = p->head.flag;
            free(p);
        }
    }
    else if(p->lp!=NULL && p->rp==NULL)/*if the node to delete just has left_child*/
    {
        if(p == BST_head)/*if BST_head is the node to delete*/
        {
            free(p->head.word_name);
            f = BST_head;
            wpp = BST_head->head.flag;
            BST_head = BST_head->lp;
            free(f);
        }
        else
        {
            if(p == f->lp)
                f->lp = p->lp;
            else if(p == f->rp)
                f->rp = p->lp;
            free(p->head.word_name);
            wpp = p->head.flag;
            free(p);
        }
    }
    else if(p->lp!=NULL && p->rp!=NULL)/*if the node to delete has both right_child and left_child*/
    {
        q = p;
        s = p->lp;
        while(s->rp != NULL)
        {
            q=s;
            s=s->rp;
        }
        free(p->head.word_name);
        wpp = p->head.flag;
        /*node s is the prior node of p in the inorder traversal, replace the node p with node s, so that the new tree is relatively balanced*/
        p->head = s->head;
        if(q != p)
            q->rp = s->lp;
        else	/*when the left child of p has no right_child, in other words  s==p->lp && q==p in this condition */
            q->lp = s->lp;
        free(s);
    }
    /*Finally free the word_place linklist of the word to delete*/
    while(wpp!=NULL)
    {
        temp = wpp->next;
        free(wpp);
        wpp = temp;
    }
    wordnum--;
    return OK;
}
/**
 * Free all the data structures whose rooms are dynamicly allocationed
 * @return   Status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:46:42+0800
 */
Status FreeAll()
{
    Destroy_SST();
    DestroyBSTree();
    if(Global_arr != NULL)
        free(Global_arr);
    if(global_wpparr != NULL)
        free(global_wpparr);
    if(global_textarr != NULL)
        free(global_textarr);
    Global_arr = NULL;
    global_wpparr = NULL;
    global_textarr = NULL;
    printf("\n\nThank you for your use, See you!\n");
    return OK;
}
/**
 * Write the unsorted SSTable into the file 
 * @return   status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:48:19+0800
 */
Status WriteSSTableFile()
{
    if(SST_head==NULL)
    {
       strcpy(errbuff,"Staic Search Table hasn't been established!\nWrite File failed!");
        return ERROR;
    }
    FILE *fp;
    word* wp = Global_arr;
    word_place* wpp;
    unsigned int rownum;

    if((fp=fopen(SSTableFile_FILENAME,"w")) == NULL)
    {
        strcpy(errbuff,"File open failed!");
        return(ERROR);
    }
    fprintf(fp, "%u\n",SST_head->length);
    for(int i=0;i<SST_head->length;i++)
    {
        fprintf(fp, "%s %u ",wp->word_name,wp->count);
        wpp = wp->flag;
        while(wpp != NULL)
        {
            rownum = (wpp->page - 1)*Page_Step + wpp->row;
            fprintf(fp, "%u ",rownum);
            wpp = wpp->next;
        }
        wp++;
    }
    fclose(fp);
    return OK;
}
/**
 * Read the file about the unsorted SST, and then rebuild the unsorted SST and sorted SST.
 * @return   status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:51:07+0800
 */
Status ReadSSTableFile()
{
    FILE *fp;
    unsigned int rownum, length, count;
    char str[MAXLENGTH];
    char* p;
    word* wp;
    word_place* wpp, *temp;
    if((fp=fopen(SSTableFile_FILENAME,"r")) == NULL)
    {
        strcpy(errbuff,"File open failed!\n");
        return(ERROR);
    }
    if(SST_head != NULL)
        Destroy_SST();
    if(Global_arr != NULL)
        free(Global_arr);
    if((SST_head = (SSTable *)malloc(sizeof(SSTable))) == NULL)
        return(MEMORY_OVERFLOW);
    fscanf(fp,"%u\n",&length);
    SST_head->length = SST_head->listsize = length;
    if((SST_head->head = (word *)malloc(sizeof(word)*SST_head->listsize)) == NULL)
        return(MEMORY_OVERFLOW);
    //*initial the pointers and word counter to NULL and 0*/
    memset(SST_head->head,0,SST_head->listsize*sizeof(word));
    wp = SST_head->head;
    for(int i=0;i<length;i++,wp++)
    {
        fscanf(fp,"%s %u ",str,&count);
        if((p=(char*)malloc(sizeof(char)*(strlen(str)+1))) == NULL)
            return(MEMORY_OVERFLOW);
        strcpy(p,str);
        wp->word_name = p;
        wp->count =	count;
        temp = wp->flag = NULL;
        for(int j=0;j<count;j++)
        {
            fscanf(fp,"%u ",&rownum);
            if((wpp = (word_place*)malloc(sizeof(word_place))) == NULL)
                return(MEMORY_OVERFLOW);
            /*caculate the page and row number of the new wordplace*/
            wpp->page = (rownum-1)/Page_Step + 1;
            wpp->row = rownum - (wpp->page-1) * Page_Step;
            /*Insert the new word_place in the corresponded list*/
            if(wp->flag == NULL)
            {
                wp->flag = temp = wpp;
                wpp->next = NULL;
            }
            else
            {
                temp->next = wpp;
                wpp->next = NULL;
                temp = temp->next;
            }
        }
    }
    if((Global_arr=(word*)malloc((SST_head->length)*sizeof(word))) == NULL)
        return(MEMORY_OVERFLOW);
    memcpy(Global_arr,SST_head->head,(SST_head->length)*sizeof(word));
    QuickSort_SST(SST_head->head,0,SST_head->length-1);
    fclose(fp);
    return OK;
}
/**
 * Write the BSTree struct into the file
 * @return   status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:52:14+0800
 */
Status WriteBSTreeFile()
{
    if(BST_head == NULL)
    {
        strcpy(errbuff,"Binary Search Tree hasn't been established!\nWrite File failed!\n");
        return ERROR;
    }
    FILE *fp;
    if((fp=fopen(BSTreeFile_FILENAME,"w")) == NULL)
    {
        strcpy(errbuff,"Open FILE failed!\n");
        return(ERROR);
    }
    PreOrderPrint(fp);
    fclose(fp);
    return OK;
}
/**
 * print the BSTree into the file @fp by preorder
 * @param    fp                       FILE pointer which open the file to store the BSTree data
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:53:15+0800
 */
Status PreOrderPrint(FILE * fp)
{
    char nullstr[3] = "#";
    word* wp;
    word_place* wpp;
    unsigned int rownum;
    BSTree *bstp = BST_head;
    SqStack *S;
    InitStack(&S);
    Push(S,bstp);
    while(!StackEmpty(S))
    {
        /*travel through the left tree*/
        while(GetTop(S,&bstp) && bstp)
        {
            wp=&(bstp->head);
            fprintf(fp, "%s %u ",wp->word_name,wp->count);
            wpp = wp->flag;
            while(wpp != NULL)
            {
                rownum = (wpp->page - 1)*Page_Step + wpp->row;
                fprintf(fp, "%u ",rownum);
                wpp = wpp->next;
            }
            Push(S,bstp->lp);
        }
        /*after arriving at the most left child in this Tree, meet the NULL node*/
        Pop(S,&bstp);
        fprintf(fp, "%s ",nullstr);
        if(!StackEmpty(S))
        {
            Pop(S,&bstp);
            Push(S,bstp->rp);
        }
    }
    FreeStack(S);
    return OK;
}
/**
 * Read BSTree File and rebuild the BSTree struct
 * @return   status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:55:04+0800
 */
Status ReadBSTreeFile()
{
    FILE *fp;
    if((fp=fopen(BSTreeFile_FILENAME,"r")) == NULL)
    {
        strcpy(errbuff,"File open failed!\n");
        return(ERROR);
    }
    if(BST_head != NULL)
        DestroyBSTree();
    ReCreatBiTree(&BST_head,fp);
    fclose(fp);
    return OK;
}
/**
 * Rebuild the BSTree by read the file which stores the information of BStree
 * @param    bstp                     the address of the BSTree node to store the root of the Rebuilded BSTree
 * @param    fp                       FILE pointer which open the BSTree file to read
 * @return                            status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:55:46+0800
 */
Status ReCreatBiTree(BSTree** bstp, FILE *fp)
{
    char str[MAXLENGTH];
    unsigned int rownum, count;
    char* p;
    word* wp;
    word_place* wpp, *temp;
    fscanf(fp,"%s ",str);
    if(feof(fp))	/*if EOF comes, feof returns a nonzero number*/
    {
        *bstp = NULL;
        return OK;
    }
    /*bstp is a leaf node*/
    if(strcmp(str,"#") == 0)
        *bstp = NULL;
    else
    {
        if(!((*bstp) = (BSTree*)malloc(sizeof(BSTree))))
            return(MEMORY_OVERFLOW);
        if((p=(char*)malloc(sizeof(char)*(strlen(str)+1))) == NULL)
            return(MEMORY_OVERFLOW);
        strcpy(p,str);
        wp = &((*bstp)->head);
        wp->word_name = p;
        fscanf(fp,"%u ",&count);
        wp->count = count;
        temp = wp->flag = NULL;
        for(int j=0;j<count;j++)
        {
            fscanf(fp,"%u ",&rownum);
            if((wpp = (word_place*)malloc(sizeof(word_place))) == NULL)
                return(MEMORY_OVERFLOW);
            /*caculate the page and row number of the new wordplace*/
            wpp->page = (rownum-1)/Page_Step + 1;
            wpp->row = rownum - (wpp->page-1) * Page_Step;
            /*Insert the new word_place in the corresponded list*/
            if(wp->flag == NULL)
            {
                wp->flag = temp = wpp;
                wpp->next = NULL;
            }
            else
            {
                temp->next = wpp;
                wpp->next = NULL;
                temp = temp->next;
            }
        }
        ReCreatBiTree(&((*bstp)->lp),fp);
        ReCreatBiTree(&((*bstp)->rp),fp);
    }
    return OK;
}
/**
 * Save the file path and the total row number of the file which is saved last time
 * @return   running status of running this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T09:58:36+0800
 */
Status saveTextFile()
{
    FILE *fp;
    if((fp=fopen(TEXT_FILENAME,"w")) == NULL)
    {
        strcpy(errbuff,"File open failed!\n");
        return(ERROR);
    }
    fprintf(fp,"%s\n%u",filepath, current_row);
    fclose(fp);
    return OK;
}
/**
 * read the book file whose path is @filepath and store its every sentence separated by '\n' into the @global_textarr
 * @return   running status of this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T10:00:02+0800
 */
Status creatText()
{
    FILE *fp;
    char pfile[300];
    if((fp=fopen(TEXT_FILENAME,"r")) == NULL)
    {
        strcpy(errbuff,"File open failed!\n");
        return(ERROR);
    }
    fgets(pfile,300,fp);
    /*transform the last '\n' into '\0'*/
    pfile[strlen(pfile)-1] = '\0';
    fscanf(fp,"%u",&current_row);
    fclose(fp);
    creatTextlist(pfile,current_row);
    /*update filename*/
    if(( filepath = (char*)malloc(sizeof(char) *(strlen(pfile)+1))) == NULL)
        return(MEMORY_OVERFLOW);
    strcpy(filepath,pfile);
    return OK;
}
/**
 * read the book file whose path is @textfile, total row number is @rownum, and store its every sentence separated by '\n' into the @global_textarr
 * @param    textfile                 full path of the text file
 * @param    rownum                   total row number of the text file
 * @return                            running status of this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T10:02:24+0800
 */
Status creatTextlist(char *textfile, unsigned rownum)
{
    FILE *fp;
    char **p, temp[300];
    if((fp=fopen(textfile,"r")) == NULL)
    {
        strcpy(errbuff,"Open TextFile of last time failed!\n");
        return(ERROR);
    }
    /*free the memory junk*/
    if(global_textarr != NULL)
    {
        p = global_textarr;
        for(int i=0;i<rownum;i++)
        {
            free(*p);
            p++;
        }
        free(global_textarr);
        global_textarr = NULL;
    }
    /*establish the text list*/
    if((global_textarr = (char**)malloc(sizeof(char*) * rownum)) == NULL)
        return(MEMORY_OVERFLOW);
    p = global_textarr;
    for(int i=0;i<rownum;i++)
    {
        fgets (temp , 300 , fp);
        if(( *p = (char*)malloc(sizeof(char)*(strlen(temp)+1))) == NULL)
            return(MEMORY_OVERFLOW);
        strcpy(*p,temp);
        p++;
    }
    fclose(fp);
    return OK;
}
/**
 * Aftering finding the word in the ADT, create a linklist of the word's word_place linklist
 * @param    wp                       the word struct pointer which is find
 * @return                            running status of this function
 * @auther   HuangWenJie
 * @datetime 2017-03-02T10:04:22+0800
 */
Status creatWpplist(word* wp)
{
    word_place *t, **p;
    if(global_wpparr != NULL)
        free(global_wpparr); 
    if((global_wpparr = (word_place**)malloc(sizeof(word_place*) * wp->count)) == NULL)
        return(MEMORY_OVERFLOW);
    t = wp->flag;
    p = global_wpparr;
    while(t != NULL)
    {
        *p = t;
        p++;
        t = t->next;
    }
    Global_wpp = *global_wpparr;
    global_wppp = global_wpparr;
    return OK;
}
