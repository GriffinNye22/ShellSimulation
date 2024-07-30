/**
 * @file client.cpp
 * @author Griffin Nye
 * @brief Client side of the TCP Distributed Information System implementation
 * CSC552 Dr. Spiegel Spring 2020
 */


#include <iostream>
#include <netdb.h>
#include <string>
#include <sys/socket.h>
#include <unistd.h>

#include "DataRecord.cpp"
#include "msgPackets.cpp"


using namespace std;

/**
 *@brief Handles client-server and user-client interaction for the Change Record menu option.
 *@param sockfd The currently connected sockets file descriptor.
 *@param myPID This client's PID.
 */
void changeRecord(int sockfd, pid_t myPID);

/**
 *@brief Attempts to connect to the server. Returns socket file descriptor on successful connection.
 *@param SERVER_PORT The server's dedicated port number.
 *@param SERVER_ADDRESS The server's address (stored as in_addr).
 *@return Socket file descriptor on successful connection; client exits on failed connection.
 */ 
int connect(int SERVER_PORT, struct in_addr *SERVER_ADDRESS);

/**
 *@brief Displays the menu of editable field options to the user
 */
void displayFieldMenu();

/**
 *@brief Displays the Main Menu to the user.
 */
void displayMenu();

/**
 *@brief Handles client-server and user-client interaction for the Display Record menu option.
 *@param sockfd The currently connected socket's file descriptor.
 *@param myPID This client's PID.
 *@param selectedMenuOption Whether or not Display Record is the currently selected menu option (false removes -999 option).
 *@return The retrieved record as an instance of DataRecord. (Last record retrieved for all records option (-999), however the return value is not used in this case).
 */
DataRecord displayRecord(int sockfd, pid_t myPID, bool selectedMenuOption);

/**
 *@brief Handles interaction with server for retrieving the record count.
 *@param sockfd The currently connected socket's file descriptor.
 *@param myPID This client's PID.
 *@return The number of records stored by the server.
 */
int getCount(int sockfd, pid_t myPID);

/**
 *@brief Retrieves the user's menu selection, prompting again for invalid entries.
 *@param mainMenu Whether or not the menu input is for the main menu
 *@return The user's valid menu selection as an uppercase character 
 */
char getMenuInput(bool mainMenu);

/**
 *@brief Handles client-server and user-client interaction for the New Record menu option
 *@param sockfd The currently connected socket's file descriptor.
 *@param myPID This client's PID
 */
void newRecord(int sockfd, pid_t myPID);

/**
 *@brief Prints the data labels for output records.
 */
void printDataLabels();

/**
 *@brief Receives and prints all log records sent by the server.
 *@param sockfd The currently connected socket's file descriptor.
 *@param myPID This client's PID.
 *@param numRecords The number of log records to receive from the server.
 */
void printLogRecords(int sockfd, pid_t myPID, int numRecords);

/**
 *@brief Prints the provided record.
 *@param record The record to be printed
 */
void printRecord(char record[]);

/**
 *@brief Prompts the user for one of the float fields for a new or updated record.
 *@param field A Character representing a field in the DataRecord
 *@return The value assigned to the field
 */
float promptFloatField(char field);

/**
 *@brief Prompts the user for the month for a new record. Validates input through recursive calls until valid input is given.
 *@return The month for the new record
 */
string promptMonth();

/**
 *@brief Prompts the user to create a new record.
 *@return The new data record
 */
DataRecord promptNewRecord();

/**
 *@brief Prompts the user for a record index between 1 and the provided max index. Validates input through recursive calls until valid input is given.
 *@param numRecords The maximum index that can be entered by the user.
 *@param allRecords Whether or not the all records option (-999) should be available.
 *@return The desired index specified by the user
 */
int promptSelRecord(int numRecords, bool allRecords);

/**
 *@brief Prompts the user for the year for a new record. Validates input through recursive calls until valid input is given.
 *@return The year for the new record
 */
string promptYear();

/**
 *@brief Handles the receipt of messages from the server.
 *@param sockfd The currently connected socket's file descriptor.
 *@param msg The expected message packet to be received.
 */
template<class MsgPacket> void receiveMsg(int sockfd, MsgPacket &msg);

/**
 *@brief Handles the transmission of messages to the server.
 *@param sockfd The currently connected socket's file descriptor.
 *@param msg The message packet to be transmitted
 */
template<class MsgPacket> void sendMsg(int sockfd, MsgPacket msg);

/**
 *@brief Handles client-server and user-client interaction for the Show Server Log menu option.
 *@param sockfd The currently connected socket's file descriptor.
 *@param myPID This client's PID.
 */
void showServerLog(int sockfd, pid_t myPID);

