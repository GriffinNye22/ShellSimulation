/**
 * @file msgPackets.cpp
 * @author Griffin Nye
 * @brief Header file defining the various packets used to transmit data across the TCP socket connection.
 * CSC552 Dr. Spiegel Spring 2020 
 *@mainpage Distributed Point-To-Point Communication System
 *@section proj_desc Project Description 
 * This program allows clients launched from any of 3 machines (acad, mcgonagall, & csc552)
 * to manipulate the records of a binary-encoded dataset stored on the server through 
 * the transmission of message packets over a TCP socket connection.
 *@section server_desc Server-side Description
 * The server accepts incoming client connections, incrementing the number of clients
 * and creating a new process for handling communications for each client. The server 
 * contains access to two separate files: The binary data file containing data records and a 
 * log file that documents all server operations. The server listens for incoming message 
 * packets on the socket, performing the corresponding operation for the received command. 
 * The server utilizes semaphores to ensure mutual exclusion amongst processes when reading 
 * and writing to/from both the dataset and the log file. The server operation is subsequently 
 * logged in the log file. 
 *@section client_desc Client-side Description
 * On startup, the client will allocate shared memory space and semaphores for all
 * clients, if it is the first client on that machine to run. All subsequent clients
 * on said machine will utilize the existing structures. It will then increment the number
 * of clients in the shared memory space and subsequently find the next available block
 * in the shared memory space, initializing its PID and startTime fields. The client will
 * utilize semaphores to ensure mutual exclusion amongst processes when reading/writing 
 * to the shared memory space and/or the log file. The client is presented with a menu of commands 
 * that can be issued to the server and subsequently prompted for their selection. The client 
 * will then be prompted for any other necessary information relevant to the command and send 
 * it over the socket to the server. All Client operations are then logged in the client log file.
 * The client may perform as many operations as they wish until they specify to exit.
 *@section readWrite Readers-Writers Problem
 * The server will utilize a strong writers' preference that allows concurrent reader access
 * for accessing the binary data file. The client will utilize a strong writers' preference
 * that allows concurrent reader access for accessing shared memory. Both the client
 * and the server will utilize a weak reader's preference with concurrent reader access 
 * for accessing their log files.
 *@section menu Menu Options:
 *@subsection new_record New Record
 * Upon selecting the New Record menu option, the client will prompt the user for
 * all necessary fields for a record and validate the user's input. The client will
 * then issue the NEW command to the server, indicating a new record is to be added,
 * passing it the newly defined record, and awaiting the server's acknowledgment message. 
 *@subsection display_record Display Record
 * Upon selecting the Display Record menu option, the client will issue the CNT command
 * to the server, indicating a request for the number of records, and await the
 * server's response. Upon receiving the number of records, the client will prompt
 * the user for the index of a record between 1 and the received number of records
 * to be displayed, validating the entered value is in that range. The user also 
 * has the option to enter -999 to display all records. The client will then issue
 * the GET command, indicating a request for a particular record or set of records,
 * and await the server's response. The client will then display the received record(s)
 * to the user.
 *@subsection change_record Change Record
 * Upon selecting the Change Record menu option, the client will perform the same
 * operations as Display Record, however, without the option to display all records.
 * After the selected record is displayed, the client will prompt the user to select
 * a field to edit from a menu of editable fields and then prompt the user for a value
 * for that field, reprompting for invalid input if necessary. The client will then
 * issue the FIX command to the server, indicating a record is to be updated, passing
 * it the newly updated record and its index, and await the server's acknowledgment
 * message.
 *@subsection show_log Show Server Log 
 * Upon selecting the Show Log menu option, the client will issue the LOG command 
 * to the server, indicating a request for the contents of the log and await the 
 * server's response. The client will receive the number of log records to expect,
 * followed by a stream of individual log records. The client will print each log
 * record to the user as it is received. 
 *@subsection view_log View Client Log
 * Upon selecting the View Client Log menu option, the client will access and print
 * the contents of its machine's log file one record a time until it reaches the end
 * of the file.
 *@subsection list_clients List Local Clients
 * Upon selecting the List Local Clients menu option, the client will access the shared
 * memory and retrieve each of the local client's process IDs one at a time,
 * displaying them to the user
 */

#include<map>
#include <string.h>
#include <sys/types.h>

using namespace std;

/*! The current year, for validating new record entries */
#define CURRENTYEAR 21
/*! Maximum size of a log record on the server. */
#define MAXLOGRECORDSIZE 64
/*! Maximum size of a record on the server. */
#define MAXRECORDSIZE 27

//MONTH ENUMERATION	
/*! An enumerated type for all of the months in a year */
enum MONTH {JAN, FEB, MAR, APR, MAY, JUN, JUL, AUG, SEP, OCT, NOV, DEC};

/**
 *@brief Used to map enum values to valid input strings for the month field of new records.
 */
class stringToMonthConverter {
	
	private:
		map<string, MONTH> monthMap;
	
	public:
	
		/**
		 *@brief Constructor for the stringToMonthConverter. Populates the monthMap.
		 */
		stringToMonthConverter() {
			monthMap["Jan"] = JAN;
			monthMap["Feb"] = FEB;
			monthMap["Mar"] = MAR;
			monthMap["Apr"] = APR;
			monthMap["May"] = MAY;
			monthMap["Jun"] = JUN;
			monthMap["Jul"] = JUL;
			monthMap["Aug"] = AUG;
			monthMap["Sep"] = SEP;
			monthMap["Oct"] = OCT;
			monthMap["Nov"] = NOV;
			monthMap["Dec"] = DEC;
		}//end constructor
		
