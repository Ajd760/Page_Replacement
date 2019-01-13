/**************************************************************************************************************
Written by: Alan Doose
CS 433 HW 5
Dec 7 2018

Purpose: This is the implementation file for the page entry data structure. This data structure holds simple 
information pertaining to a page of memory such as page number, frame number when in main memory, time last 
accessed, and flags for dirty and valid bits

Assumptions: It is assumed that this data structure will be used in a Page Table data structure to simulate
paged virtual memory in a computer system
*************************************************************************************************************/

#include "page.h"

//Default constructor - do not use
page::page()
{
	//default values, these will be set by the simulation right after a page is created and moved into the page table and main memory
	dirty = false;
	valid = false;
	frame_number = 0;
	last_page_access_time = 0;
	page_num = 0;
}

//Constructor for creating a page that is being brought into the page table
page::page(int page_n, int frame_allocated, int time)
{
	//set given values
	dirty = false;
	valid = true;
	frame_number = frame_allocated;
	last_page_access_time = time;
	page_num = page_n;
}

//Destructor
page::~page()
{
	//Nothing to do
}
