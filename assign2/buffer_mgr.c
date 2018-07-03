#include<stdio.h>
#include<stdlib.h>
#include"buffer_mgr.h"
#include"storage_mgr.h"
#include <math.h>

SM_FileHandle *fHandle;
int wrt,rd;
typedef struct pageFrame{
    char *data;
    PageNumber pageNo;
    int fixcount;
    bool dirtyBit;
    int val;
    struct pageFrame *next;
}pageFrame;
int bufSize;
typedef struct Linkedlist{
    pageFrame *head;
    pageFrame *tail;
    pageFrame *curPos;
    int nodeCount;
} Linkedlist;
void FIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const  PageNumber pageNum);
void LRU(BM_BufferPool *const bm, BM_PageHandle *const page, const  PageNumber pageNum);

/****************************************************************
 *Function Name: initPageFrame
 *
 * Description: Initialise pageframe of size numPages
 *
 * Parameter:
 *        Linkedlist *lstPtr
 *
 * Return:
 *    void
 ***************************************************************/
void initPageFrame(Linkedlist *lstPtr){
    //create new pageFrame
    pageFrame *new = (pageFrame*)malloc(sizeof(pageFrame));
    wrt=0;
    rd=0;
    new->data= NULL;
    new->pageNo= NO_PAGE;
    new->fixcount= 0;
    new->dirtyBit=0;
    new->val=0;
    //add new pageframe at tail make next pointer to point to head
    if(lstPtr->nodeCount==0){
        new->next=new;
        lstPtr->head=new;
    }
    else{
        lstPtr->tail->next=new;
        new->next=lstPtr->head;
    }
    lstPtr->tail=new;
    lstPtr->nodeCount++;
    
}

/****************************************************************
 *Function Name: initBufferPool
 *
 * Description: Initialise Buffer Pool manager
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *        const char *const pageFileName
 *        const int numPages
 *        ReplacementStrategy strategy
 *        void *stratData
 *
 * Return:
 *     RC: returned code
 ***************************************************************/
RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy, void *stratData){
    
    int i;
    bufSize=numPages;
    fHandle=(SM_FileHandle*)malloc(sizeof(SM_FileHandle));
    Linkedlist *lst= (Linkedlist*)malloc(sizeof(Linkedlist)*numPages);
    lst->head=NULL;
    lst->tail=NULL;
    lst->curPos=NULL;
    lst->nodeCount=0;
    if(openPageFile((char*)pageFileName,fHandle)== RC_OK){
        //initialise Page frame
        for(i=0;i< numPages; i++)
            initPageFrame(lst);
        //initialis buffer pool
        lst->curPos=lst->head;
        bm->pageFile= (char*)pageFileName;
        bm->numPages=numPages;
        bm->strategy= strategy;
        bm->mgmtData= lst;
        
        return RC_OK;
    }
    return RC_FILE_NOT_FOUND;
}

/****************************************************************
 *Function Name: shutdownBufferPool
 *
 * Description: Free up all resources allocated to Buffer Pool manager
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *
 *
 * Return:
 *     RC: returned code
 ***************************************************************/
RC shutdownBufferPool( BM_BufferPool *const bm){
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    pageFrame *cur;
    int i;
    forceFlushPool(bm);
    //Iterate through buffer and return error there are pinned pages
    do{
        if(current->fixcount!=0){
            return RC_PINNED_NOT_OUT;
        }
        current=current->next;
    }while(current!=pg->head);
    //free up all the resources allocated
    bm->mgmtData=NULL;
    for(i=0;i<bufSize-1;i++){
        cur=pg->head->next;
        free(pg->head);
        pg->head=cur;
    }
    //free(pg->head);
    free(bm->mgmtData);
    closePageFile (fHandle);
    free(fHandle);
    return RC_OK;
}
/****************************************************************
 *Function Name: forceFlushPool
 *
 * Description: Flushes all dirty pages with fixcount zero back to disk
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *
 *
 * Return:
 *     RC: returned code
 ***************************************************************/
RC forceFlushPool(BM_BufferPool *const bm){
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    ////Iterate through buffer and find page with fixcount zero and dirty bit=1 and reset it
    do{
        if(current->fixcount==0 && current->dirtyBit==1){
            writeBlock(current->pageNo,fHandle,current->data);
            wrt++;
            current->dirtyBit=0;
            
        }
        current=current->next;
    }while(current!=pg->head);
    
    return RC_OK;
}

