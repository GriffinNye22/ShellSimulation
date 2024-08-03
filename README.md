# ShellSimulation
A Shell Simulation for processing retrieval, addition, and update requests from multiple clients to a binary-encrypted date file on a server using Message Queues, Semaphores, IPC, and Shared Memory. Has a few concurrency issues, but showcases my design skills and low-level programming skills.

|                       |                                                                                                                               |
|----------------------:|-------------------------------------------------------------------------------------------------------------------------------|
|**Last Modified Date:**| 5/26/21                                                                                                                       |
|**Last Modifications:**| Implemented SemaphoreSet & SharedMemoryManager Wrapper Classes and LogBinRWSemMonitor Class (with some concurrency issues :( )|
|     **Documentation:**| [Doc Site](https://codedocs.xyz/GriffinNye22/ShellSimulation/)                                                                |  
|                       | [Design Docs](https://github.com/GriffinNye22/SkipBo/tree/main/Design%20Documents%20%26%20Page%20Views)                       |
|          **Platform:**| UNIX                                                                                                                          |
|          **Language:**| C                                                                                                                             |

## Purpose:
The purpose of this project was to utilize a dataset chosen from Kaggle.com to 
analyze the data while also learning how to make concurrent operations
on the dataset from multiple clients. I originally selected a dataset on
video-game related sales from 1990-2020. However, I cleansed and shrunk the dataset
down to only include dates from July 2019 - June 2020. I wanted to see
the effects of the COVID-19 pandemic on the video game industry, but did not notice when 
selecting my data originally that for some reason the Software field of the data does not 
have any entries from July 2020 to December 2020. In addition, from July 2020 
onward a new field for Game Content is introduced in the data. At first, I 
believed it was possibly just a rewording for the Software field (it seemed 
unlikely, but one ended and one began on the same month, which was odd) but 
upon further inspection, the revenues for the Game Content field were 
tremendously higher than the revenues for the Software field for the months 
before. This lack of a Software field and addition of a new field would 
have skewed the data, especially since one of the fields is a total field,
so in order to maintain 4 fields of data I changed the date range of the dataset 
to include the last month possessing a value for the Software field, while still
maintaining a full year of data.


## Design Details  

### Commands:  
**CNT-** Notifies the server to retrieve and send the total number of records in the bin file.  
**GET-** Notifies the server to retrieve one or more records indicated by the provided index.  
**FIX-** Instructs the server to update the record found at the provided index with the record provided.  
**NEW**- Notifies the server to add the provided record  
**LOG**- Notifes the server to send its log records one-by-one  

### Packet Types:  

```cpp
Packet {
  long msgType; //Recipient of the message  
  pid_t sender; //Sender of the message  
  char[3] cmd;  //Command to be sent  
}  
```
Packet is the generic packet used to send commands that require no additional
arguments such as CNT, where we request the number of records in the bin file.  

**COMMANDS USED WITH:** CNT (client request), LOG (client request)  

```cpp
intPacket: public Packet {
  long msgType; //Recipient of the message
  pid_t sender; //Sender of the message
  char[3] cmd;  //Command to be sent
  int val;      //Integer value to be sent (# of records, # of log Records)
}
```
intPacket is a sub-struct of the generic Packet used in cases where an integer
needs to be sent, such as the server response to the CNT command.

**COMMANDS USED WITH:** CNT (server response), GET (client request), LOG (server response for record count)

```cpp
recPacket: public Packet {
  long msgType;                 //Recipient of the message
  pid_t sender;                 //Sender of the message
  char[3] cmd;                  //Command to be sent
  char record[MAXRECORDSIZE+1]; //A record from the binFile
}
```
recPacket is a sub-struct of the generic Packet used when entire records
need to be transmitted between the client and the server.

**COMMANDS USED WITH:** GET (server response), NEW (server response)

```cpp
intRecPacket: public Packet {
  long msgType;                 //Recipient of the message
  pid_t sender;                 //Sender of the message
  char[3] cmd;                  //Command to be sent
  int val;                      //Index of the record
  char record[MAXRECORDSIZE+1]; //Updated record (or success/failure message)
}
```
intRecPacket is a hybrid packet of intPacket and recPacket, containing both
a char array for a record and an integer value for an index and is used in
cases where both a record and its index must be transmitted, such as the FIX
command.

**COMMANDS USED WITH:** FIX (client request & server response), NEW (client request)

```cpp
serPacket: public intRecPacket {
  long msgType;                 //Recipient of the message
  pid_t sender;                 //Sender of the message
  char[3] cmd;                  //Command to be sent
  int val;                      //Integer Value to be sent (# of records, # of log Records, index of record)
  char record[MAXRECORDSIZE+1]; //Record string
}
```
serPacket is a union struct of all packet types that is used by the server
for reading incoming messages. Its relevant fields are populated and used
to construct the appropriate packet type to be sent back to the client.

**COMMANDS USED WITH:** ALL RECEIVED MESSAGES ON SERVER

```cpp
logPacket: public Packet {
  long msgType;                  //Recipient of the message
  pid_t sender;									 //Sender of the message
  char[3] cmd;                   //Command to be sent
  char record[MAXLOGRECORDSIZE]; //Log record string
}
```
logPacket is a sub-struct of the generic Packet used to send log records to 
the client when the Show Log menu option is selected.

**COMMANDS USED WITH:** LOG (Server sending log records)

### DESIGN:

**NOTE:** record indexes are 1-based in my implementation.
	
For my design, I decided that I wanted to make everything as modular as possible.
I noticed there are a lot of instances where the same data needs to be sent
or received in multiple places, so I wanted to be able to reuse as much code as
possible to make my job a lot easier. 

I began my design by defining methods that would need to be used on both the 
client and the server. At this point in time, these methods were those for 
displaying menus, executing the required actions for a menu option, retrieving 
user input where required, sending and receiving messages,and basic file 
operations on the server side such as getting total records or adding a record. 

Sending and receiving packets on the client-side were both separated into their own
respective methods, sendMsg() and receiveMsg(), so as not to clutter the code with a
mass of msgsnd() and msgrcv() system calls. These methods utilize the polymorphic
nature of the message packets and the Liskov Substitution Principle by accepting a
template parameter of a defined type: MsgPacket, allowing me to simply pass whatever
message packet type needs to be sent or received to these methods, reducing the need to
write multiple sendMsg() and receiveMsg() methods with various signatures. Sending
messages on the server-side behaves in the same manner, however, receiving messages on
the server-side utilizes a union packet: SerPacket, which contains fields for all of the
possible packet types the server could receive on the message queue.

From here I began developing diagrams similar to a sequence diagram for the
interactions between client and server. The first diagram describes the general overall
structure of almost all client-server interaction: creating and connecting to the queue,
opening the file on the server side, getting menu input from the client, executing
the appropriate action on client side, sending message to server, executing the 
appropriate action on the server side, logging the request, and sending a message
back to the client. 

Once I developed this sequence for basic interaction, I then developed an individual
diagram in the same way for each of the 4 menu options, so as to get a better
visualization of what needed to be accomplished for each. This not only made implementing
the code a lot easier, but also allowed me to see where code could be reused between
menu options, thus enhancing the modularity of my design.

Once these documents were created, I began the implementation for the client and
server by getting them both to startup properly.

**STARTUP:**
  > Starting both the client and server up was pretty simple. Server creates the message queue and then opens the bin file, exiting if either of those fail, while the client attempts to connect
  > to the message queue and closes on failure of that. Connections were handled on the server side using two methods: setupConnection(): which creates a socket, binds the server address to said
  > socket, and specifies it as the listening socket; and awaitConnections(): which accepts incoming client connection requests and forks a child server for each successful request.
		
Once the Startup was complete, I implemented the display of the menu and user input on the client side.
	
**MENU:**
  > Menu is displayed to the user via displayMenu(). getMenuInput() is then called to retrieve the user's input. getMenuInput() is self-validating, so if the  user's entry is not a valid command,
  > it returns the value of a recursive call to itself, so that upon valid entry, the user's valid entry bubbles up to the surface and is returned to main(). I also noticed that entering more than
  > a single character caused issues because of the recursive call (entering -1 would prompt the user twice more because of the unused 1), so I tweaked it a bit to accept up to 80 characters of
  > (obviously invalid) input that will be discarded, so that the user simply receives another prompt.

Once the menu was complete, I then began implementing the Display Record menu option,
as it would be needed to check the success of all other menu options and also
contained a lot of code that could be reused for the other menu options, especially
Change Record. We'll go over this one a little more in depth, so as to understand
the basic structure of the client and server for handling messages.
	
**DISPLAY RECORD:**  
  > When the user chooses Display Record from the menu, the displayRecord() method is called which performs all the necessary method calls. First, displayRecord() needs the number of records, but
  > since this will be needed for other menu options, as well, I decided to implement the retrieval of the number of records into its own method getCount().
		
  >**GETCOUNT():**
>   > getCount() is passed the message queue ID and the client PID, since it will be sending and receiving messages to/from the server. The client constructs a generic Packet with msgType 1,
>   > sender cliPID, and cmd "CNT". This message packet is then passed along with the message queue ID to sendMsg() as its second parameter, which accepts any Packet type using a template class
>   > MsgPacket. I decided to design it this way to make sending messages simpler, so that all I have to do is package up the message appropriately and pass it to be sent. The addition of the
>   > template class was also very useful as I considered creating multiple method definitions for each packet type, but was now able to use just a single method. The receiveMsg() method on the
>   > client side is designed in the same way, except it passes the MsgPacket by reference so as to keep the changes outside of the method's scope. This way, to receive a message, I just need to
>   > pass the message queue id, the client's pid, and an instance of whatever packet type the client is expecting to receive.
>   > 
>   > Meanwhile, the server is infinitely looping in receiveMsgs(), awaiting incoming messages. The server receives and stores all messages in a serPacket, which inherits from all other Packet
>   > types. I designed it this way because unlike the client, the server does not know which packet type to expect, as multiple clients are making requests at once and the menu operations are
>   > not performed atomically. Upon receipt of the message, it passes the message, the message queue ID and the bin file's file pointer to handleCmd(), which essentially acts as a switch
>   > on the cmd field, performing the appropriate actions for each command.
>   >
>   > For the CNT command, the server calls getTotalRecords(), which calculates
>   > the total number of records in the file by seeking to the beginning of the
>   > file and incrementing a ctr while looping with fgets() through the file. Once
>   > the number of records is calculated, the msgType and the sender of the serPacket
>   > are updated and it's msgType, sender, and cmd fields along with the total
>   > number of records are used to construct an intPacket, which is then passed
>   > to sendMsg() on the server side. sendMsg() is designed the same way as on the
>   > client side, allowing all message packet types to be sent through this single method.
>   >
>   > Back on the client side, getCount() passes the message queue id, the client's PID and an intPacket to receiveMsg(), where it receives the server's response. getCount() then returns the val
>   > field of the received intPacket back to displayRecords().
		
  > Once the number of records is retrieved, promptSelRecord() is called and passed the number of records and a value of true to include the all records option (-999), in which the client is
  > prompted to provide an index between 1 and the value passed or -999. This method is also self-validating, however, not by recursion.
  >
  > The desired index is then returned and another intPacket is constructed with proper msgType and sender, the command "GET", and the desired index. This is then sent to the server with
  > sendMsg() and the Data Labels are printed to the screen to prepare for printing the record(s).
  >
  > On receipt of this message, the server passes it to handleCmd() along with msgID and cliPID and checks the value of the val field. If the value is not -999, it calls sendRecord() passing it
  > the file pointer, msgID, desired index, and the pid of the requesting client. sendRecord() then calls getRecord(), passing it the filepointer and the desired index, which uses fseek() and
  > fgets() to retrieve and return the desired record. sendRecord() then constructs a recPacket, setting its msgType and sender appropriately, and inserting the selected record into it before
  > passing it to sendMsg() to be sent back to the client.
  >
  > If the desired index value was -999, sendAllRecords() is called which calls getTotalRecords() to calculate the total number of records, which is then used to iterate through each possible
  > record index, passing said record index as its idx parameter. sendAllRecords() then returns the number of records sent to the client. After a record(s) have been sent to the client,
  > logRequest() is called to log the operation in the server log which is serverLog.txt by default.
  
  > **LOGREQUEST():**
>   > LogRequest() takes client PID, the first character of the issued command, the number of records stored on server/sent to client, and the index of the desired record (or -999). Number of
>   > Records and index of desired record parameters have default values of -1, as they are not all needed for each command. For example, for the CNT command, logRecord() will be called as so:
>   > logRecord(cliPID, 'C', numRecords), as no index value exists for this command. For another example, the GET command with index 1, logRecord() will be called like this:
>   > logRecord(cliPID, 'G', -1, 1);
		
  > Back on the client side, a recPacket is passed to receiveMsg() and the message containing the record is received. The record field of the recPacket is passed to printRecord, which constructs
  > an instance of the DataRecord class and calls its printRecord() function which prints the record. I decided to reuse this DataRecord class, as it already performs the
  > parsing in its constructor which simply requires a string argument. The DataRecord class also contains methods to automatically set the precision for all float data types.
  >
  > In the case that the user entered -999, the receiveMsg() and printRecord() calls are simply looped as many times as there are records.
  >
  > displayRecord() then ends by returning the selected record back to main, prompting for another menu option.
		
**CHANGE RECORD:**
  > changeRecord() first calls displayRecord() to prompt the user for the index of a record to be displayed. displayRecord() is passed the message queue ID,  the client's PID and a value of
  > false to indicate that the all records option (-999) should not be made available to the user. displayRecord() then returns the retrieved record as an instance of the DataRecord object, which
  > will be used to update the record.
  >
  > changeRecord() then calls displayFieldMenu() which will print the menu options for the editable fields. I chose to only allow the Accessories, Hardware, and Software fields to be edited, as
  > editing the date would allow holes in the data and updating the Total field without updating any of the other 3 fields would cause the Total field to become meaningless. So, instead, the
  > total field will automatically be updated on editing any of the other 3 fields, so as to accurately represent the total.
  >
  > changeRecord() then calls getMenuInput() which prompts the user to select an editable field from the menu. I decided at this point to change getMenuInput() to include a boolean parameter
  > indicating whether or not the menu input was for the main menu or not. I was going to create a separate method for the field menu input, but realized the code was exactly the same as
  > getMenuInput() with the exception of the valid characters, so instead, I added the boolean parameter to determine which set of valid characters the input should be checked against. So, in
  > this case, since getMenuInput() is receiving input for the field menu and not the main menu, getMenuInput() will be passed false. getMenuInput() will then return the char representing the
  > field selected from the menu.
  >
  > This char is then passed to the promptFloatField() method, which prompts the user for a float field value, validates it, reprompting if necessary, and then returns the entered value. I wrote
  > this method generically, using the char passed to the method to determine which field to prompt for, as this method can later be reused for adding a new Record.
  >
  > Once promptFloatField() returns, the field values can now be updated. In order to do this, I first added set<Fieldname>() methods to the DataRecord class, as they were not needed in the last
  > project. Additionally, I added a method to DataRecord that updates the value for Total, based off of the values for the Accesssories, Hardware, and Software fields called updateTotal() and I
  > also implemented a toCString() method that returns the DataRecord object as a C-string, allowing easier conversion to be passed to the server. A default constructor was also added to
  > DataRecord to allow for declaration and initialization to occur separately.
  >
  > Now that these are added, a switch is simply used to determine which one to call based off of the user's selection from the menu. Then the updateTotal() method of the DataRecord is called to
  > update the total. An intRecPacket is constructed with the appropriate msgType and sender, a cmd value of "FIX", a record value of the DataRecord.toString() and a val value of the record's 
  > index. The intRecPacket is then sent to the server using sendMsg().
  >
  > When the server receives the intRecPacket containing the updated record, it calls handleCmd() which calls updateRecord() passing it the file pointer, the index of the record, the updated
  > record, and the record size. updateRecord() seeks to the record and overwrites it in the file. updateRecord() returns a boolean value indicating its success. An intRecPacket is then
  > constructed, setting the proper msgType and sender, inserting the index of the updated record, and the accompanying acknowledgment message (SUCCESS or FAILURE). This packet is then sent to
  > the client via sendMsg(). The server then logs the request, by calling logRequest(), passing it the client's PID, a cmd value of 'F' to indicate the server received the "FIX" command, -1 for 
  > the numRecords as it is irrelevant for this operation, and the index of the updated record.
  >
  > Upon receiving the acknowledgment packet, the client notifies the user of the outcome of updating the record.

**NEW RECORD:**
  > newRecord() starts by calling promptNewRecord(), which prompts the user for all the fields for a new record. promptNewRecord() first calls promptMonth() which prompts the user for a month
  > entered as its 3-letter abbreviation. It checks for valid input using a class called stringToMonthConverter which contains a map of 3 character strings to elements of a month enumerated
  > type. This map is populated upon construction of the object. In addition to the map and the constructor, the class contains a method isMonth() which accepts a string and returns a boolean
  > value indicating whether the string is a valid 3-letter month abbreviation. promptMonth() will continue to prompt the user until valid input is provided, upon which it returns the value
  > entered for month.
  >
  > promptNewRecord() combines the results of promptMonth() with the results of promptYear() to form the monthYear field of the new record. promptYear() prompts the user for a year. The year
  > must be in 2 digit form and must also be less than or equal to the current year, which is defined as a constant in msgQPackets.h, so for example, the year 2015 must be entered as 15. This
  > method continues to prompt the user until valid input is provided.
  >  
  > The accessories, software, and hardware fields are prompted by reusing the promptFloatField() used in changeRecord(), passing the associated character for each. The total field is then
  > calculated by combining the values of all of these 3 fields.
  >
  > All of the aforementioned fields are used to construct a DataRecord object, which is returned to the newRecord() method. newRecord() then begins constructing an intRecPacket using the
  > toString() method of the DataRecord object to populate its record field.
  >
  > A recPacket was originally going to be used for this method, however, since the server uses a serPacket to receive any incoming messages, the val data member was declared before the record
  > field in the struct and recPacket does not contain a val field, the record field of the received message was being offset by 4 bytes, cutting off the month in the record string. Attempting
  > to switch the order of declaration within the intRecPacket struct fixed this issue, however, it caused the displayRecord option to become unusable, as the val field of the received message
  > on the server side was being corrupted, due to intPacket's lack of a record field. Therefore, the most effective solution was to utilize an intRecPacket and pass a val field value of -1. This
  > issue was not made apparent until now, as although recPackets were transmitted previously, they were transmitted by the server side to the client side, where the expected packet type is
  > passed to receiveMsg(), whereas in this instance, the server does not know what type of packet to expect.
  >
  > The constructed intRecPacket is then sent to the server using sendMsg(). Upon receipt of the message on the server side, the addRecord() method is called, passing it the file pointer, the
  > record string, and the record size. addRecord() uses the previously discussed getTotalRecords() method to calculate the number of records and then reuses updateRecord() by passing it the
  > file pointer, the record, the record size, and the total number of records + 1 for its index field, as the new record will be appended. It then returns the return value of this call, which
  > is the boolean result of its success.
  >
  > A recPacket is then constructed, setting its appropriate message type and sender, and attaching the accompanying acknowledgment message (SUCCESS or FAILURE) in the record field of the
  > packet. This packet is sent to the client via sendMsg() and the request is logged by calling logRequest, passing it the client's PID and a cmd value of 'N' to indicate the server received the
  > "NEW" command.
  >
  > Upon receipt of the message on the client side, the user is notified of the outcome of the command via displaying a success or failure message.
		
**SHOW LOG:**
  > showLog() first constructs a base Packet, setting the proper sender and msgType and the command "LOG". This packet is then sent to the server via sendMsg().
  >
  > Upon receipt of the message, the server passes it to handleCmd(), along with the message queue ID, and the file pointer to the bin file. From here, getTotalLogRecords() is called which
  > calculates the total number of log records in the server log in a similar fashion to getTotalRecords() using fgets(). The MAXLOGRECORDSIZE is used in this method to allow the buffer to store 
  > all records. The LOGFILENAME is also stored as a constant in server.cpp's main(), so as to allow easier updating of the server log file name, in the case that it needs to be changed.
  >
  > MAXLOGRECORDSIZE is set at 64, as the longest log entry message in LogRequest() is the server response for showing the log, which contains 56 characters, plus a maximum of 5 characters for
  > the pid (32768 is max), and an extra 3 characters for the record number, as the bin file is highly unlikely to exceed 999 records at this point in time. If at any point the storage capability
  > of the bin file exceeds this assumption, the MAXLOGRECORDSIZE constant can easily be updated in the MsgQPackets.cpp file.
  >
  > The server then sends the number of total log records back to the client via sendMsg(). After sending the number of total log records back to the client, the server then calls sendLog(),
  > passing it the message queue ID, the client's PID, and the number of records in the log file.
  >
  > sendLog() opens the file for reading, and loops for as many records there are in the log file. For each loop, sendLogRecord() is called and passed a file pointer to the log file, the message queue ID,
  > and the client's PID.
  >
  > sendLogRecord() reads a record from the log file, constructs a logPacket using the proper msgType and sender, and the retrieved log record. The log record is then sent to the client via
  > sendMsg(). The server then logs the operation in the server log via logRequest().
  > 
  > Back in the client, after receiving the total number of log records, the client calls printLogRecords() passing it the message queue ID, the client's PID, and the total number of log
  > records. From here, the client receives each log record and prints it to the screen.

**Readers-Writers Problem:**  
  
  For the Readers-Writers Problem, I first designed and implemented a wrapper class, 
  SemaphoreSet, that constructs a System V semaphore set given a key, the number of 
  semaphores, and the perm flags. The class contains methods for all standard semaphore
  operations, such as: signal(), wait(), set(), get(), getAll(), setAll(), retrieving
  the number of processes waiting for a semaphore's value to increase, and retrieving
  the number of semaphores in the set. I decided to implement this class to keep my
  code organized and not clutter the code with the construction of semun and sembuf 
  structures for semop() and semctl() calls. 
  
  I then designed and implemented a class, LogBinRWSemMonitor, that implements a Monitor
  for synchronizing readers and writers for both the bin and log files using the 
  SemaphoreSet class. The Monitor is constructed using the key to be used for the 
  SemaphoreSet. add\*\*\*Reader() and add\*\*\*Writer() methods perform the pre-CS semaphore
  operations and the rem\*\*\*Reader() and rem\*\*\*Writer() methods perform the post-CS
  semaphore operations. This further increased the organization of my code and improved
  the simplicity of handling Critical Sections on the server-side, as each call to a
  purely reading and writing method simply must be enclosed with the corresponding
  add\*\*\* and rem\*\*\* methods. Additionally, my design of these classes allowed me 
  to maintain the Monitor objects between parent and child server processes utilizing
  the semget() call's retrieval of the semaphore id for a provided key. This allowed
  me to simply construct the Monitor object in the parent and initialize the default 
  semaphore values using its init() method and access said object by constructing
  a "new" Monitor object in the child process, which would simply load and retrieve
  the existing semaphore set into my SemaphoreSet wrapper class. The reason the default
  semaphore values are set in init() and not the constructor is due to the fact that
  readers and writers may be active when a client connects and constructs their version
  of the Monitor and contained SemaphoreSet. If these values were set in the constructor,
  they would reset the semaphore values, thus causing a massive risk of deadlock. Thus,
  init() only needs to be called by the parent process. This method proved to be much 
  simpler than attempting to use the boost::interprocess library to write the
  structure into shared memory.
  
  For my Monitor class, I utilized a few more than the recommended amount of semaphores,
  as I wanted to implement a algorithm with strong writer's preference that also allowed
  concurrent readers for the dataset. I decided to do this to ensure that clients 
  retrieving data from the server are provided with as up-to-date information as possible, 
  as I pictured the following scenario: Suppose a record has an incorrect value and two clients 
  are double-checking the data was entered correctly. Client A requests to edit a record
  and submits their changes, meanwhile Client B subsequently requests to edit the same record. 
  With reader's preference, if the readers held the critical section, Client A's
  changes would not make it through before Client B receives the record back from 
  the server and begins to make the same change. Client B therefore unnecessarily 
  issues a command, or produces an unintended result (in a more general case where 
  the same field is not edited by both parties). This could simply be avoided by
  allowing the writer to momentarily suspend all readers while it updates the record.
  While the effect of this seems small, its effects quickly scale with the number of
  clients. This same concept applies for adding new records. 
  
  For the dataset, I utilized 2 mutexes for the reader and writer counts, a mutex
  for the writer's CS access, a mutex acting as a condition variable to catch all 
  readers that entered the reader count section yet had not entered the CS when a 
  writer arrives, and another mutex acting as a condition varible to make an arriving 
  writer wait for readers in the critical section to finish executing before
  it gains entry (these condvar Semaphores also include the unheard signal function).
  Additionally, I utilized 2 counting semaphores to track the number of readers and
  writers by initializing both to 0 and performing signal on entry and wait on exit.
  
  For the log file, I decided to utilize a weak reader's preference with concurrent
  reader access, as the strong writer's preference would most certainly starve any
  readers due to the frequency of logging operations, especially once the scale increases.
  I achieved this using 3 semaphores: a mutex for readerCount, a mutex for writer 
  access and making readers wait until writers are finished, and a counting semaphore
  for the readercount.

  Seeing as though the server needed to manage the manipulation of a potentially
  unbounded dataset receiving requests by a conceivably large number of clients, I needed
  to apply the principle of Parsimony for a few critical aspects in order to minimize my
  application’s use of system resources. First, I applied this principle when it came to
  transmitting messages between the server and all of its connecting clients, by only
  transmitting data necessary for the issued command, increasing the application’s
  responsiveness. When it came to transmitting the contents of the dataset or the logfile, I
  ensured that only one record was transmitted and stored at a time, as the dataset could
  presumably continue to grow over time. While this required an increased number of
  transmitted messages, it simultaneously held allocation on the stack to a minimum, since
  a retrieved record is removed from memory by the time the next data record is retrieved,
  which I found to be much more advantageous. Lastly, when it came to updating records, I
  wanted to make sure that write operations to the dataset were kept to a minimum by only
  updating the single specified record in the binary data file rather than rewriting the file
  with the updated record included.

**Known Bugs:**  

  Unfortunately I was unable to implement Shared Memory or Signals, as I really
  struggled with creating an algorithm for handling the Readers-Writers Problem.
  It had seemed that I had developed a working solution, as the majority of my tests
  were working for me and printing out the status of the readers and writers seemed 
  to show that writers and readers were not accessing the Critical section at the same time,
  however now I seem to be receiving a lot of invalid argument instances, most notoriously
  a record Count of 0, which causes infinite looping on the selection prompt, and 
  an extremely high record count over 5 million. Additionally, I ran into some issues 
  where the server would receive messages two or more times from the client if their last message
  was a writer before receiving SIGPIPE.
