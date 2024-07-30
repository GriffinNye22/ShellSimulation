/**
 * @file server.cpp
 * @author Griffin Nye
 * @brief Server side of the TCP Distributed Information System implementation
 * CSC552 Dr. Spiegel Spring 2020
 */


#include <arpa/inet.h>
#include <cstdio>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "msgPackets.cpp"
#include "LogBinRWSemMonitor.cpp"


using namespace std;

/*! My assigned port on acad for the server's listening socket. */
#define PORTNUM 15005

//PROTOTYPES//

/**
 *@brief Adds a record to the bin file
 *@param binPtr File pointer to the bin file
 *@param record The record to be added.
 *@param recordSize Size of the record to be added.
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset and server log.
 *@return The success of the addition  
 */
bool addRecord(FILE * binPtr, char record[], int recordSize, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Listens for incoming commands from the connected client.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file.
 *@param logPtr The file pointer to the server log file.
 */
void awaitCommands(int commfd, FILE *binPtr, FILE *logPtr);

/**
 *@brief Listens for incoming client connections and creates child servers for each succesful connection.
 *@param listenfd The listening socket's file descriptor.
 *@param binPtr The file pointer to the binary data file.
 *@param logPtr The file pointer to the server log file.
 */
void awaitConnections(int listenfd, FILE *binPtr, FILE *logPtr);

/**
 *@brief Handles client request for the edit of a record from the dataset.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file.
 *@param logPtr The file pointer to the server log file.
 *@param cliPID The requesting client's PID.
 *@param recIdx The index of the requested record to edit.
 *@param record The edited record string.
 *@param recordSize The size of the edited record string.
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset and server log.
 */
void changeRecord(int commfd, FILE *binPtr, FILE *logPtr, pid_t cliPID, int recIdx, char record[], int recordSize, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Handles client request for the retrieval of one or more records from the dataset.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file.
 *@param logPtr The file pointer to the server log file.
 *@param cliPID The requesting client's PID.
 *@param recIdx The index of the requested record (-999 for all records).
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset and server log.
 */
void displayRecord(int commfd, FILE *binPtr, FILE *logPtr, pid_t cliPID, int recIdx, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Retrieves a record with the provided index from the file specified.
 *@param binPtr File pointer to the bin file
 *@param idx Index of the desired record
 *@return The desired record in C-string format
 */
string getRecord(FILE *binPtr, int idx);

/**
 *@brief Calculates and returns the number of log records stored in the server log file.
 *@param logPtr The file pointer to the server log file.
 *@return The total number of log records stored in log server file
 */
int getTotalLogRecords(FILE *logPtr);

/**
 *@brief Calculates and returns the number of records stored in the binary data file.
 *@param binPtr The file pointer to the binary data file.
 *@return The total number of records stored in binary data file.
 */
int getTotalRecords(FILE *binPtr);

/**
 *@brief Decides the appropriate course of action for a received command then logs the operation(s) performed.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file.
 *@param logPtr The file pointer to the server log file.
 *@param clientMsg The message packet received from the client.
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset and server log.
 */
void handleCmd(int commfd, FILE *binPtr, FILE *logPtr, serMsgPacket clientMsg, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Logs the successful connection of an incoming client
 *@param logPtr The file pointer to the server log file.
 *@param clientAddress The string representation of the client's address.
 */
void logConnection(FILE *logPtr, string clientAddress);

/**
 *@brief Logs the client request and server response for any given operation.
 *@param logPtr The file pointer to the server log file.
 *@param cliPID The connecting client's PID.
 *@param cmd Character representing the command issued to the server
 *@param numRecords (optional) The number of records stored on server(CMD)/sent to client(GET)
 *@param idx (optional) The index of the desired record from GET command
 */
 void logRequest(FILE *logPtr, pid_t cliPID, char cmd, int numRecords = -1, int idx = -1);

/**
 *@brief Handles client request for the addition of a new record to the dataset.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file.
 *@param logPtr The file pointer to the server log file.
 *@param cliPID The requesting client's PID.
 *@param record The record to be added.
 *@param recordSize The size of the record to be added.
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset and server log.
 */
void newRecord(int commfd, FILE *binPtr, FILE *logPtr, pid_t cliPID, char record[], int recordSize, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Attempts to open the file provided. Returns file pointer on successful open.
 *@param filename The path to the file
 *@param filetype The type of file being transmitted ("data" or "log"). Used to generate appropriate error message.
 *@return filePointer on successful open; server exits on failed open.
 */ 
FILE * openFile(string filename, string filetype);

/**
 *@brief Handles the receipt of messages from clients.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file.
 *@param logPtr The file pointer to the server log file.
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset and server log.
 */
void receiveMsgs(int commfd, FILE *binPtr, FILE *logPtr, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Handles client request for retrieving the record count.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file.
 *@param logPtr The file poitner to the server log file.
 *@param cliPID The requesting client's PID.
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset and server log.
 */
void recordCount(int commfd, FILE *binPtr, FILE *logPtr, pid_t cliPID, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Retrieves each record in the file one by one and sends them to the requesting client.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset.
 *@return The number of records sent to the client
 */
int sendAllRecords(int commfd, FILE *binPtr, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Handles client request for retrieving the contents of the server log.
 *@param commfd The communications socket's file descriptor.
 *@param logPtr The file pointer to the server log file.
 *@param cliPID The requesting client's PID.
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset.
 */
void sendLog(int commfd, FILE *logPtr, pid_t cliPID, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Retrieves and sends a single record from the server log file.
 *@param commfd The communications socket's file descriptor.
 *@param logPtr The file pointer to the server log file.
 *@param idx The index of the log record to be sent.
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset. 
 */
void sendLogRecord(int commfd, FILE *logPtr, int idx, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Retrieves the record found at the provided index from the file and sends it to the requesting client.
 *@param commfd The communications socket's file descriptor.
 *@param binPtr The file pointer to the binary data file
 *@param idx The index of the desired record
 *@param fileMonitor Monitor for synchronizing read & write operations on the dataset.
 */
void sendRecord(int commfd, FILE *binPtr, int idx, LogBinRWSemMonitor &fileMonitor);

/**
 *@brief Handles the transmission of messages to clients
 *@param commfd The communications socket's file descriptor.
 *@param msg The message packet to be transmitted
 */
template<class MsgPacket> void sendMsg(int commfd, MsgPacket msg);

/**
 *@brief Sets up the server connection by creating the listening socket, binding it to the server address, and creating the listening queue.
 *@param port The server's dedicated port number.
 *@return The listening socket's file descriptor on successful creation; client exits on failure.
 */
int setupConnection(int port);

/**
 *@brief Updates the record at the provided index.
 *@param binPtr The file pointer to the bin file.
 *@param idx The index of the record to be updated.
 *@param record The updated record string.
 *@param recordSize The size of the updated record string.
 *@return The success of updating the record.
 */
bool updateRecord(FILE * binPtr, int idx, char record[], int recordSize);

//DEFINITIONS//

/**
 *@brief Calls the appropriate methods to ensure proper functionality of the server.
 *@param argc The number of command-line arguments
 *@param argv List of command-line arguments
 */
int main(int argc, char * argv[]) {
	const string BINFILE = "gameRevenue.bin";
	const string LOGFILE = "ser.log";

	FILE * binPtr,* logPtr;	
	int listenfd, commfd;
	
	//Perform Server startup operations
	listenfd = setupConnection(PORTNUM);
	binPtr = openFile(BINFILE, "data");
	logPtr = openFile(LOGFILE, "log");
  LogBinRWSemMonitor fileMonitor(PORTNUM);
	fileMonitor.init();
	
	//Wait for incoming connections
	cout << "Listening for incoming connections..." << endl;
	awaitConnections(listenfd, binPtr, logPtr);

}//end main

//Adds a record to the bin file
bool addRecord(FILE * binPtr, char record[], int recordSize, LogBinRWSemMonitor &fileMonitor) {
  bool success;
	
	//Get record count
	fileMonitor.addBinReader();
	int count = getTotalRecords(binPtr);
	fileMonitor.remBinReader();
  
	//Add the new record
	fileMonitor.addBinWriter();
	success = updateRecord(binPtr, count+1, record, recordSize);
	fileMonitor.remBinWriter();
	
  return success;
}//end addRecord

//Listens for incoming client connections and creates child servers for each successful connection.
void awaitConnections(int listenfd, FILE *binPtr, FILE *logPtr) {
	char clientAddr[INET_ADDRSTRLEN];
	int numClients = 0;
	int commfd, pid;
	socklen_t cliSize;
	string strClientAddress;
	struct sockaddr_in client;
	
	//Continuously accept incoming client connections
	while(true) {
		
		cliSize = sizeof(client);
		
		//Accept incoming client connection
		if((commfd = accept(listenfd, (struct sockaddr *) &client, &cliSize)) == -1) {
		  perror("Error accepting incoming connection: ");
		  cout << "Shutting down server..." << endl;
		  exit(EXIT_FAILURE);
		}//end if			
		
		//Increment Client count
		numClients++;
		
		//Delegate connection to child server
		if((pid = fork() ) == -1) {
			perror("Error creating child server process: ");
			cout << "Shutting down server..." << endl;
			exit(EXIT_FAILURE);
		} else if(pid == 0) { //Child Server
			
			//Construct client address string
			inet_ntop(AF_INET, &client.sin_addr, clientAddr, INET_ADDRSTRLEN); 
			strClientAddress = clientAddr;
			strClientAddress += ":" + to_string( htons(client.sin_port) );
			cout << strClientAddress << "connected\n"; 
			
			//Construct Monitor for child server (Essentially just gains access to previous semSet)
			LogBinRWSemMonitor fileMonitor(PORTNUM);
			
			//Log client arrival
			fileMonitor.addLogWriter();
			logConnection(logPtr, strClientAddress);
			fileMonitor.remLogWriter();
			
			//Await client connection
			receiveMsgs(commfd, binPtr, logPtr, fileMonitor);
		} else { //Parent Server
			
		}//end if

	}//end while
	
}//end awaitConnections

//Handles client request for the edit of a record from the dataset.
void changeRecord(int commfd, FILE *binPtr, FILE *logPtr, pid_t cliPID, int recIdx, char record[], int recordSize, LogBinRWSemMonitor &fileMonitor) {
	intRecMsgPacket ackMsg;
	bool success;
	string strSuccess;
	
	//Update the record
	fileMonitor.addBinWriter();
	success = updateRecord(binPtr, recIdx, record, recordSize);
	fileMonitor.remBinWriter();
	
	//Insert Success or Failure message
	if(success) {
		strSuccess = "SUCCESS";
	} else {
		strSuccess = "FAILURE";
	}//end if
	
	//Assemble acknowledgment message
	ackMsg = intRecMsgPacket(getpid(), "FIX", recIdx, &strSuccess[0] );
	
	//Send acknowledgment to client
	sendMsg(commfd, ackMsg);
	
	//Log the client request & server response
	fileMonitor.addLogWriter();
	logRequest(logPtr, cliPID, 'F', -1, recIdx);
	fileMonitor.remLogWriter();
}//end changeRecord

//Handles client request for the retrieval of one or more records from the dataset.
void displayRecord(int commfd, FILE *binPtr, FILE *logPtr, pid_t cliPID, int recIdx, LogBinRWSemMonitor &fileMonitor) {
	int numRecords;

	//Determine whether to send all records or a single record.
	if(recIdx == -999) {
		//Send all records to client & log the operation
		numRecords = sendAllRecords(commfd, binPtr, fileMonitor);
		fileMonitor.addLogWriter();
		logRequest(logPtr, cliPID, 'G', numRecords, recIdx);
		fileMonitor.remLogWriter();
	} else {
		//Send record to client & log the operation
		sendRecord(commfd, binPtr, recIdx, fileMonitor);
		fileMonitor.addLogWriter();
		logRequest(logPtr, cliPID, 'G', -1, recIdx);
		fileMonitor.remLogWriter();
	}//end if
		
}//end displayRecord

//Retrieves a record with the provided index from the file specified.
string getRecord(FILE *binPtr, int idx) {
	char record[MAXRECORDSIZE+1]; 
 
 	rewind(binPtr);
 
  //Seek to desired record
  fseek(binPtr, (MAXRECORDSIZE + 1) * (idx - 1), SEEK_SET);
  
  //Read record
  int i = fread(&record, sizeof(char), sizeof(record)/sizeof(char), binPtr);
  
  string recordBuf(record);
  
  return recordBuf;
}//end getRecord

//Calculates and returns the number of log records stored in the server log file.
int getTotalLogRecords(FILE *logPtr) {
  char logRecord[MAXLOGRECORDSIZE];
  int ctr = 0;
  
	//Set file pointer to beginning of file
	rewind(logPtr);
	
  //Continue reading log records until end of file is reached.
  while( fgets(logRecord, sizeof(logRecord)/sizeof(char), logPtr) != NULL ) {
    ctr++;
  }//end while
  
  return ctr;
}//end getTotalLogRecords

//Calculates and returns the number of records stored in the binary data file.
int getTotalRecords(FILE *binPtr) {
  int ctr = 0;
  char record[MAXRECORDSIZE+2];
  
  //Set file pointer to beginning of file
  rewind(binPtr);
  
  //Continue reading records until end of file is reached
  while( fgets(record, sizeof(record)/sizeof(char), binPtr) != NULL ) {
    ctr++;
  }//end while

  return ctr;
}//end getTotalRecords

//Decides the appropriate course of action for a received command then logs the operation(s) performed
void handleCmd(int commfd, FILE *binPtr, FILE *logPtr, serMsgPacket clientMsg, LogBinRWSemMonitor &fileMonitor) {
  
	//Determine issued command
  if( strcmp(clientMsg.cmd, "CNT") == 0) {
    recordCount(commfd, binPtr, logPtr, clientMsg.sender, fileMonitor);
  } else if( strcmp(clientMsg.cmd, "GET") == 0) {
    displayRecord(commfd, binPtr, logPtr, clientMsg.sender, clientMsg.val, fileMonitor);
  } else if( strcmp(clientMsg.cmd, "FIX") == 0) {
    changeRecord(commfd, binPtr, logPtr, clientMsg.sender, clientMsg.val, clientMsg.record, sizeof(clientMsg.record), fileMonitor);
  } else if( strcmp(clientMsg.cmd, "NEW") == 0) {
    newRecord(commfd, binPtr, logPtr, clientMsg.sender, clientMsg.record, sizeof(clientMsg.record), fileMonitor);
  } else if( strcmp(clientMsg.cmd, "LOG") == 0) {
    sendLog(commfd, logPtr, clientMsg.sender, fileMonitor);
  }//end if
	
}//end handleCmd

//Logs the successful connection of an incoming client
void logConnection(FILE * logPtr, string clientAddress) {
	fprintf(logPtr, "%s successfully connected.\n", &clientAddress[0]);
}//end logConnection

//Logs the client request and server response for any given operation.
void logRequest(FILE * logPtr, pid_t cliPID, char cmd, int numRecords, int idx) {
	
  //Add Log entry based on command
  switch(cmd) {
  
    //CMD command
    case 'C':
      fprintf(logPtr, "Server responded to Client %li with %i total records.\n", (long) cliPID, numRecords);
			break;
			
    //FIX command
    case 'F':
      fprintf(logPtr, "Server successfully updated record #%i for Client %li.\n", idx, (long) cliPID);
      break;
      
    //GET command
    case 'G':
      
      //Requested all records
      if(idx == -999) {
        fprintf(logPtr, "Server responded to Client %li with list of %i records.\n", (long) cliPID, numRecords);
      } else { //Requested single record
        fprintf(logPtr, "Server responded to Client %li with record #%i.\n", (long) cliPID, idx);
      }//end if
      
      break;
    
    //NEW command
    case 'N':
      fprintf(logPtr, "Server successfully added record provided by Client %li.\n", (long) cliPID);
      break;
    //LOG command
    case 'L':
      fprintf(logPtr, "Server responded to Client %li with list of %i log records.\n", (long) cliPID, numRecords);
      break;
  }//end switch
	
	fflush(logPtr);
}//end logRequest

//Handles client request for the addition of a new record to the dataset and its subsequent logging.
void newRecord(int commfd, FILE *binPtr, FILE *logPtr, pid_t cliPID, char record[], int recordSize, LogBinRWSemMonitor &fileMonitor) {
	bool success;
	intRecMsgPacket ackMsg;
	string strSuccess;
	
	//Add record
	success = addRecord(binPtr, record, recordSize, fileMonitor);
	
	//Insert Success or Failure message
	if(success) {
		strSuccess = "SUCCESS";
	} else {
		strSuccess = "FAILURE";
	}//end if
	
	//Assemble acknowledgment message
	ackMsg = intRecMsgPacket(getpid(), "NEW", -1, &strSuccess[0]);
	
	//Send acknowledgment to client
	sendMsg(commfd, ackMsg);
	
	//Log server operation
	fileMonitor.addLogWriter();
	logRequest(logPtr, cliPID, 'N');
	fileMonitor.remLogWriter();
}//end newRecord

//Attempts to open the file provided. Returns file pointer on successful open.
FILE * openFile(string filename, string filetype) {
	string openMode;
	
	if( filetype == "data" ) {
		openMode = "rb+";
	} else {
		openMode = "ab+";
	}//end if
	
	FILE * filePtr = fopen(filename.c_str(), openMode.c_str() );
	
	//Exit if error opening file
	if(filePtr == NULL) {
		cout << "Error opening " + filetype + " file " + filename + "." << endl;
		cout << "Shutting down server..." << endl;
		exit(EXIT_FAILURE);		
	}//end if
	
	return filePtr;
}//end openBinFile

//Handles the receipt of messages from the client.
void receiveMsgs(int commfd, FILE *binPtr, FILE *logPtr, LogBinRWSemMonitor &fileMonitor) {
  serMsgPacket msg; 
  
  while(true) {
    if( read(commfd, &msg, sizeof(msg) ) == -1) {
      perror("Error receiving messages from client: ");
    } else {
      handleCmd(commfd, binPtr, logPtr, msg, fileMonitor);
    }//end if  
  }//end while
  
}//end receiveMsgs

//Handles client request for retrieving the record count.
void recordCount(int commfd, FILE *binPtr, FILE *logPtr, pid_t cliPID, LogBinRWSemMonitor &fileMonitor) {
	int numRecords;
	intMsgPacket finalMsg;
    
	//Get total number of records
	fileMonitor.addBinReader();
	numRecords = getTotalRecords(binPtr);
	fileMonitor.remBinReader();
	
	//Assemble record count message packet
	finalMsg = intMsgPacket(getpid(), "CNT", numRecords);
	
	//Send number of records to client
	sendMsg(commfd, finalMsg);
	
	//Log the server response.
	fileMonitor.addLogWriter();
	logRequest(logPtr, cliPID, 'C', numRecords);
	fileMonitor.remLogWriter();
}//end recordCount

//Retrieves each record in the file one by one and sends them to the requesting client.
int sendAllRecords(int commfd, FILE *binPtr, LogBinRWSemMonitor &fileMonitor) {
 
 	//Prepare reader for reading, retrieve the record count, and cleanup
  fileMonitor.addBinReader();
	int numRecords = getTotalRecords(binPtr);
	fileMonitor.remBinReader();
  
  //Call sendRecord for each record in file
  for(int i = 1; i <= numRecords; i++) {
    sendRecord(commfd, binPtr, i, fileMonitor);
  }//end for
  
  return numRecords;
}//end sendAllRecords

//Handles client request for retrieving the contents of the server log.
void sendLog(int commfd, FILE *logPtr, pid_t cliPID, LogBinRWSemMonitor &fileMonitor) {
	int numRecords;
	intMsgPacket cntMsg;
	
	//Get total number of records
	fileMonitor.addLogReader();
	numRecords = getTotalLogRecords(logPtr);
	fileMonitor.remLogReader();
	
	//Construct log record count message packet
	cntMsg = intMsgPacket(getpid(), "LOG", numRecords);
	
	//Send number of log records back to client
	sendMsg(commfd, cntMsg);

	rewind(logPtr);

	//Send contents of the log back to client
  for(int i = 0; i < numRecords; i++) {
    sendLogRecord(commfd, logPtr, i, fileMonitor);
  }//end for
	
	//Log the client request & server response
	fileMonitor.addLogWriter();
	logRequest(logPtr, cliPID, 'L', numRecords);
	fileMonitor.remLogWriter();
}//end sendLog

//Retrieves and sends a single record from the server log file.
void sendLogRecord(int commfd, FILE *logPtr, int idx, LogBinRWSemMonitor &fileMonitor) {
  char logRecord[MAXLOGRECORDSIZE];
  logMsgPacket logMsg;
	
	//Seek to desired log record
	//fseek(logPtr, (MAXLOGRECORDSIZE + 1) * (idx - 1), SEEK_SET);
	
  //Get line from log
	fileMonitor.addLogReader();
  fgets(logRecord, sizeof(logRecord)/sizeof(char), logPtr);
	fileMonitor.remLogReader();
	
	//Assemble retrieved log record message packet
	logMsg = logMsgPacket(getpid(), "LOG", logRecord);
  
  //Send log record to client
  sendMsg(commfd, logMsg);
}//end sendLogRecord

//Retrieves the record found at the provided index from the file and sends it to the requesting client.
void sendRecord(int commfd, FILE *binPtr, int idx, LogBinRWSemMonitor &fileMonitor) {
  string record;
  char * recordCString = (char*) malloc(sizeof(record) + 1);
  recMsgPacket recMsg;
	
	//Prepare Reader for reading, retrieve the record, and cleanup
	fileMonitor.addBinReader();
  record = getRecord(binPtr, idx);
	fileMonitor.remBinReader();
  
	//Assemble retrieved record message packet
  recMsg = recMsgPacket(getpid(), "GET", &record[0]);
  
  //Send retrieved record to client
  sendMsg(commfd, recMsg);
}//end sendRecord

//Handles the transmission of messages to clients
template<class MsgPacket> void sendMsg(int commfd, MsgPacket msg) {
  if( write(commfd, &msg, sizeof(msg) ) == -1) {
    perror("Error sending message to client: ");
  } else {
    //cout << "Message sent to client" << endl;
  }//end if
}//end sendMsg

//Sets up the server connection by creating the listening socket, binding it to the server address, and creating the listening queue.
int setupConnection(int port) {
	const int MAX_CONN = 10;
	int listenfd;
	struct sockaddr_in serverAddress;
	
	//Create listening socket
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error creating listening socket: ");
		cout << "Shutting down server..." << endl;
		exit(EXIT_FAILURE);
	}//end if
	
	//Intialize server address and port
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	serverAddress.sin_port = htons(port);
	
	//Bind server address to the listening socket
	if( bind(listenfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress) ) == -1) {
		perror("Error binding server address to listening socket: ");
		cout << "Shutting down server..." << endl;
		close(listenfd);
		exit(EXIT_FAILURE);
	}//end if
	
	//Specify listenfd as listening socket with queue of size MAX_CONN
	if( listen(listenfd, MAX_CONN) == -1) {
		perror("Error specifying listening socket: ");
		cout << "Shutting down server..." << endl;
		exit(EXIT_FAILURE);		
	}//end if
	
	return listenfd;
}//end setupConnection

//Updates the record at the provided index
bool updateRecord(FILE * binPtr, int idx, char record[], int recordSize) {
  int charsWritten;
  
  //Set File pointer back to beginning of file
  rewind(binPtr);
  
  //Seek to desired record
  fseek(binPtr, (MAXRECORDSIZE+1) * (idx - 1), SEEK_SET);
  
  //Write updated record to file
  charsWritten = fwrite(record, sizeof(char), recordSize/sizeof(char), binPtr);
  
  return charsWritten != 0;
}//end updateRecord