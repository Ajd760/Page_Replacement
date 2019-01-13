/**************************************************************************************************************
Written by: Alan Doose
CS 433 HW 5
Dec 7 2018

Purpose: This is the header file for the page entry data structure. This data structure holds simple information
	pertaining to a page of memory such as page number, frame number when in main memory, time last accessed,
	and flags for dirty and valid bits

Assumptions: It is assumed that this data structure will be used in a Page Table data structure to simulate
	paged virtual memory in a computer system
*************************************************************************************************************/

#ifndef _PAGE
#define _PAGE

//This class represents a page table entry in a simulation for a computer using
//  a demand paging memory allocation scheme
class page
{
public:
	page(); //default constructor

	//Setting constructor for use in simulation
	page(int page_n, int frame_allocated, int time);
	~page(); //Destructor
	
	int last_page_access_time; //time at which the page was last accessed (in seconds)
	int page_num; //page number in the page table
	int frame_number; //frame that this page is in in main memory
	bool dirty; //true if write referenced
	bool valid; //true if in main memory

};

#endif