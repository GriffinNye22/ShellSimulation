/**
 *@file LogBinRWSemMonitor.cpp
 *@author Griffin Nye
 *@brief Implementation of a Monitors for the Readers-Writers problem using semaphores 
         Uses strong writers preference while allowing concurrent reader access 
				 for the bin file and weak readers preference while allowing concurrent 
				 reader access for the log file.
 */

#ifndef LOGBINRWSEMMONITOR	
#define LOGBINRWSEMMONITOR

#include "SemaphoreSet.cpp"
#include <unistd.h>

using namespace std;

/**
 *@brief Monitor for managing Readers and Writers to the bin and log files using semaphores.
 */
class LogBinRWSemMonitor {
	private:
		const int LOGREADERCOUNT = 0, LOGMUTEX = 1, NUMLOGREADERS = 2, BINREADERCOUNTMUTEX = 3, 
		          BINWRITERCOUNTMUTEX = 4, BINWRITERMUTEX = 5, NOBINREADERS = 6, 
							NOBINWRITERS = 7, NUMBINREADERS = 8, NUMBINWRITERS = 9;
		SemaphoreSet semSet; 
		
	public:
		
		/**
		 *@brief Constructs the LogBinMonitor object.
		 *@param semKey The key for the SemaphoreSet
		 */
		LogBinRWSemMonitor(int semKey) {
			semSet = SemaphoreSet(semKey, 10, IPC_CREAT | 0777);
		}//end LogBinMonitor
		
		/**
		 *@brief Initializes all semaphores in semSet to their default values.
		 */
		void init() {
			semSet.setAll(0);
			semSet.set(LOGREADERCOUNT,1);
			semSet.set(LOGMUTEX,1);			
			semSet.set(BINREADERCOUNTMUTEX,1);
			semSet.set(BINWRITERCOUNTMUTEX,1);
			semSet.set(BINWRITERMUTEX,1);
			//cout << "bin sems:" << endl;
			//semSet.print();
		}//end init
		
		/**
		 *@brief Performs the necessary synchronization to add a bin Reader and prepare it for reading from a critical section.
		 */
		void addBinReader() {
			//cout << "Reader Arrived" << endl;
			
			//Wait until previous reader is done updating numReaders
			semSet.wait(BINREADERCOUNTMUTEX);	

				//Wait for all writers to finish 
				if(semSet.get(NUMBINWRITERS) > 0) {
					cout << "READERS WAITING FOR WRITER(S) TO COMPLETE" << endl;
					semSet.wait(NOBINWRITERS);
					cout << "READERS GAINED ACCESS:" << endl;
					semSet.print();
				}//end if
				
				//Don't trigger wait on noReaders until all writers are done				
				semSet.signal(NUMBINREADERS); //NUMREADERS++
				cout << "Reader Added: " << semSet.get(NUMBINREADERS) << endl;
				
			//Signal next reader to update numReaders
			semSet.signal(BINREADERCOUNTMUTEX);
			
			cout << "Reader " << getpid() << " entering CS." << endl;
			
		}//end addReader
		
		/**
		 *@brief Performs the necessary synchronization to remove a Reader after it reads from a critical section.
		 */
		void remBinReader() {
			//TEST PRINTS
			cout << "Reader " << getpid() << " exiting CS." << endl;
			
			//Wait until previous reader is done updating numReaders
			semSet.wait(BINREADERCOUNTMUTEX);
			
				semSet.wait(NUMBINREADERS); //NUMBINREADERS--
				cout << "Reader Removed: " << semSet.get(NUMBINREADERS) << endl;
				
				//Wake up processes waiting on noReaders when last reader exits
				if(semSet.get(NUMBINREADERS) == 0 && semSet.getWaitCount(NOBINREADERS) > 0) {
					semSet.signal(NOBINREADERS);
				}//end if
			
			//Signal next reader to update numReaders
			semSet.signal(BINREADERCOUNTMUTEX);
			
		}//end remReader 
		