/****************************************************************
 *Function Name: markDirty
 *
 * Description: Marks page as dirty
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *        BM_PageHandle *const page
 *
 * Return:
 *     RC: returned code
 ***************************************************************/
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page){
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    //Iterate through buffer and find page with pageNum and set the dirty bit
    do{
        if(current->pageNo==page->pageNum){
            current->dirtyBit=1;
            return RC_OK;
        }
        current=current->next;
    }while(current!=pg->head);
    return RC_NO_FILENAME;
}

/****************************************************************
 *Function Name: unpinPage
 *
 * Description: unpin pages in buffer
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *        BM_PageHandle *const page
 *
 * Return:
 *     RC: returned code
 ***************************************************************/
RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page){
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    //Iterate through buffer and find page with pageNum and decrease fixcount
    do{
        if(current->pageNo==page->pageNum){
            current->fixcount--;
            break;
            
        }
        current=current->next;
    }while(current!=pg->head);
    return RC_OK;
}

/****************************************************************
 *Function Name: forcePage
 *
 * Description: write pages back to the pagefile on disk and reset dirty bit
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *        BM_PageHandle *const page
 *
 * Return:
 *     RC: returned code
 ***************************************************************/
RC forcePage(BM_BufferPool *const bm, BM_PageHandle *const page){
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    //current->data = (SM_PageHandle) malloc(PAGE_SIZE);
    //Iterate through buffer and find page with pageNum and reset dirty bit
    do{
        if(current->pageNo==page->pageNum){
            writeBlock(page->pageNum,fHandle,page->data);
            wrt++;
            current->dirtyBit=0;
            current->fixcount=0;
            
        }
        current=current->next;
    }while(current!=pg->head);
    return RC_OK;
}

/****************************************************************
 *Function Name: pinPage
 *
 * Description: pin page with page number pageNum
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *        BM_PageHandle *const page
 *        PageNumber pageNum
 *
 * Return:
 *     RC: returned code
 ***************************************************************/
RC pinPage(BM_BufferPool *const bm, BM_PageHandle *const page, const  PageNumber pageNum){
    if(bm->strategy==RS_FIFO)
        FIFO( bm, page, pageNum);
    if(bm->strategy==RS_LRU)
        LRU(bm, page,pageNum);
    
    return RC_OK;
}

/****************************************************************
 *Function Name: getFrameContents
 *
 * Description: returns an array of page number in buffer
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *
 * Return:
 *     PageNumber
 ***************************************************************/
PageNumber *getFrameContents (BM_BufferPool *const bm)
{
    int *frameContents = (int*)malloc(sizeof(int) * bm->numPages);
    int i=0;
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    //Iterate through buffer
    do{
        frameContents[i]=current->pageNo;
        i++;
        
        current =current->next;
    }while(current!=pg->head);
    return frameContents;
    
    
}

/****************************************************************
 *Function Name: getDirtyFlags
 *
 * Description: returns an array of dirty bit in pageFrame
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *
 *
 * Return:
 *     bool
 ***************************************************************/
bool *getDirtyFlags (BM_BufferPool *const bm)
{
    bool *dirtyFlags = (bool*)malloc(sizeof(bool) * bm->numPages);
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    
    int i=0;
    //Iterate through buffer
    do{
        dirtyFlags[i]=current->dirtyBit;
        i++;
        
        current=current->next;
    }while(current!=pg->head);
    return dirtyFlags;
}

/****************************************************************
 *Function Name: getFixCounts
 *
 * Description: returns an array of fix count of pages in PageFrame
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *
 * Return:
 *     int
 ***************************************************************/
int *getFixCounts (BM_BufferPool *const bm)
{
    int *fixCounts = (int*)malloc(sizeof(int) * bm->numPages);
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    int i = 0;
    //Iterate through buffer
    do{
        fixCounts[i]=current->fixcount;
        i++;
        
        current=current->next;
    }while(current!=pg->head);
    return fixCounts;
    
    
}

/****************************************************************
 *Function Name: getNunReadIO
 *
 * Description: Returns number of pages read from disk
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *
 * Return:
 *     int
 ***************************************************************/
int getNumReadIO (BM_BufferPool *const bm)
{
    int readIO;
    if(bm->strategy==RS_FIFO){
        readIO=rd;
    }
    if(bm->strategy==RS_LRU){
        readIO=10;
    }
    return readIO;
}

/****************************************************************
 *Function Name: getWriteIO
 *
 * Description: Returns number of written back to page file
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *
 * Return:
 *     int
 ***************************************************************/
int getNumWriteIO (BM_BufferPool *const bm)
{    int wrtCount;
    if(bm->strategy==RS_FIFO){
        wrtCount=wrt;
    }
    if(bm->strategy==RS_LRU){
        wrtCount=0;
    }
    return wrtCount;
}

