/**
 *@file DataRecord.cpp
 *@author Griffin Nye
 *@brief A structure for storing, manipulating, and exporting records from the dataset
 */


#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;

/**
 * @brief Class for holding a single record of Data
 */
class DataRecord {
  private:
    string monthYear;
		int idx;
    float accessories;
    float hardware;
    float software;
    float total;
  
  public:
  
		/**
		 *@brief Default Constructor for the DataRecord object
		 */
		DataRecord() {
		}//end constructor
		
		/**
		 *@brief Constructs a DataRecord object using data field values
		 *@param monthYear The month and year of captured revenue
		 *@param accessories The generated revenue from accessories
		 *@param hardware The generated revenue from hardware
		 *@param software The generated revenue from software
		 *@param total The total generated revenue
		 *@param recIdx The index of the record, -1 by default
		 */
		DataRecord(string monthYear, float accessories, float hardware, float software, float total, int recIdx = -1) {
			this -> monthYear = monthYear;
			this -> accessories = accessories;
			this -> hardware = hardware;
			this -> software = software;
			this -> total = total;
			this -> idx = recIdx;
		}//end constructor
	
    /**
     *@brief Constructs a DataRecord object given a record string read from the bin file, by
     *       parsing it into the object's data members. 
     *@param record Record string read from the file.
		 *@param recIdx Index of the DataRecord in the server bin file
     */
    DataRecord(string record, int recIdx = -1)  {
			size_t pos;
			
			//Parse Month & Year
			pos = record.find(',');
			monthYear = record.substr(0,pos);
			record.erase(0, pos+1);
			
			//Parse Total Revenue
			pos = record.find(',');
			total = stof(record.substr(0,pos) );
			record.erase(0,pos+1);
			
			//Parse Hardware Revenue
			pos = record.find(',');
			hardware = stof(record.substr(0,pos) );
			record.erase(0,pos+1);
			
			//Parse Software Revenue
			pos = record.find(',');
			software = stof(record.substr(0,pos) );
			record.erase(0,pos+1);
			
			//Parse Accessories Revenue
			accessories = stof(record);
			
			idx = recIdx;
		}//end constructor
   
    /**
     *@brief Converts fields of type float to their fixed precision string types.
     *@param field Float Data field
     *@return A fixed precision float-converted string.
     */
    string fieldToString(float field) {
			stringstream ss;
			
			ss << fixed << setprecision(2) << field;
			
			return ss.str();
		}//end fieldToString
		
		/**
		 *@brief Retrieves the idx data member
		 *@return The value of the idx member
		 */
		int getRecordIndex() {
			return idx;
		}//end getRecordIndex
     
		/**
     *@brief Prints the DataRecord object in a well-formatted manner.
     */
    void printRecord() {
			cout << right << setw(9) << monthYear 
			<< setw(13) << "$" + fieldToString(accessories) + " bil"
			<< setw(11) << "$" + fieldToString(hardware) + " bil" 
			<< setw(11) << "$" + fieldToString(software) + " bil"
			<< setw(11) << "$" + fieldToString(total) + " bil" << endl;
		}//end printRecord
		
		/**
		 *@brief Sets the accessories data member to the provided value
		 *@param val Value to set data member to
		 */
		void setAccessories(float val) {
			accessories = val;
		}//end setAccessories

		/**
		 *@brief Sets the hardware data member to the provided value
		 *@param val Value to set data member to
		 */
		void setHardware(float val) {
			hardware = val;
		}//end setHardware

		/**
		 *@brief Sets the Software data member to the provided value
		 *@param val Value to set data member to
		 */
		void setSoftware(float val) {
			software = val;
		}//end setSoftware

		/**
		 *@brief Sets the total data member to the provided value
		 *@param val Value to set data member to
		 */
		void setTotal(float val) {
			total = val;
		}//end setTotal
		
		/**
		 *@brief Converts the DataRecord object to a string
		 *@return The string representation of the DataRecord object
		 */
		string toString() {
			return monthYear + "," + fieldToString(total) + "," + fieldToString(hardware) 
											 + "," + fieldToString(software) + "," + fieldToString(accessories) + '\0';
		}//end DataRecord
		
		/**
		 *@brief Updates the total field by summing the hardware, software, and accessories fields
		 */
		void updateTotal() {
			total = accessories + software + hardware;
		}//end updateTotal;
};//end DataRecord 