		/**
		 *@brief Performs the necessary synchronization to add a Writer and prepare it for writing to a critical section.
		 */
    void addBinWriter() {
			//cout << "WRITER ARRIVED:" << endl;
			
			//Wait until previous writer is done updating numWriters
	    semSet.wait(BINWRITERCOUNTMUTEX);
			
				semSet.signal(NUMBINWRITERS); //NUMBINWRITERS++
				cout << "Writer Added: " << semSet.get(NUMBINWRITERS) << endl;
			
			  //Wait for readers currently reading to finish
			  if(semSet.get(NUMBINREADERS) > 0) {	
			    cout << "WRITER WAITING FOR CURRENT READERS TO FINISH" << endl;
			    semSet.wait(NOBINREADERS);
			    cout << "WRITERS GAINED ACCESS:" << endl;
			    semSet.print();
			  }//end if
			
			//Signal next writer to update numWriters
			semSet.signal(BINWRITERCOUNTMUTEX);
			//Wait until previous writer is done writing
			semSet.wait(BINWRITERMUTEX);
			
			cout << "WRITER " << getpid() << " entering CS." << endl;
    }//end addWriter
		
		/**
		 *@brief Performs the necessary synchronization to remove a Writer after it writes to a critical section.
		 */
		void remBinWriter() {
			cout << "WRITER " << getpid() << " exiting CS." << endl;
			//Signal next writer to write
			semSet.signal(BINWRITERMUTEX);
			//Wait until previous writer is done updating numWriters
			semSet.wait(BINWRITERCOUNTMUTEX);
			
				semSet.wait(NUMBINWRITERS); //NUMBINWRITERS--
				cout << "Writer Removed: " << semSet.get(NUMBINWRITERS) << endl;
				
				//Wake up processes waiting on noBinWriters when last writer exits		
				if(semSet.get(NUMBINWRITERS) == 0 && semSet.getWaitCount(NOBINWRITERS) > 0) {
					semSet.signal(NOBINWRITERS);
				}//end if
			
			//Signal next writer to update numWriters
			semSet.signal(BINWRITERCOUNTMUTEX);
		}//end remWriter
	
		/**
		 *@brief Performs the necessary synchronization to add a Reader and prepare it for reading from a critical section.
		 */
		void addLogReader() {
			//Wait until previous reader is done updating numReaders
			semSet.wait(LOGREADERCOUNT);	
			
				//Increment numReaders
				semSet.signal(NUMLOGREADERS);
				//cout << "Reader Added" << endl;
				//semSet.print();
				
				
				//Wait for current writers to finish 
			  if(semSet.get(NUMLOGREADERS) == 1) {
					semSet.wait(LOGMUTEX);
					//cout << "Readers gained access" << endl;
				}//end if
				
			//Signal next reader to update numReaders
			semSet.signal(LOGREADERCOUNT);
			//cout << "Reader entered" << endl;
		}//end addReader
		
		/**
		 *@brief Performs the necessary synchronization to remove a Reader after it reads from a critical section.
		 */
		void remLogReader() {
			//cout << "Reader exited" << endl;
			
			//Wait until previous reader is done updating numReaders
			semSet.wait(LOGREADERCOUNT);
			
				//Decrement numReaders
				semSet.wait(NUMLOGREADERS);
				//cout << "Reader removed" << endl;
				//semSet.print();
				
				//Give writers access when last reader exits
				if(semSet.get(NUMLOGREADERS) == 0) {
					semSet.signal(LOGMUTEX);
				}//end if
			
			//Signal next reader to update numReaders
			semSet.signal(LOGREADERCOUNT);
		}//end remReader 
		
		/**
		 *@brief Performs the necessary synchronization to add a Writer and prepare it for writing to a critical section.
		 */
    void addLogWriter() {
			//Wait for previous writer to finish or readers to give back access
			semSet.wait(LOGMUTEX);
			//cout << "Writer entered" << endl;
    }//end addWriter
		
		/**
		 *@brief Performs the necessary synchronization to remove a Writer after it writes to a critical section.
		 */
		void remLogWriter() {
			//cout << "Writer exited" << endl;
			//Signal next writer to write or give access to readers
			semSet.signal(LOGMUTEX);
		}//end remWriter
	
};//end LogBinRWSemMonitor
#endif