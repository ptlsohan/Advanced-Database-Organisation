 *********************Asignment-2:Buffer Manager*******************
*Sohan Manohar Patil - A20388048\
*Kavya Goli            - A20378676\
*Nivedita Merla     - A20391011\

/***************************************************************************************\
************Instructions to run******************\
***************************************************************************************/\
In terminal
1. Change the location to directory where files are stored.
2. Type make to compile file
3. Type ./test_assign2_1 to execute

/***************************************************************************************
************Function Description******************
***************************************************************************************
initBufferPool
1. Initialise all pageframe of buffer pool
2. Initialise buffer pool manager field with page file.

shutdownBufferPool
1. Flush all pages in buffer pool to disk
2. Check if there are any pinned pages in buffer. If so, return an error.
3. Free up all the resources associated with buffer pool.

forceFlushPool
1. Check if there are any dirty pages with fixcount zero. 
2. If so, write the page back to the disk.
3. Set dirty bit to zero.

markDirty
1. Find the page in buffer pool with page number pageNum. 
2. Mark the page as dirty.

unpinPage
1.  Find the page in buffer pool with page number pageNum. 
2. Decrease the fixcount of page by 1.

forcePage
1. Write current content of page back to the page file on disk.

pinPage
1. Find the requested page in buffer pool with page number pageNum.
2. If the buffer is empty ,load the page from disk into buffer.
3. If the requested page is in buffer, increase the fixcount and return page to client.
4. If the buffer is full, replace a page using appropriate page replacement strategy and the fixcount.

getFrameContents
1.  Returns an array of page numbers stored in pageframe.

getDirtyFlags
1. Returns an array of dirty bits for pages stored in pageframe of size numPages.

getFixCounts
1. Returns an array of fixcount for pages stored in pageframe of size numPages.

getNumReadIO
1. Returns the number of pages read from disk.

getNumWriteIO
1. Returns the number of pages written back to page file on disk.

************************************************************************
                         *** Additional Functions***
************************************************************************
initPageFrame
1. Create new node and initialize all the fields.
2. Creates a circular linked list of all nodes.

FIFO
1. If the buffer is full, Check if the requested page is in buffer. If it is present increase fixcount by 1.
2. If the page is not in the buffer, replace the page which is there for longest time in buffer with the requested page

LRU
1.If the buffer is full, Check if the requested page is in buffer. If it is present increase fixcount by 1.
2. If the page is not in the buffer, replace the page which has not been accessed recently.