/**
 *@brief Calls the appropriate methods to ensure proper functionality of the client.
 *@param argc The number of command-line arguments
 *@param argv List of command-line arguments
 */
int main(int argc, char *argv[]) {
	const int SERVER_PORT = 15005;
	struct in_addr *SERVER_ADDRESS = (struct in_addr *) (gethostbyname("acad.kutztown.edu")->h_addr);

	char selection;
	int sockfd;
	pid_t myPID = getpid();

	//Connect to server
	sockfd = connect(SERVER_PORT, SERVER_ADDRESS);
	
	do {
		//Display Main Menu & get user input
		displayMenu();
		selection = getMenuInput(true);
		
		switch(selection) {
			case 'N':
				newRecord(sockfd, myPID);
				break;
			case 'D':
				displayRecord(sockfd, myPID, true);
				break;	
			case 'C':
				changeRecord(sockfd, myPID);
				break;
			case 'S':
				showServerLog(sockfd, myPID);
				break;
			case 'V':
				//viewClientLog(commfd);
				break;
			case 'L':
			  //listLocalClients(commfd);
				break;
			case 'E':
				cout << endl << "Client successfully closed..." << endl;
		}//end switch
		
	} while(selection != 'E');
	
}//end main

//Handles client-server and user-client interaction for the Change Record menu option.
void changeRecord(int sockfd, pid_t myPID) {
	char selectedField;
	float fieldValue;
	intRecMsgPacket updateMsg;
	string recordString;
	
	//Prompt user for record number and display it
	DataRecord retrievedRecord = displayRecord(sockfd, myPID, false);
	
	//Display menu of editable fields to the user
	displayFieldMenu();
	
	//Prompt for selected field to edit
	selectedField = getMenuInput(false);
	
	//Prompt the user to edit the field's value
	fieldValue = promptFloatField(selectedField);
	
	//Update the appropriate field
	switch(selectedField) {
		case 'A':
			retrievedRecord.setAccessories(fieldValue);
			break;
		case 'H':
			retrievedRecord.setHardware(fieldValue);
			break;
		case 'S':
			retrievedRecord.setSoftware(fieldValue);
			break;
	}//end switch
	
	//Update the total field
	retrievedRecord.updateTotal();
	
	//Assemble the edit record request message
	updateMsg = intRecMsgPacket(myPID, "FIX", retrievedRecord.getRecordIndex(), &retrievedRecord.toString()[0] );
	
	//Send Updated record to server
	sendMsg(sockfd, updateMsg);
	
	//Receive acknowledgment from server
	receiveMsg(sockfd, updateMsg);
	
	//Notify user of outcome
	if( strcmp(updateMsg.record, "SUCCESS") == 0) {
	  //Notify user of successful update
		cout << endl << "Record #" << updateMsg.val << " successfully updated." << endl;
	} else {
		//Notify user of failed update
		cout << endl << "Failed to update Record #" << updateMsg.val << "." << endl;
	}//end if
	
}//end changeRecord

//Attempts to connect to the server. Returns socket file descriptor on successful connection.
int connect(int SERVER_PORT, struct in_addr * SERVER_ADDRESS) {
  int sockfd;
	struct sockaddr_in server;

	//Create socket for connecting to server
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error connecting to server: ");
		cout << "Shutting down..." << endl;	
		exit(EXIT_FAILURE);		
  }//end if
	
  //Specify Server Address and Port
  server.sin_family = AF_INET;
  server.sin_port = htons(SERVER_PORT);
	
	struct in_addr * addr;
	struct hostent * he;
	
	he = gethostbyname("localhost");
    
	//  STORE IN STRUCT
	addr = (struct in_addr *) he->h_addr;
	server.sin_addr = *addr;
	
	//Connect to Server
  if( connect(sockfd, (struct sockaddr *) &server, sizeof(server) ) == -1) {
		perror("Failed to connect to server: ");
		cout << "Shutting down..." << endl;
		exit(EXIT_FAILURE);
  } else {
		cout << "Successfully connected to server." << endl;
  }//end if

  return sockfd;	
}//end connect	

//Displays the menu of editable field options to the user
void displayFieldMenu() {
	cout << endl << "Select Field to edit:" << endl;
	cout << "A)ccessories" << endl;
	cout << "H)ardware" << endl;
	cout << "S)oftware" << endl;
}//end displayFieldMenu

//Displays the Main Menu to the user
void displayMenu() {
	cout << endl << "Menu:" << endl;
	cout << "N)ew Record" << endl;
	cout << "D)isplay Record" << endl;
	cout << "C)hange Record" << endl;
	cout << "S)how Server Log" << endl;
	cout << "E)xit" << endl << endl;
}//end displayMenu

