/**************************************************************************************************************
Written by: Alan Doose
CS 433 HW 5
Dec 7 2018

Purpose: This is the implementation file for the Page Table data structure. This data structure is used to simulate
	a virtual memory page table in a computer system that uses demand paging.

Assumptions: It is assumed that this data structure will be used in a in a simulation for paged virtual
	memory in a computer system. This class also depends on:
			page.h
			random.h
*************************************************************************************************************/

#include "PageTable.h"

//Default constructor - should not be used
PageTable::PageTable()
{
	page_table_count = 0;
}

//This constructor takes the virtual memory size and page size (both in Bytes)
//  and sets the page table size to (max_virtual_mem / page_size)
PageTable::PageTable(int page_size, int max_virtual_mem, int free_frames)
{
	//set page table size equal to the number of pages that the virtual memory has (based on the page size)
	page_table_size = max_virtual_mem / page_size;
	page_table_entries = new page*[page_table_size]; //initialize the page table array to size large enough to hold all pages
	page_table_count = 0; //no reference in page table at start
	free_frame_count = 0;

	for (int i = 0; i < page_table_size; i++)
	{
		page_table_entries[i] = NULL;
	}
	
	//add free frame numbers to the free frames queue (all frames are free at start of simulation)
	for (int i = 0; i < free_frames; i++)
	{
		free_frame_list.push(i);
	}
	free_frame_count = free_frame_list.size();
	
}

//Destructor
PageTable::~PageTable()
{
	for (int i = 0; i < page_table_size; i++)
	{
		if (page_table_entries[i] != NULL)
		{
			delete page_table_entries[i];
			page_table_entries[i] = NULL;
		}
	}
}

//Resets the state of the page table and internal queues/lists that are used
//	in the simulations for the different page replacement algorithms
void PageTable::reset(int page_size, int max_virtual_mem, int free_frames)
{
	//set page table size equal to the number of pages that the virtual memory has (based on the page size)
	page_table_size = max_virtual_mem / page_size;
	//page_table_entries = new page*[page_table_size]; //initialize the page table array to size large enough to hold all pages
	page_table_count = 0; //no reference in page table at start
	free_frame_count = 0;

	for (int i = 0; i < page_table_size; i++)
	{
		delete page_table_entries[i];
		page_table_entries[i] = NULL;
	}

	//add free frame numbers to the free frames queue (all frames are free at start of simulation)
	for (int i = 0; i < free_frames; i++)
	{
		free_frame_list.push(i);
	}
	free_frame_count = free_frame_list.size();

	while (!FIFO_queue.empty())
		FIFO_queue.pop();
	while(!LRU_list.empty())
		LRU_list.pop_back();

	random_list.clear();
}

//Overloaded bracket operator for direcly accessing page entry pointers in the page table
page* PageTable::operator[](int i)
{
	if (i >= 0 && i < page_table_size)
		return page_table_entries[i];
	else return NULL;
}

//Returns a free frame index number if one is available, otherwise returns -1 to indicate physical memory is full
int PageTable::giveFreeFrame()
{
	if (free_frame_list.empty())
		return -1;
	else
	{
		//return the index of the free frame and pop it off the queue
		int freeFrame = free_frame_list.front();
		free_frame_list.pop();
		free_frame_count = free_frame_list.size();
		return freeFrame;
	}
}

//Adds the given pageNumber to the page table, updates that pages time, and assigns it to the given free frame
void PageTable::addPagetoTable(int pageNumber, int currentTime, int freeFrame)
{
	if (freeFrame != -1)
	{
		page* p = new page(pageNumber, freeFrame, currentTime);
		page_table_entries[pageNumber] = p; //"add" page to table
		page_table_entries[pageNumber]->valid = true; //reassert that page is valid (should have been set in constructor)
		page_table_entries[pageNumber]->dirty = false; //reassert that page is NOT dirty yet because it was just added to the table(should have been set in constructor)
		FIFO_queue.push(pageNumber);
		//LRU_list.updateLRU(pageNumber);
		random_list.push_back(pageNumber);
		page_table_count++;
	}
	else
	{
		//Debug msg - this should not happen if caller provides a free frame
		std::cout << "**Debug message: control reached call to addPagetoTable with a full main memory***\n"; 	
	}
}