		/**
		 *@brief Returns whether the provided string is a valid 3-letter month abbreviation
		 *@param month The string to be checked
		 *@return Whether the string is a valid 3-letter month abbreviation		 
		 */
		bool isMonth(string month) {
			map<string, MONTH>::iterator itr = monthMap.find(month);
			
			//Return corresponding MONTH
			if(itr != monthMap.end() ) {
			  return true;	
			} else { //Return NULL if no match found	
			  return false;
			}//end if
			
		}//end isMonth
		
};//end stringToMonthConverter

//PACKETS

/**
 *@struct msgPacket
 *@brief Base Struct TCP message packet
 *@var msgPacket::sender 
 * The pid of the sender of the message
 *@var msgPacket::cmd 
 * The command for the message
 */
struct msgPacket {
	public:
		pid_t sender; 
		char cmd[4];
		
		/**
		 *@brief Default constructor for msgPacket
		 */
		msgPacket() {
		}//end constructor
		
		/**
		 *@brief Constructs a msgPacket given its elements.
		 *@param sender The sender of the message's PID.
		 *@param cmd The command for the message.
		 */
		msgPacket(pid_t sender, const char cmd[]) {
			this->sender = sender;
			strcpy(this->cmd, cmd);
		}//end constructor
		
};//end msgPacket

/**
 *@struct intMsgPacket
 *@brief Sub-Struct TCP message packet for transmitting record count & indexes
 *@var intMsgPacket::val
 * Integer value for the record index or record count
 */
struct intMsgPacket : public msgPacket {
	public:
		int val;
		
		/**
		 *@brief Default constructor for intMsgPacket.
		 */
		intMsgPacket() {
		}//end constructor
		
		/**
		 *@brief Constructs an intMsgPacket given its elements.
		 *@param sender The sender of the message's PID.
		 *@param cmd The command for the message.
		 *@param val The record count or record index to be transmitted in the message.
		 */
		intMsgPacket(pid_t sender, const char cmd[], int val) {
			this->sender = sender;
			strcpy(this->cmd, cmd);
			this->val = val;
		}//end constructor
		
};//end intMsgPacket

/**
 *@struct recMsgPacket
 *@brief Sub-Struct TCP message packet for transmitting records
 *@var recMsgPacket::record 
 * The container for the record string
 */
struct recMsgPacket : public msgPacket {
	public:
		char record[MAXRECORDSIZE+1];
		
		/**
		 *@brief Default constructor for recMsgPacket.
		 */
		recMsgPacket() {
		}//end constructor
		
		/**
		 *@brief Constructs a recMsgPacket given its elements.
		 *@param sender The sender of the message's PID.
		 *@param cmd The command for the message.
		 *@param record The record to be transmitted in the message.
		 */
		recMsgPacket(pid_t sender, const char cmd[], char record[]) {
			this->sender = sender;
			strcpy(this->cmd, cmd);
			strcpy(this->record, record);
		}//end constructor
		
};//end recMsgPacket

/**
 *@struct intRecMsgPacket 
 *@brief Sub-Struct TCP message packet for transmitting a record and its index.
 *@var intRecMsgPacket::val 
 *  The record index
 *@var intRecMsgPacket::record 
 *  The container for the record string
 */
struct intRecMsgPacket : public msgPacket{
	public:
		int val;
		char record[MAXRECORDSIZE+1];
		
		/**
		 *@brief Default constructor for intRecMsgPacket.
		 */
		intRecMsgPacket() {
		}//end constructor
		
		/**
		 *@brief Constructs an intRecMsgPacket given its elements.
		 *@param sender The sender of the message's PID.
		 *@param cmd The command for the message.
		 *@param val The record count or record index to be transmitted in the message.
		 *@param record The record to be transmitted in the message.
		 */
		intRecMsgPacket(pid_t sender, const char cmd[], int val, char record[]) {
			this->sender = sender;
			strcpy(this->cmd, cmd);
			this->val = val;
			strcpy(this->record, record);
		}//end constructor
		
};//end intRecMsgPacket

/**
 *@struct logMsgPacket 
 *@brief Sub-Struct TCP message packet for transmitting log records
 *@var logMsgPacket::logRecord 
 *  The container for the log record string
 */
struct logMsgPacket: public msgPacket{
	public:
		char logRecord[MAXLOGRECORDSIZE];
		
		/**
		 *@brief Default constructor for logMsgPacket.
		 */
		logMsgPacket() {
		}//end constructor
		
		/**
		 *@brief Constructs a intMsgPacket given its elements.
		 *@param sender The sender of the message's PID.
		 *@param cmd The command for the message.
		 *@param logRecord The log record to be transmitted.
		 */
		logMsgPacket(pid_t sender, const char cmd[], char logRecord[]) {
			this->sender = sender;
			strcpy(this->cmd, cmd);
			strcpy(this->logRecord, logRecord);
		}//end constructor
			
};//end logMsgPacket

/**
 *@struct serMsgPacket 
 *@brief Union Struct TCP message packet for receiving messages on server side
 */
struct serMsgPacket : public intRecMsgPacket {
	public:
	
		/**
		 *@brief Default constructor for serMsgPacket
		 */
		serMsgPacket() {
		}//end constructor
		
		/**
		 *@brief Constructs a serMsgPacket given its elements.
		 *@param sender The sender of the message's PID.
		 *@param cmd The command for the message.
		 *@param val The record count or record index to be transmitted in the message.
		 *@param record The record to be transmitted in the message.
		 */
		serMsgPacket(pid_t sender, const char cmd[], int val, char record[]) {
			this->sender = sender;
			strcpy(this->cmd, cmd);
			this->val = val;
			strcpy(this->record, record);
		}//end constructor
	
	
};//end serPacket
