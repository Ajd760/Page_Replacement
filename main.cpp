/**************************************************************************************************************
Written by: Alan Doose
CS 433 HW 5
Dec 7 2018

Purpose: This program simulates 3 different page replacement algorithms - FIFO (First in First out), LRU (Least 
	Recently Used, and a Random victim page selection algorithm. The user must specify on the command line when
	running the program both the page size and the physical memory size to simulate. Logical memory size is
	set to MAX_VIRTUAL_MEM 134217728 (128 MB). 

Assumptions: It is assumed that the user will enter real values as parameters on the command line for the page
	size as well as the physical memory size. It is also assumed that this program will be run in a Unix
	environment because it depends on sys/time.h

Dependencies: This driver file depends on the following class files:
		PageTable.h
		page.h
		random.h
*************************************************************************************************************/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sys/time.h>
#include "PageTable.h"

//Prototypes for helper functions
bool checkPowerof2(int n);

/***** constants, globals, and definitions *******/
#define MAX_VIRTUAL_MEM 134217728 //maximum virtual memory is 128 MB = 134217728 Bytes (2^20 * 2^7 = 2^27 = 134217728)
#define MB_IN_BYTES 1048576 //1 MB = 1048576 B (2^20), this is used to convert the physical memory parameter to Bytes

//Global statistic variables:
int gMemoryReferences = 0;
int gPageFaults = 0;
int gPageReplacements = 0;
int gFlushes = 0;
/************************************************/

