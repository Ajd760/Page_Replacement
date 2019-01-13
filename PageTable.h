/**************************************************************************************************************
Written by: Alan Doose
CS 433 HW 5
Dec 7 2018

Purpose: This is the header file for the Page Table data structure. This data structure is used to simulate 
	a virtual memory page table in a computer system that uses demand paging. 

Assumptions: It is assumed that this data structure will be used in a in a simulation for paged virtual 
	memory in a computer system. This class also depends on:
			page.h
			random.h
*************************************************************************************************************/

#ifndef _PAGE_TABLE
#define _PAGE_TABLE

#include <list>
#include <queue>
#include <iostream>
#include <time.h>
#include <vector>
#include "page.h"
#include "random.h"

//This class is implemented here with PageTable data structure to be used to less-than compare the time at which a 
//  page was added to the table for the LRU simulation
class LRUComparator
{
public:
	int operator() (const page* p1, const page* p2)
	{
		return p1->last_page_access_time > p2->last_page_access_time;
	}
};

//This class is used to simulate the page table in a demand paging scheme for virtual memory in a computer system
class PageTable
{
public:
	PageTable(); //default constructor

	//Setting constructor:
	//@param page_size - the page size in Bytes
	//@param max_virtual_mem - the maximum virtual memory of the simulated process
	//@param free_frames - the number of free frames in the simulated system
	PageTable(int page_size, int max_virtual_mem, int free_frames);
	//Destructor - cleans up poitners
	~PageTable();

	//Resets the state of the page table and internal queues/lists that are used
	//	in the simulations for the different page replacement algorithms
	void reset(int page_size, int max_virtual_mem, int free_frames);

	//Returns a free frame number if one is available, otherwise returns -1 to indicate physical memory is full
	int giveFreeFrame();
	//Returns a page number to evict based on a FIFO queue, returns -1 if the queue is empty
	int giveFIFOtoEvict();
	//Returns a page number to evict based on a LRU priority queue, returns -1 if the priority queue is empty
	int giveLRUtoEvict();
	//Returns a random page number to evict from the pages in the page table, returns -1 if no pages are in main memory
	int giveRandomtoEvict();
	//Replaces the given page_to_remove in the page table with page_replacing_removed, param time sets time at which the page was added
	void replace(int page_to_remove, int page_replacing_removed, int time);
	
	void replaceLRU(int page_to_remove, int page_replacing_removed, int time);
	//Removes the given page from the page table
	void removePagefromTable(int pageNum);
	//Adds the given pageNumber to the page table, updates that pages time, and assigns it to the given free frame
	void addPagetoTable(int pageNumber, int currentTime, int freeFrame);
	
	void addPagetoTableLRU(int pageNumber, int currentTime, int freeFrame);
	
	//Checks if a given page number is in the page table, returns true if so, false otherwise
	bool checkPageinTable(int pageNumber);
	//Returns true if the page table is full, false otherwise
	bool isFull() const;
	//Returns true if main memory has no free frames, false otherwise
	bool mainMemisFull() const;

	//Updates the LRU list with page_ref placed at front of list, then searches through list and deletes duplicate entries
	void updateLRU(int page_ref);
	//Returns the maxiumum page table size
	int getPageTableSize() { return page_table_size; }
	//Returns the number of elements in the page table
	int getPageTableCount() { return page_table_count; }
	//Returns the number of free frames in main memory
	int getNumberofFreeFrames() { return free_frame_list.size(); }

	//Overloaded bracket operator for direcly accessing page entry pointers in the page table
	page* operator[](int i); //bracket operator for accessing slots in the page table

private:
	page** page_table_entries; //array of page table entries, size figured at runtime based on page size
	std::queue<int> free_frame_list; //queue holding free frame numbers in main memory
	
	//when pages are added to the page table in the FIFO algorithm, a reference to their index is placed in this queue
	std::queue<int> FIFO_queue;
	
	//list based on time added. The back element of the list will hold the LRU element in the page table, while the front element of the list will hold the MRU element in the table
	std::list<int> LRU_list;

	
	//List for holding page numbers, will be accessed randomly to determine a random page to replace in the Random algorithm
	std::vector<int> random_list;

	int free_frame_count; //qty of free frames in main memory
	int page_table_size; //max size of the page table
	int page_table_count; //qty of entries in the page table
};

#endif