# Page_Replacement
A main memory page replacement algorithm simulator

This program simulates 3 different page replacement algorithms - FIFO (First in First out), LRU (Least 
	Recently Used, and a Random victim page selection algorithm. The user must specify on the command line when
	running the program both the page size and the physical memory size to simulate. Logical memory size is
	set to MAX_VIRTUAL_MEM 134217728 (128 MB). 

Assumptions: It is assumed that the user will enter real values as parameters on the command line for the page
	size as well as the physical memory size. It is also assumed that this program will be run in a Unix
	environment because it depends on sys/time.h

*Note: must supply a memory references text file as input
