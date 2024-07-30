/**
 * @file SharedMemoryManager.cpp
 * @author Griffin Nye
 * @brief Wrapper Class Implementation of System V Shared Memory for use with the
 *        Distributed Point-To-Point Communications System. It should be noted
 *        that the term shared memory space refers to the entire allocated address
 *        space shared between all client processes while the term shared memory 
 *        block refers to a segment in the shared memory space containing a single 
 *        client process' data.
 * CSC552 Dr. Spiegel Spring 2020
 */

#include <chrono>
#include <ctime>
#include <sys/shm.h>

using namespace std::chrono;

// /**
 // *@class SharedMemoryManager
 // *@brief WrapperClass Implementation of UNIX Shared Memory for easier creation, storage, and retrieval.
 // */
// class SharedMemoryManager {
  // private:
	  // const int MAXMEMBLOCKS = 100;
		// int shmID, key, spaceSize, blockSize, permFlags;
		// void * spacePtr;
		// void * blockPtr;
	
	// public:
	
	  // /**
		 // *@struct sharedMemBlock
		 // *@brief Container struct of all fields in a client process' shared memory block.
		 // *@var sharedMemBlock::pid 
		 // * The pid of the client process using this block of shared memory.
		 // *@var sharedMemBlock::startTime
		 // * The current system time at the time of the client process' execution.
		 // *@var sharedMemBlock::numCommands
		 // * The number of commands this client process has issued to the server.
		 // *@var sharedMemBlock::lastMsgTime
		 // * The current system time when the client process last issued a command.
		 // */
	  // struct sharedMemBlock {
			// pid_t pid;
			// system_clock::time_point startTime;
			// int numCommands;
			// system_clock::time_point lastMsgTime;
		// }//end struct
		
		// /**
		 // *@brief Allocates the shared memory space and constructs the SharedMemory object.
		 // *@param key The key for the shared memory space.
		 // *@param perms The permission flags for the shared memory space.
 		 // */
		// SharedMemoryManager(int key, int perms) {
			// this->permFlags = perms;
			// this->spaceSize = sizeof(int) + (sizeof(sharedMemBlock) * MAXMEMBLOCKS) + 1;
			// int temp = shmget(key, size, perms);
			
			// if(temp == -1) {
				// perror("SharedMemory construction error");
			// } else {
				// this->shmID = temp;
			// }//end if
		// }//end SharedMemory
		
		// /**
		 // *@brief Attaches the first open shared memory address to the calling process.
		 // *@return Pointer to the first byte of the shared memory space. NULL on failure to attach.
		 // */
		// void * attach() {
			// void * tempPtr = shmat(shmID, NULL, 0);
			
			// if(tempPtr == (void *) -1) {
				// perror("SharedMemory.attach() error");
			// } else {
				// this->spacePtr = tempPtr;
			// }//end if
		// }//end attach
		
		// /**
		 // *@brief Detaches the shared memory space from the calling process.
		 // *@return Success status of the call.
		 // */
		// bool detach() {
			// int result = shmdt(this->spacePtr);
			
			// if(result == -1) {
				// perror("SharedMemory.detach() error");
			// }//end if
			
		// }//end detach
		
		// /**
		 // *@brief Destroys the shared memory space. (NOTE: This is NOT a destructor for the SharedMemory object, however, calling this will render the object useless.)
		 // *@return Success status of the call.
		 // */
		// bool destroy() {
			// int result;
			
			// result = shmctl(semID, IPC_RMID, NULL);
			
			// if(result == -1) {
				// perror("SharedMemory.detach() error");
			// }//end if
			
			// return (result != -1);
		// }//end detach
		
		// /**
		 // *@brief Increments the number of clients field in the shared memory space
		 // */
	  // void incrementNumClients() {
			// int * numClients = *(int *) beginPtr;
			// numClients++;
		// }//end writeNumClients
		
		// /**
		 // *@brief Decrements the number of clients field in the shared memory space
		 // */
	  // void decrementNumClients() {
			// int * numClients = *(int *) beginPtr;
			// numClients--;
		// }//end writeNumClients
		
		// /**
		 // *@brief Initializes the PID and startTime fields of the shared memory block and stores the pointer to this block.
		 // *@param cliPID The calling process' PID.
		 // */
		// void initBlock(pid_t cliPID) {
			// struct sharedMemBlock * newBlockPtr = findAvailBlock();
			
			// //Store PID and startTime
			// newBlockPtr->pid = cliPID;
			// newBlockPtr->startTime = system_clock::now();
			
			// this->blockPtr = newBlockPtr;
		// }//end initBlock
		
		// /**
		 // *@brief Clears the calling process' shared memory block.
		 // */
	  // void clearBlock() {
		  // //Zero out the shared memory block
		  // memset(this->blockPtr,0,sizeof(sharedMemBlock);
	  // }//end clearBlock
	
		// /**
		 // *@brief Increments/Updates the numCommand and lastMsgTime fields of the shared memory block.
		 // */
		// void incrementCommand() {
			// this->blockPtr->numCommands++;
			// this->blockPtr->lastMsgTime = system_clock::now();
		// }//end updateBlock

		// /**
		 // *@brief Finds the first available shared memory block in the shared memory space.
		 // *@return The sharedMemBlock struct pointer to the first available shared memory block.
		 // */
		// struct sharedMemBlock * findAvailBlock() {
			// struct sharedMemBlock * newBlockPtr;
			
			// //Loop through each block testing if it is populated
			// for(void * begin = spacePtr + sizeof(int); begin < spaceSize - sizeof(sharedMemBlock); begin+=sizeof(sharedMemBlock) {
				// newBlockPtr = (struct sharedMemBlock *)begin;	
				
				// //Check if block is empty
				// if(newBlockPtr->pid = 0) {
					// break;
				// }//end if
				
			// }//end for
			
			// return newBlockPtr;
		// }//end findAvailBlock

// }//end SharedMemory