//Handles client-server and user-client interaction for the Display Record menu option.
DataRecord displayRecord(int sockfd, pid_t myPID, bool selectedMenuOption) {
	DataRecord retrievedRecord;
	int count = getCount(sockfd, myPID);
	int numRecords = 1;
	int recNum;
	intMsgPacket idxMsg;
	recMsgPacket recMsg;

	//Prompt user for index of desired record
	recNum = promptSelRecord(count, selectedMenuOption);
	
	//Assemble the record request message
	idxMsg = intMsgPacket(myPID, "GET", recNum);
	
	//Request record at index recNum
	sendMsg(sockfd, idxMsg);
	
	if(recNum == -999) {
		numRecords = count;
	}//end if
	
	//Print the Data Headings
	printDataLabels();
	
	for(int i = 0; i < numRecords; i++) {
		//Receive the requested record
		receiveMsg(sockfd, recMsg);
		
		//Construct DataRecord
		retrievedRecord = DataRecord(recMsg.record, recNum);
	
		//Print the requested record
		printRecord(recMsg.record);
	}//end for
	
	
	return retrievedRecord;
}//end displayRecord

//Handles interaction with server for retrieving the record count.
int getCount(int sockfd, pid_t myPID) {
	msgPacket msg;
	intMsgPacket responseMsg;
	
	//Assemble the record count request message
	msg = msgPacket(myPID, "CNT");

	//Request number of records	
	sendMsg(sockfd, msg);
	
	//Receive number of records
	receiveMsg(sockfd, responseMsg);
	
	return responseMsg.val;
}//end getCount

//Retrieves the user's menu selection, prompting again for invalid entries
char getMenuInput(bool mainMenu) {
		char * inp = new char[80];
	char sel;
	
	//Prompt selection
	cout << "Selection: ";
	cin >> inp;
	
	//RePrompt for non-char entries
	if( strlen(inp) > 1 ) {
		return getMenuInput(mainMenu);
	} else {
		sel = inp[0];
	}//end if
	
	//Convert to uppercase
	sel = toupper(sel);
	
	//Return selection if valid, otherwise prompt again
	if(mainMenu && (sel == 'C' || sel == 'D' || sel == 'E' || sel == 'N' || sel == 'S') ) {
		return sel;
	} else if (!mainMenu && (sel == 'A' || sel == 'H' || sel == 'S') ) {
		return sel;
	} else {
		return getMenuInput(mainMenu);
	}//end if
	
}//end getMenuInput

//Handles client-server and user-client interaction for the New Record menu option
void newRecord(int sockfd, pid_t myPID) {
	intRecMsgPacket recMsg;
	string recordString;

	//Prompt user for New Record values	
	DataRecord newRecord = promptNewRecord();
	
	//Get new record as string
	recordString = newRecord.toString();
	
	//Assemble the new record message
	recMsg = intRecMsgPacket(myPID, "NEW", -1, &recordString[0] );
	
	//Send new record to server
	sendMsg(sockfd, recMsg);
	
	//Receive acknowledgment message
	receiveMsg(sockfd, recMsg);

	//Notify user of outcome 
	if( strcmp(recMsg.record, "SUCCESS") == 0) {
		cout << endl << "Record successfully added." << endl;
	} else {
		//Notify user of failure to add record
		cout << endl << "Failed to add new record." << endl;
	}//end if

}//end newRecord

//Prints the data labels for output records.
void printDataLabels() {
	//Print Data Headings
	cout << endl;
  cout << right << setw(9) << "Month" << setw(13) << "Accessories" 
       << setw(11) << "Hardware" << setw(11) << "Software" << setw(9) << "Total"
       << endl;
}//end printDataLabels

//Receives and prints all log records sent by the server.
void printLogRecords(int sockfd, pid_t myPID, int numRecords) {
	logMsgPacket logMsg;
	
	cout << endl << "SERVER LOG:" << endl;
	
	//Receive and print all server log records
	for(int i = 0; i < numRecords; i++) {
		
		//Receive log record from server
		receiveMsg(sockfd, logMsg);
		
		//Print server log record
		cout << logMsg.logRecord;
		
	}//end for

}//end printLogRecords

//Prints the provided record.
void printRecord(char record[]) {
	
	//Convert the record from char array to string
  string strRecord(record);  
    
  //Construct DataRecord
  DataRecord data(strRecord);
    
  //Print record
  data.printRecord();
	
}//end printRecord

//Prompts the user for one of the float fields for a new or updated record.
float promptFloatField(char field) {
	string fieldName;
	float fieldValue;
	
	//Determine field being edited
	switch(field) {
		case 'A':
			fieldName = "Accessories";
			break;
		case 'H':
			fieldName = "Hardware";
			break;
		case 'S':
			fieldName = "Software";
			break;
		case 'T':
			fieldName = "Total";
			break;
	}//end switch

	//Prompt for field value, reprompt if invalid entry	
	do {
		cout << "Value for field " << fieldName << ": ";
		cin >> fieldValue;
	} while( cin.fail() );
	
	return fieldValue;
}//end promptFloatField

