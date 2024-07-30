/**
 *@file SemaphoreSet.cpp  
 *@author Griffin Nye
 *@brief Wrapper Class Implementation of UNIX System V semaphores.
 */


#ifndef SEMAPHORESET
#define SEMAPHORESET

#include <cstddef>
#include <iostream>
#include <sys/ipc.h>
#include <sys/sem.h>

using namespace std;

/**
 *@brief Wrapper Class for UNIX System V Semaphores.
 */
class SemaphoreSet {
private:
  int semID;
  int key;
  int numSems;
  int permFlags;
    
  union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    #if defined(_linux_)
    struct seminfo  *__buf;  /* Buffer for IPC_INFO      */
  #endif
  };
    
public:

  /**
   *@brief Default constructor for the semaphore set.
   */
  SemaphoreSet() {};

  /**
   *@brief Constructs a semaphore set.
   *@param key The key for the semaphore set.
   *@param numSems The number of semaphore to create.
   *@param permFlags The permissions flags for the semaphore set.
   *@return The semaphore set identifier (0 indicates semaphore set already created, 
            negative value means semaphore set does not exist).
   */
  SemaphoreSet(int key, int numSems, int permFlags) {
    this -> key = key;
    this -> numSems = numSems;
    this -> permFlags = permFlags;
    this -> semID = semget(key, numSems, permFlags);
  }//end constructor
  
  /**
   *@brief Destroys the semaphore set.
   */
  void close() {
    semctl(semID, 0, IPC_RMID, 0);
  }//end destructor
  
  /**
   *@brief Adds a semaphore to the semaphore set.
   *@return The new semaphore set identifier.
   */
  int addSemaphore() {
    //Delete semaphore set and create new semaphore set with additional semaphore
    semctl(semID, 0, IPC_RMID, 0);
    numSems++;
    this -> semID = semget(this -> key, this -> numSems, this -> permFlags);
    return (this -> semID);
  }//end addSemaphore
  
  /**
   *@brief Removes a semaphore to the semaphore set.
   *@return The new semaphore set identifier.
   */
  int removeSemaphore() {
    //Delete semaphore set and create new semaphore set with one less semaphore
    semctl(semID, 0, IPC_RMID, 0);
    numSems--;
    this -> semID = semget(this -> key, this -> numSems, this -> permFlags);
    return (this -> semID);
  }//end removeSemaphore
  
  /**
   *@brief Issues a wait operation on the semaphore.
   *@param semNum The semaphore to issue the wait operation to.
   *@return 0 on success, -1 on failure.
   */
  int wait(unsigned short semNum) {
    struct sembuf semBuf;
    
    //Specify semaphore number to wait
    semBuf.sem_num = semNum;
    semBuf.sem_op = -1;
    semBuf.sem_flg = 0;
    
    return semop(semID, &semBuf, 1);
  }//end wait
  
  /**
   *@brief Signals a semaphore to wakeup a process.
   *@param semNum The semaphore to issue the signal to.
   *@return 0 on success, -1 on failure.
   */
  int signal(unsigned short semNum) {
    struct sembuf semBuf;
    
    //Specify semaphore number to signal
    semBuf.sem_num = semNum;
    semBuf.sem_op = 1;
    semBuf.sem_flg = 0;
    
    return semop(semID, &semBuf, 1);
  }//end signal
  
  /**
   *@brief Retrives the value of all semaphores in the set.
   *@return The array of semaphore values on success, NULL on failure.
   */
  unsigned short * getAll() {
    int result;
    semun semUn;
    
    //Allocate array for storing semaphore values
    semUn.array = (ushort *)malloc(numSems * sizeof(ushort) );
    
    //Store result of semctl
    result = semctl(semID, 0, GETALL,semUn);
    
    //Return array of semaphore values
    if(result == 0) {
      return semUn.array;
    } else {
      cout << to_string(result) << endl;
      perror("SemaphoreSet.getAll() Error");
      return NULL;
    }//end if
    
  }//end getAll
  
  /**
   *@brief Sets the value of all semaphores in the set to the specified value.
   *@param val The value to set all semaphores in the set to.
   *@return 0 on success, -1 on failure.
   */
  int setAll(int val) {
    semun semUn;
    
    //Allocate array for setting semaphore values
    semUn.array = new unsigned short[numSems];
    
    //Populate array with new semaphore values
    for(int i = 0; i < numSems; i++) {
      semUn.array[i] = val;
    }//end for
  
    return semctl(semID, 0, SETALL, semUn);
  }//end setAll
  
  /**
   *@brief Gets the value of a single semaphore in the set.
   *@param semNum The number of the semaphore for which to retrieve its value.
   *@return The value of the specified semaphore.
   */
  unsigned short get(unsigned short semNum) {
    return semctl(semID, semNum, GETVAL);
  }//end get
  
  /**
   *@brief Sets the value of a single semaphore in the set.
   *@param semNum The number of the semaphore for which to set its value.
   *@param val The value to set the semaphore to.
   *@return 0 on success, -1 on failure.
   */
  int set(unsigned short semNum, int val) {
    semun semUn;
    
    //Populate structure with new value for semaphore
    semUn.val = val;
    
    return semctl(semID, semNum, SETVAL, semUn);
  }//end set
  
  /**
   *@brief Returns the number of processes waiting for the semaphore to increase its value.
   *@param semNum The semaphore for which to retrieve the number of waiting processes.
   *@return The number of processes waiting for the semaphore to increase its value.
   */
  int getWaitCount(unsigned short semNum) {
    return semctl(semID, semNum, GETNCNT);
  }//end getWaitCount
  
  /**
   *@brief Gets the number of semaphores in the set.
   *@return The number of semaphores in the set.
   */
  int getNumSems() {
    return numSems;
  }//end getNumSems
  
  /**
   *@brief Prints the values of the semaphore set.
   *@author Dr. Daniel Spiegel (source: shmdemo.cpp) Modified by: Griffin Nye
   */
  void print() {
    unsigned short * values = (ushort *)malloc(sizeof(ushort) * numSems);
  
    values = getAll();
    
    fprintf(stderr,"Semaphore Values ");
    for(int i=0; i<numSems; i++)
      fprintf(stderr,"#%d. %d  | ", i, values[i]);
    fprintf(stderr,"\n");
  }//end print

};//end Semaphore

#endif