int main(int argc, char* argv[])
{
  std::cout << "==========================================================================================" << std::endl;
  std::cout << "This is a simulation of different page replacement algorithms" << std::endl;
  std::cout << "This program analyzes the efficiency of FIFO, LRU, and random page replacement algorithms" << std::endl;
  std::cout << "\twith given page and physical memory sizes passed on the command line." << std::endl;
  std::cout << "Written by: Alan Doose" << std::endl;
  std::cout << "==========================================================================================" << std::endl;
  
  
  //Check input count validity
  if (argc != 3)
    {
      std::cout << "Improper program usage: You must enter exacly two command line arguments:" << std::endl;
      std::cout << "1st - page size in bytes between 256 and 8192 inclusive, and must be a power of 2" << std::endl;
      std::cout << "2nd - physical memory size in megabytes, must be a power of 2" << std::endl;
    }
  else //correct number of inputs, check their value validity
    {
      //get command line parameters
      int pageSize = atoi(argv[1]);
      int physicalMemoryinMB = atoi(argv[2]);
      int physMeminBytes = physicalMemoryinMB * MB_IN_BYTES; //convert physical memory parameter to bytes
      
      //Check input error conditions
      if (pageSize < 256 || pageSize > 8192 || !checkPowerof2(pageSize))
	{
	  std::cout << "Invalid page size parameter passed, must be between 256 and 8192, and must be a power of 2" << std::endl;
	}
      else if (!checkPowerof2(physicalMemoryinMB))
	{
	  std::cout << "Invalid physical memory size parameter passed, must be a power of 2" << std::endl;
	}
      
      else //inputs are valid, simulation starts in here
	{
	  std::ifstream fin; //input file stream for the references.txt file
	  std::string fileName = "references.txt";
	  fin.open(fileName.c_str()); //open the input file
	  if (!fin) //check input file was opened successfully
	    {
	      std::cout << "Error opening references.txt input file - ensure file is in same directory as this program" << std::endl;
	    }
	  else //no problem with the file
	    {
	      int numberOfFrames = physMeminBytes / pageSize; //number of frames in physical memory
	      std::cout << "Page Size: " << pageSize << " B" << std::endl;
	      std::cout << "Phys Mem Size: " << physMeminBytes << " B" << std::endl;
	      std::cout << "Number of Frames: " << numberOfFrames << std::endl;
	      
	      //create the page table with given parameters
	      PageTable page_table(pageSize, MAX_VIRTUAL_MEM, numberOfFrames);
	      
	      std::cout << "Page Table size: " << page_table.getPageTableSize() << std::endl;
	      
	      timeval* startTime = new timeval();
	      timeval* currentTime = new timeval(); //will be used to get elapsed time by subtracting startTime from this
	      
	      int nextReference = 0; //container for reading virtual memory references from the references.txt file
	      int page_num_referenced = 0; //container for holding the computed page number from each logical address
	      bool read = true; //checked for every reference, if this is false then it is a write reference
	      int pageNumtoRemove = 0; //container for holding a reference to a page to be evicted
	      
	      /***********FIFO simulation*****************************************************************************/
	      std::cout << "Starting Simulation for FIFO Algorithm..." << std::endl;
	      gettimeofday(startTime, NULL); //get start time right before simulation
	      fin >> nextReference; //grab first reference
	      //std::cout << "First ref is: " << nextReference << std::endl;
	      while (fin) //read until eof
		{
		  //get page number from virtual reference address (not computing offset since not required for simulation)
		  page_num_referenced = nextReference / pageSize;
		  
		  //check if this is a read or write
		  if (nextReference % 2 == 0)
		    read = true;
		  else read = false;
		  
		  //Check if referenced memory is in the page table
		  if (page_table.checkPageinTable(page_num_referenced))
		    {
		      //if a read, handle
		      if (read)
			{/*nothing to do!*/}
		      else//else it is a write, handle (set dirty flag)
			{
			  page_table[page_num_referenced]->dirty = true;
			}
		      
		    }
		  else //handle if it is not already in page table
		    {
		      //check if valid reference, if not, abort
		      if (nextReference > MAX_VIRTUAL_MEM)
			{
			  std::cout << "Invalid logical memory reference encountered! --> " << nextReference << " <-- Skipping this reference..." << std::endl;
			  break;
			}
		      else //it is a valid memory reference, so add to table
			{
			  //check if no free frames
			  if (page_table.mainMemisFull())
			    {
			      //if it is, replace based on FIFO (check flush on replaced page)
			      pageNumtoRemove = page_table.giveFIFOtoEvict();
			      
			      if (page_table[pageNumtoRemove]->dirty) //check if this is a flush
				gFlushes++;
			      
			      page_table.replace(pageNumtoRemove, page_num_referenced, 0); //time parameter set to 0 since FIFO doesn't care about time
			      
			      //check read or write and handle
			      if (read)
				{/*nothing to do!*/}
			      else //write reference
				{
				  //if a write then the referenced page is marked dirty
				  page_table[page_num_referenced]->dirty = true;
				}
			      gPageReplacements++;
			    }
			  else //else main mem is not full, add to table normally (and set page valid)
			    {
			      page_table.addPagetoTable(page_num_referenced, 0, page_table.giveFreeFrame()); //time parameter set to 0 since FIFO doesn't care about time
			      //check if read or write and handle
			      if (read)
				{/*nothing to do!*/}
			      else
				{
				  //if a write then the referenced page is marked dirty
				  page_table[page_num_referenced]->dirty = true;
				}
			    }
			}
		      gPageFaults++; //page was not in memory so increment page fault counter
		    }
		  gMemoryReferences++;
		  fin >> nextReference; //get next reference before restarting loop
		}//END FIFO Simulation loop
	      
	      //Print results for FIFO
	      gettimeofday(currentTime, NULL); //get end time
	      int endtimeTotaluS = ((currentTime->tv_sec*1000000) + currentTime->tv_usec) - ((startTime->tv_sec*1000000) + startTime->tv_usec);
	      int endTimeS = endtimeTotaluS / 1000000;
	      int endTimeuS = endtimeTotaluS % 1000000;
	      std::cout << "End of FIFO simulation\n";
	      std::cout << "Total Time elapsed: " << endTimeS << " seconds, " << endTimeuS << " microseconds." << std::endl;
	      std::cout << "Total memory references: " << gMemoryReferences << std::endl;
	      std::cout << "Total page faults: " << gPageFaults << std::endl;
	      std::cout << "Total page replacements: " << gPageReplacements << std::endl;
	      std::cout << "Total page flushes: " << gFlushes << std::endl;
	      std::cout << "\n\n";
	      /***********END FIFO simulation*********************/
	      
	      
	      /***********LRU simulation START*************************/
	      
	      //reset the page table and physical memory states
	      page_table.reset(pageSize, MAX_VIRTUAL_MEM, numberOfFrames);
	      
	      //reset Global statistic variables:
	      gMemoryReferences = 0;
	      gPageFaults = 0;
	      gPageReplacements = 0;
	      gFlushes = 0;
	      
	      //reset the file stream to the beginning of the references.txt file for the next algorithm
	      fin.clear();
	      fin.seekg(0, std::ios::beg);
	      fin >> nextReference; //grab first reference
		  
	      gettimeofday(startTime, NULL); //get start time right before simulation
	      std::cout << "Starting Simulation for LRU Algorithm...\n";
	      while (fin) //read until eof
		{
		  //get page number from virtual reference address (not computing offset since not required for simulation)
		  page_num_referenced = nextReference / pageSize;
		  
		  //check if this is a read or write
		  if (nextReference % 2 == 0)
		    read = true;
		  else read = false;
		  
		  //Check if referenced memory is in the page table
		  if (page_table.checkPageinTable(page_num_referenced))
		    {
		      //if a read, handle
		      if (read)
			{/*nothing to do!*/}
		      else//else it is a write, handle (set dirty flag)
			{
			  page_table[page_num_referenced]->dirty = true;
			}
			  //update the LRU list with this reference at the front (and search for and delete other entries of this reference later in the list)
			  page_table.updateLRU(page_num_referenced);
		      
		    }
		  else //handle if it is not already in page table
		    {
		      //check if valid reference, if not, abort
		      if (nextReference > MAX_VIRTUAL_MEM)
			{
			  std::cout << "Invalid logical memory reference encountered! --> " << nextReference << " <-- Skipping this reference..." << std::endl;
			  break;
			}
		      else //it is a valid memory reference, so add to table
			{
			  //check if no free frames
			  if (page_table.mainMemisFull())
			    {
			      //if it is, replace based on LRU
			      pageNumtoRemove = page_table.giveLRUtoEvict();
			      
			      if (page_table[pageNumtoRemove]->dirty) //check if this is a flush
				gFlushes++;
			      
			      page_table.replaceLRU(pageNumtoRemove, page_num_referenced, 0);
			      
			      //check read or write and handle
			      if (read)
				{/*nothing to do!*/}
			      else //write reference
				{
				  //if a write then the referenced page is marked dirty
				  page_table[page_num_referenced]->dirty = true;
				}
			      gPageReplacements++;
			    }
			  else //else main memory is not full, add to table normally (and set page valid)
			    {
			      page_table.addPagetoTableLRU(page_num_referenced, 0, page_table.giveFreeFrame());
								//check if read or write and handle
			      if (read)
				{/*nothing to do!*/}
			      else
				{
				  //if a write then the referenced page is marked dirty
				  page_table[page_num_referenced]->dirty = true;
				}
			    }
			}
		      gPageFaults++; //page was not in memory so increment page fault counter
		    }
		  gMemoryReferences++;
		  fin >> nextReference; //get next reference before restarting loop
		}//END LRU Simulation loop
	      
	      //Print results for LRU
	      gettimeofday(currentTime, NULL); //get end time
	      endtimeTotaluS = ((currentTime->tv_sec*1000000) + currentTime->tv_usec) - ((startTime->tv_sec*1000000) + startTime->tv_usec);
          endTimeS = endtimeTotaluS / 1000000;
          endTimeuS = endtimeTotaluS % 1000000;
	      std::cout << "End of LRU simulation\n";
	      std::cout << "Total Time elapsed: " << endTimeS << " seconds, " << endTimeuS << " micro seconds." << std::endl;
	      std::cout << "Total memory references: " << gMemoryReferences << std::endl;
	      std::cout << "Total page faults: " << gPageFaults << std::endl;
	      std::cout << "Total page replacements: " << gPageReplacements << std::endl;
	      std::cout << "Total page flushes: " << gFlushes << std::endl;
	      std::cout << "\n\n";
	      /***********END LRU simulation********************/
	      
	      
	      /***********Random simulation********************/
	      //reset the page table and physical memory states
	      page_table.reset(pageSize, MAX_VIRTUAL_MEM, numberOfFrames);
	      
	      //reset Global statistic variables:
	      gMemoryReferences = 0;
	      gPageFaults = 0;
	      gPageReplacements = 0;
	      gFlushes = 0;
	      
	      //reset the file stream to the beginning of the references.txt file for the next algorithm
	      fin.clear();
	      fin.seekg(0, std::ios::beg);
	      fin >> nextReference; //grab first reference

	      std::cout << "Starting Simulation for Random Algorithm:" << std::endl;
	      gettimeofday(startTime, NULL); //get start time right before simulation
	      while (fin) //read until eof
		{
		  //get page number from virtual reference address (not computing offset since not required for simulation)
		  page_num_referenced = nextReference / pageSize;
		  
		  //check if this is a read or write
		  if (nextReference % 2 == 0)
		    read = true;
		  else read = false;
		  
		  //time(&currentTime); //get current time for use in LRU algorithm
		  
		  //Check if referenced memory is in the page table
		  if (page_table.checkPageinTable(page_num_referenced))
		    {
		      //if a read, handle
		      if (read)
			{/*nothing to do!*/}
		      else//else it is a write, handle (set dirty flag)
			{
			  page_table[page_num_referenced]->dirty = true;
			}
		    }
		  else //handle if it is not already in page table
		    {
		      //check if valid reference, if not, abort
		      if (nextReference > MAX_VIRTUAL_MEM)
			{
			  std::cout << "Invalid logical memory reference encountered! --> " << nextReference << " <-- Skipping this reference..." << std::endl;
			  break;
			}
		      else //it is a valid memory reference, so add to table
			{
			  //check if no free frames
			  if (page_table.mainMemisFull())
			    {
			      //if it is, replace randomly
			      pageNumtoRemove = page_table.giveRandomtoEvict();
			      
			      if (page_table[pageNumtoRemove]->dirty) //check if this is a flush
				gFlushes++;
			      
			      page_table.replace(pageNumtoRemove, page_num_referenced, 0); //time parameter set to 0 since random doesnt care when pages were last used
			      
			      //check read or write and handle
			      if (read)
				{/*nothing to do!*/}
			      else //write reference
				{
				  //if a write then the referenced page is marked dirty
				  page_table[page_num_referenced]->dirty = true;
				}
			      gPageReplacements++;
			    }
			  else //else main memory is not full, add to table normally (and set page valid)
			    {
			      page_table.addPagetoTable(page_num_referenced, 0, page_table.giveFreeFrame()); //time parameter set to 0 since random doesnt care when pages were last used
			      //check if read or write and handle
			      if (read)
				{/*nothing to do!*/}
			      else
				{
									//if a write then the referenced page is marked dirty
				  page_table[page_num_referenced]->dirty = true;
				}
			    }
			}
		      gPageFaults++; //page was not in memory so increment page fault counter
					}
		  gMemoryReferences++;
		  fin >> nextReference; //get next reference before restarting loop
		}//END LRU Simulation loop
	      
				 //Print results for LRU
	      gettimeofday(currentTime, NULL); //get end time
	      endtimeTotaluS = ((currentTime->tv_sec*1000000) + currentTime->tv_usec) - ((startTime->tv_sec*1000000) + startTime->tv_usec);
              endTimeS = endtimeTotaluS / 1000000;
              endTimeuS = endtimeTotaluS % 1000000;
	      std::cout << "End of Random simulation\n";
	      std::cout << "Total Time elapsed: " << endTimeS << " seconds, " << endTimeuS << " micro seconds." << std::endl;
	      std::cout << "Total memory references: " << gMemoryReferences << std::endl;
	      std::cout << "Total page faults: " << gPageFaults << std::endl;
	      std::cout << "Total page replacements: " << gPageReplacements << std::endl;
	      std::cout << "Total page flushes: " << gFlushes << std::endl;
	      std::cout << "\n\n";
	      
	      /***********END Random simulation***************/
	    }
	}
    }
  
  std::cout << "End of Simulations\n\n============================================\n";
  return 0;
}

//Helper function to check if a number is a power of 2
bool checkPowerof2(int n)
{
  //if a number, n, is power of 2 then the bitwise & of n and n-1 will be 0 (e.g. 1000 & 0111 = 0000)
  // I take the logical not of that result so that I can logical && it with the original number
  // I take the logical && of the result with the original number in case n was 0
  if (n && (!(n&(n - 1))))
    return true;
  else return false;
}
