#include <stdio.h>
#include <stdlib.h>
#include "storage_mgr.h"
#include "dberror.h"


/****************************************************************
 *Function Name: initStorageManager
 *
 * Description: Initialise Storage Manager
 * 
 * Parameter: void
 * 
 * Return:
 *     void 
 ***************************************************************/
void initStorageManager(void){
    printf("***Initialising Storage Manager***");
}

/****************************************************************
 *Function Name: createPageFile
 *
 * Description: Create new Page file and fill with '\0' bytes.
 * 
 * Parameter: 
 *        char *fileName
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC createPageFile(char *fileName){
    FILE *fp;
    //open file in write binary mode
    fp=fopen(fileName, "wb");
    if(fp==NULL){
        printf("unable to open file");
        return RC_FILE_NOT_FOUND;
    }
    int i;
    //fill new page with '\0' bytes
    for(i=0;i<PAGE_SIZE;i++){
        fwrite("\0",sizeof(char),1,fp);
        
    }
    //close file
    fclose(fp);
    return RC_OK; 
}

/****************************************************************
 *Function Name: openPageFile
 *
 * Description: Open an existing page file and initialize file handle
 * 
 * Parameter: 
 *        char *fileName
 *        SM_FileHandle *fHandle
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC openPageFile(char *fileName, SM_FileHandle *fHandle){
    FILE *fp;
    int size;
    //Open an existing file in read mode
    fp=fopen(fileName,"rb+");
    if(fp==NULL){
        printf("unable to open file");
        //return error if file does not exist
        return RC_FILE_NOT_FOUND;
    }
    //Initialize file handle field
    fHandle->fileName =fileName;
    fHandle->curPagePos=0;
    fseek(fp, 0L, SEEK_END);
    size=ftell(fp);
   
    fHandle->totalNumPages=size/PAGE_SIZE;
    fseek(fp, 0, SEEK_SET);
    fHandle->mgmtInfo=fp;
    return RC_OK;
}


/****************************************************************
 *Function Name: closePageFile
 *
 * Description: Close an open page file
 * 
 * Parameter: 
 *       SM_FileHandle *fHandle
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC closePageFile(SM_FileHandle *fHandle){
    //close opened page file
    fclose(fHandle->mgmtInfo);
    return RC_OK;
    
}

/****************************************************************
 *Function Name: destroyPageFile
 *
 * Description: Delete page file
 * 
 * Parameter: 
 *       char *fileName
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC destroyPageFile(char *fileName){
    remove(fileName);
    return RC_OK;
}

/****************************************************************
 *Function Name: readBlock
 *
 * Description: Read pageNumth block
 * 
 * Parameter: 
 *        int pageNum
 *        SM_FileHandle *fHandle
 *        SM_PageHandle memPage
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC readBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check if pageNum is greater than totalNumPages
    if(pageNum> fHandle->totalNumPages){
        //return error if pageNum is greater
      return RC_READ_NON_EXISTING_PAGE;  
    }
    //move file pointer to appropriate position
    fseek(fHandle->mgmtInfo, pageNum*PAGE_SIZE, SEEK_SET);
    //reads a block of size PAGE_SIZE
    fread((void*)memPage, PAGE_SIZE, 1,fHandle->mgmtInfo);
    //Update curPagePos to pageNum+1
    fHandle->curPagePos=pageNum+1; 
    return RC_OK;

}


/****************************************************************
 *Function Name: getBlockPos
 *
 * Description: Return current page position
 * 
 * Parameter: 
 *        SM_FileHandle *fHandle
 * 
 * Return:
 *       int
 ***************************************************************/
int getBlockPos(SM_FileHandle *fHandle){
    return fHandle->curPagePos; 
}