//Prompts the user for the month for a new record.
//Validates input through recursive calls until valid input is given.
string promptMonth() {
	//bool invalidInput = false;
	stringToMonthConverter monthConverter;
	string month;
	
	//Prompt until valid input given
	do {
		
		//Prompt for month and receive input
		cout << "Month (3 letter abbreviation): ";
		cin >> month;
		
		//Convert entry to capitalize the first letter only	
		month[0] = toupper(month[0]);
		
		for(int i = 1; i < month.length(); i++) {
			month[i] = tolower(month[i]);
		}//end for
		
		
		//Return entry if valid Month
		if( monthConverter.isMonth(month) ) {
			return month;
		} else {
			cout << "Please enter a valid 3-letter month abbreviation." << endl;
		}//end if

	} while(true);
	
}//end promptMonth

//Prompts the user to create a new record
DataRecord promptNewRecord() {
	DataRecord newRecord;
	float accessories;
	float hardware;
	float software;
	float total;
	string monthYear;
	string month;
	
	cout << "NEW RECORD:" << endl;	

	//Prompt user for record fields
	month = promptMonth();
	monthYear = month + " '" + promptYear();
	accessories = promptFloatField('A');
	hardware = promptFloatField('H');
	software = promptFloatField('S');
	
	//Calculate total
	total = accessories + hardware + software;
	
	//Construct new DataRecord
	newRecord = DataRecord(monthYear, accessories, hardware, software, total);
	
	return newRecord;
}//end promptNewRecord

//Prompts the user for a record index between 1 and the provided max index. 
//Validates input through recursive calls until valid input is given.
int promptSelRecord(int numRecords, bool allRecords) {
	char sel[5];
	
	//Display Number of records
	cout << endl << "There are " << numRecords << " records currently stored." << endl;
		
	//Continue to prompt until user provides integer in range.
	do {
		cout << "Desired Record (1-" << numRecords << "): ";
		cin >> sel;
	} while( (isdigit(sel[0]) == 0 && sel[0] != '-') || 
				 ( (atoi(sel) < 1 && ( !allRecords || atoi(sel) != -999 ) ) || 
						atoi(sel) > numRecords) );
	
	return atoi(sel);
}//end promptSelRecord

//Prompts the user for the year for a new record.
//Validates input through recursive calls until valid input is given.
string promptYear() {
	string year;
	
	do {
		
		cout << "Year (last 2 numbers): ";
		cin >> year;
		
		//Reprompt if user did not enter 2 digits for the year
		if( year.length() != 2) {
			//invalidInput = true;
			cout << "Year must be entered as 2 digits, for example, 2015 entered as 15)." << endl;
			continue;
		}//end if
		
		//Check entry is valid 2-digit year
		for(int i = 0; i < year.length(); i++) {
			
			if( isdigit(year[i]) == 0) {
				//invalidInput = true;
				cout << "Please enter a valid year as 2 digits.";
				break;
			} else if(i == year.length() - 1 ) {
				int numYear;
				
				numYear = stoi(year);

				//Check if year has happened yet				
				if(numYear <= CURRENTYEAR) {
					return year;
				} else {
					cout << "Year entered must be on or before the year 20" << CURRENTYEAR << "." << endl;
				}//end if

			}//end if

		}//end for
		
	}while(true);
	
}//end promptYear

//Handles the receipt of messages from the server.
template<class MsgPacket> void receiveMsg(int sockfd, MsgPacket &msg) {
	
	if( read(sockfd, &msg, sizeof(msg) ) == -1) {
		perror("Error receiving message from server: ");
	} else {
		
	}//end if
  
}//end receiveMsg

//Handles the transmission of messages to the server.
template<class MsgPacket> void sendMsg(int sockfd, MsgPacket msg) {
	
	if( write(sockfd, &msg, sizeof(msg) ) == -1) {
		perror("Error sending message to server: ");
	}//end if
	
}//end sendMsg

//Handles client-server and user-client interaction for the Show Server Log menu option.
void showServerLog(int sockfd, pid_t myPID) {
	intMsgPacket cntMsg;
	msgPacket cmdMsg;
	
	//Assemble Command message packet
	cmdMsg = msgPacket(myPID, "LOG");

	//Send LOG command to server
	sendMsg(sockfd, cmdMsg);
	
	//Receive Log Record Count from server
	receiveMsg(sockfd, cntMsg);
	
	//Receive and print the log records
	printLogRecords(sockfd, myPID, cntMsg.val);
}//end showLog