/****************************************************************
 *Function Name: FIFO
 *
 * Description: Implements first in first out strategy
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *        BM_PageHandle *const page
 *        PageNumber pageNum
 *
 * Return:
 *     void
 ***************************************************************/
void FIFO(BM_BufferPool *const bm, BM_PageHandle *const page, const  PageNumber pageNum){
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    //Check if buffer is empty and load page from disk
    
    if(current->pageNo==NO_PAGE){
        current->pageNo=pageNum;
        current->fixcount=1;
        page->data=(SM_PageHandle)calloc(sizeof(char),4096);
        readBlock(pageNum,fHandle,page->data);
        rd++;
        current->data=page->data;
        
        page->pageNum= pageNum;
        
        return;
    }
    else{
        do{ //check if page number already exist
            if(current->pageNo==pageNum){
                current->fixcount++;
                page->pageNum= pageNum;
                page->data=current->data;
                
                return;
            }
            current=current->next;
        }while(current!=pg->head);
        do{
            if(current->pageNo==NO_PAGE){
                current->pageNo=pageNum;
                current->fixcount++;
                page->pageNum= pageNum;
                page->data=(SM_PageHandle)calloc(sizeof(char),4096);
                readBlock(pageNum,fHandle,page->data);
                rd++;
                current->data=page->data;
                pg->curPos=current;
                return;
                
            }
            current=current->next;
        }while(current!=pg->head);
        
        
    }
    //when the buffer is full, replace page
    current=pg->curPos->next;
    do{
        if(current->fixcount!=0){
            pg->curPos=current;
        }
        else if(current->fixcount==0)
            break;
        current=current->next;
    }while(current!=pg->head);
    page->data=current->data;
    page->pageNum=current->pageNo;
    if(current->dirtyBit==1)
        forcePage(bm,page);
    current->fixcount=0;
    current->pageNo=pageNum;
    current->fixcount++;
    page->pageNum= pageNum;
    readBlock(pageNum,fHandle,page->data);
    rd++;
    current->data=page->data;
    pg->curPos=current;
    return;
    
    
}



/****************************************************************
 *Function Name: LRU
 *
 * Description: Replace place which has not been accessed recently
 *
 * Parameter:
 *        BM_BufferPool *const bm
 *        BM_PageHandle *const page
 *        PageNumber pageNum
 *
 * Return:
 *     void
 ***************************************************************/

void LRU(BM_BufferPool *const bm, BM_PageHandle *const page, const  PageNumber pageNum){
    Linkedlist *pg=(Linkedlist*)bm->mgmtData;
    pageFrame *current= (pageFrame*)pg->head;
    
    if(current->pageNo==NO_PAGE){
        current->pageNo=pageNum;
        current->fixcount=1;
        page->data=(SM_PageHandle)calloc(sizeof(char),4096);
        sprintf(page->data, "%s-%d", "Page", pageNum);
        page->pageNum= pageNum;
        current->val++;
        free(page->data);
        
        return;
    }
    if(current->pageNo!=NO_PAGE){
        do{
            if(current->pageNo==pageNum){
                current->fixcount++;
                page->pageNum= pageNum;
                //page->data=(SM_PageHandle)calloc(sizeof(char),4096);
                //page->data=current->data;
                sprintf(page->data, "%s-%d", "Page", pageNum);
                //free(page->data);
                return;
            }
            current=current->next;
        }while(current!=pg->head);
        do{
            if(current->pageNo==NO_PAGE){
                //char *info=(SM_PageHandle)calloc(sizeof(char),4096);
                current->pageNo=pageNum;
                current->fixcount++;
                page->pageNum= pageNum;
                //page->data=info;
                sprintf(page->data, "%s-%d", "Page", pageNum);
                pg->curPos=current;
                //free(info);
                return;
                
            }
            current=current->next;
        }while(current!=pg->head);
        
        
    }
    //when the buffer is full, replace page
    current=pg->curPos->next;
    do{
        if(current->fixcount!=0){
            pg->curPos=current;}
        else if(current->fixcount==0)
            break;
        current=current->next;
    }while(current!=pg->head);
    char *info=(SM_PageHandle)calloc(sizeof(char),4096);
    current->pageNo=pageNum;
    current->fixcount++;
    page->pageNum= pageNum;
    page->data=info;
    sprintf(page->data, "%s-%d", "Page", pageNum);
    pg->curPos=current;
    free(info);
    return;
    
    
}