/****************************************************************
 *Function Name: readFirstBlock
 *
 * Description: Read first block
 * 
 * Parameter: 
 *        SM_FileHandle *fHandle
 *        SM_PageHandle memPage
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Move file pointer to first block
    fseek(fHandle->mgmtInfo, 0, SEEK_SET);
    //read first block 
    fread((void*)memPage, PAGE_SIZE, 1,fHandle->mgmtInfo);
    //update current page position
    fHandle->curPagePos=1; 
    return RC_OK;

}


/****************************************************************
 *Function Name: readPreviousBlock
 *
 * Description: Read previous block
 * 
 * Parameter: 
 *        SM_FileHandle *fHandle
 *        SM_PageHandle memPage
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check if current page position is a valid position
    if((fHandle->curPagePos-1)<0 )
       return RC_READ_NON_EXISTING_PAGE;
    //move file pointer to previous block position
    fseek(fHandle->mgmtInfo, (fHandle->curPagePos-1)*PAGE_SIZE, SEEK_SET);
    //read previous block
    fread((void*)memPage, PAGE_SIZE, 1,fHandle->mgmtInfo);
    fseek(fHandle->mgmtInfo, -1*PAGE_SIZE, SEEK_CUR);
    //decrease current page position by 1
    fHandle->curPagePos=fHandle->curPagePos-1; 
    return RC_OK;

}

/****************************************************************
 *Function Name: readCurrentBlock
 *
 * Description: Read current block
 * 
 * Parameter: 
 *        SM_FileHandle *fHandle
 *        SM_PageHandle memPage
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    //move file pointer to current block position
    fseek(fHandle->mgmtInfo, 0, SEEK_CUR);
    //read current block
    fread((void*)memPage, PAGE_SIZE, 1,fHandle->mgmtInfo);
    fseek(fHandle->mgmtInfo, -1*PAGE_SIZE, SEEK_CUR);
    //Update current page position
    fHandle->curPagePos=fHandle->curPagePos; 
    return RC_OK;

}

/****************************************************************
 *Function Name: readNextBlock
 *
 * Description: Read Next block
 * 
 * Parameter: 
 *        SM_FileHandle *fHandle
 *        SM_PageHandle memPage
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check if current page position is greater than total num pages
    if(fHandle->curPagePos>fHandle->totalNumPages)
        return RC_READ_NON_EXISTING_PAGE;//return error if curpagePos is greater
    //Move file pointer to next block position
    fseek(fHandle->mgmtInfo, (fHandle->curPagePos+1)*PAGE_SIZE, SEEK_SET);
    //read next block
    fread((void*)memPage, PAGE_SIZE, 1,fHandle->mgmtInfo);
    fseek(fHandle->mgmtInfo, (fHandle->curPagePos+1)*PAGE_SIZE, SEEK_SET);
    //increase the current page position by 1
    fHandle->curPagePos=fHandle->curPagePos+1; 
    return RC_OK;

}


/****************************************************************
 *Function Name: readLastBlock
 *
 * Description: Read last block
 * 
 * Parameter: 
 *        SM_FileHandle *fHandle
 *        SM_PageHandle memPage
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Move file pointer to last block position
    fseek(fHandle->mgmtInfo, -1*PAGE_SIZE, SEEK_END);
    //read last block
    fread((void*)memPage, PAGE_SIZE, 1,fHandle->mgmtInfo);
    //update current page position
    fHandle->curPagePos=fHandle->totalNumPages; 
    return RC_OK;

}

/****************************************************************
 *Function Name: writeBlock
 *
 * Description: write pageNumth block
 * 
 * Parameter: 
 *        int pageNum
 *        SM_FileHandle *fHandle
 *        SM_PageHandle memPage
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    //check if pageNum is greater than totalNumPages
    if(pageNum>fHandle->totalNumPages){
       return RC_WRITE_FAILED;//return error if pageNum is greater  
    }
    //Move file pointer to appropriate position
    fseek(fHandle->mgmtInfo, pageNum*PAGE_SIZE, SEEK_SET);
    fwrite((void*)memPage, PAGE_SIZE, 1,fHandle->mgmtInfo);
    //Update current page position to pageNum+1
    fHandle->curPagePos=pageNum+1; 
    return RC_OK;
}


/****************************************************************
 *Function Name: writeCurrentBlock
 *
 * Description: write current block
 * 
 * Parameter: 
 *        SM_FileHandle *fHandle
 *        SM_PageHandle memPage
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage){
    //Move file pointer to current block position
    fseek(fHandle->mgmtInfo, 0, SEEK_CUR);
    fwrite((void*)memPage, PAGE_SIZE, 1,fHandle->mgmtInfo);
    fseek(fHandle->mgmtInfo, -1*PAGE_SIZE, SEEK_CUR);
    //update current Page position
    fHandle->curPagePos=fHandle->curPagePos; 
    return RC_OK;

}

/****************************************************************
 *Function Name: appendEmptyBlock
 *
 * Description: add new block
 * 
 * Parameter: 
 *        SM_FileHandle *fHandle
 *
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC appendEmptyBlock(SM_FileHandle *fHandle){
 
    if(fHandle->mgmtInfo==NULL){
        printf("unable to open file");
        return RC_FILE_NOT_FOUND;
    }
    char *newPage;
    //allocate a block of memory and set it to zero
    newPage = (char *) calloc(PAGE_SIZE, sizeof(char));
    int i;
    //move file pointer to end of file
    fseek(fHandle->mgmtInfo,0L,2);
    //append new page of zero bytes
    for(i=0;i<PAGE_SIZE;i++){
        fwrite(newPage,sizeof(char),1,fHandle->mgmtInfo);
        fseek(fHandle->mgmtInfo,0L,2);
    }
    int size;
    //update file handle field
    size=ftell(fHandle->mgmtInfo);
   
    fHandle->totalNumPages=size/PAGE_SIZE;
    //fseek(fHandle->mgmtInfo, 0, SEEK_SET);
    fHandle->curPagePos = fHandle->totalNumPages - 1;
    //printf("Cur pages:%d", fHandle->curPagePos);
    //free allocated memory
    free(newPage);
    return RC_OK;

}


/****************************************************************
 *Function Name: ensureCapacity
 *
 * Description: If file has less number of pages append additional pages 
 * 
 * Parameter: 
 *        int numberOfPages
 *        SM_FileHandle *fHandle
 * 
 * Return:
 *     RC: returned code 
 ***************************************************************/
RC ensureCapacity(int numberOfPages, SM_FileHandle *fHandle){
    //check if totalNumPages is less than numberOfPages
    if(fHandle->totalNumPages< numberOfPages){
      int appendPage,i;
      //determine the number of extra page to be appended
      appendPage=numberOfPages-fHandle->totalNumPages;
      for(i=0;i<appendPage;i++){
         appendEmptyBlock(fHandle);
        // printf("No of pages:%d", fHandle->totalNumPages);
      }
      //update file handle field
      fHandle->totalNumPages=numberOfPages;
      fHandle->curPagePos = fHandle->totalNumPages - 1;
      return RC_OK;
    }
    return RC_OK;
}