//Adds the given pageNumber to the page table for the LRU algorithm, updates that pages time, and assigns it to the given free frame
void PageTable::addPagetoTableLRU(int pageNumber, int currentTime, int freeFrame)
{
	if (freeFrame != -1)
	{
		page* p = new page(pageNumber, freeFrame, currentTime);
		page_table_entries[pageNumber] = p; //"add" page to table
		page_table_entries[pageNumber]->valid = true; //reassert that page is valid (should have been set in constructor)
		page_table_entries[pageNumber]->dirty = false; //reassert that page is NOT dirty yet because it was just added to the table(should have been set in constructor)
		//FIFO_queue.push(pageNumber);
		updateLRU(pageNumber);
		//random_list.push_back(pageNumber);
		page_table_count++;
	}
	else
	{
		//Debug msg - this should not happen if caller provides a free frame
		std::cout << "**Debug message: control reached call to addPagetoTable with a full main memory***\n"; 	
	}
}

//Checks if a given page number is in the page table, returns true if so, false otherwise
bool PageTable::checkPageinTable(int pageNumber)
{
	if (page_table_entries[pageNumber] != NULL)
	{
		if (page_table_entries[pageNumber]->valid)
			return true;
		else return false;
	}
	return false; //if the pageNumber slot is NULL then it is not in the table
}

//Returns true if the page table is full, false otherwise
bool PageTable::isFull() const
{
	if (page_table_count >= page_table_size)
		return true;
	else return false;
}

//Returns true if main memory has no free frames, false otherwise
bool PageTable::mainMemisFull() const
{
	if (free_frame_list.empty()) //if the free frame list is empty, then the main memory is full
		return true;
	else return false;
}

//Adds page_ref to the front of the LRU list and searches through the entire list to delete duplicate entries
//	The rear element of the LRU list will always hold the LRU page number, while the front of the list will hold
//	the MRU page number
void PageTable::updateLRU(int page_ref)
{
	//LRU_list.remove(page_ref); //remove any previous reference to this page reference
	
	LRU_list.push_front(page_ref); //insert this page reference as the front item (most recently used - least recently used will be the rear element of the list)
	
}

//Returns a page number to evict based on a FIFO queue, returns -1 if the queue is empty
int PageTable::giveFIFOtoEvict()
{
	if (!FIFO_queue.empty())
	{
		int evictedPageNum = FIFO_queue.front();
		FIFO_queue.pop();
		//std::cout << "FIFO queue not empty, returning page num " << evictedPageNum << "\n";
		return evictedPageNum;
	}
	else return -1;
}

//Returns a page number to evict based on a LRU list, returns -1 if the list is empty
int PageTable::giveLRUtoEvict()
{
	if (!LRU_list.empty())
	{
		int evictedPageNum = LRU_list.back();
		LRU_list.pop_back();
		//LRU_list.remove(evictedPageNum);
		//std::cout << "LRU queue not empty, returning page num " << evictedPageNum << "\n";
		return evictedPageNum;
	}
	else return -1;
}

//Returns a random page number to evict from the pages in the page table, returns -1 if no pages are in main memory
int PageTable::giveRandomtoEvict()
{
	int randIndex = ranInt(0, random_list.size());
	int randomPage = random_list[randIndex];
	random_list.erase(random_list.begin()+ randIndex);
	return randomPage;
}

//Replaces the given page_to_remove for LRU in the page table with page_replacing_removed, param time sets time at which the page was added
void PageTable::replaceLRU(int page_to_remove, int page_replacing_removed, int time)
{
	//std::cout << "Removing page " << page_to_remove << " and replacing with " << page_replacing_removed << "\n";
	removePagefromTable(page_to_remove);
	addPagetoTableLRU(page_replacing_removed, time, giveFreeFrame());
}

//Replaces the given page_to_remove in the page table with page_replacing_removed, param time sets time at which the page was added
void PageTable::replace(int page_to_remove, int page_replacing_removed, int time)
{
	//std::cout << "Removing page " << page_to_remove << " and replacing with " << page_replacing_removed << "\n";
	removePagefromTable(page_to_remove);
	addPagetoTable(page_replacing_removed, time, giveFreeFrame());
}

//Removes the given page from the page table
void PageTable::removePagefromTable(int pageNum)
{
	free_frame_list.push(page_table_entries[pageNum]->frame_number);
	delete page_table_entries[pageNum];
	page_table_entries[pageNum] = NULL;
}
