/**
 * @file createBin.cpp
 * @author Griffin Nye
 * @brief CSC552 Dr. Spiegel Spring 2020 Transfers data from an input .csv file into an output binary-encoded
 *        file, whose filenames are provided as command-line arguments. Returns
 *        the number of lines read from the inFile or failed open flags.
 */


#include<cstring>
#include<fstream>
#include<iostream>

using namespace std;

/**
 *@brief Transfers data from the input csv file using File Pointers into the output binary file using file streams.
 *@param inFile Filename for the inFile.
 *@param outFile Filename for the outFile.
 *@return The number of lines read from the inFile. -1 indicates inFile_FAIL 0 indicates outFile fail.
 */ 
int transferData(char * inFile, char * outFile);

/**
 *@brief Ensures proper usage of the executable and calls for the transfer of the csv data to binary-encoded data.
 *@param argc Number of command line arguments
 *@param argv Array of command line arguments
 *@return The number of lines read from the inFile. -1 indicates inFile_FAIL, 0 indicates outFile fail.
 */
int main(int argc, char * argv[]) {
  
  //Print Usage statement if improper usage occurs
  if(argc!=3) {
    cout << "USAGE: ./createBin <.csv source file> <destination file>" << endl;
    return -2;
  } else {
    return transferData(argv[1], argv[2]);
  }//end if
  
}//end main
             
int transferData(char * inFile, char * outFile) {
  FILE * filePtr;
  ifstream in;
  int lines = 0;
  string buf;
  
  //Open the files
  in.open(inFile, ios::in);
  filePtr = fopen(outFile, "wb");
  
  //If the inFile fails to open, return inFile_FAIL flag
  if( in.fail() ) {
    cerr<< strerror(errno);
    return -1;
  //If the outFile fails to open or be created, return outFile_FAIL flag
  } else if(filePtr==NULL) {
    return 0;
  }//end if
  
  //Read & Transfer each line until the file is empty
  while(getline(in,buf,'\r') ) {
    
    //Ignore carriage return character
    in.ignore(1,'\r');
    
    //Copy std::string into c-string
    char charBuf[buf.length()+1];
    strcpy(charBuf, buf.c_str());
    charBuf[buf.length()+1] = '\n';
    
    //Write to binary-encoded file
    fwrite(&charBuf, sizeof(char), sizeof(charBuf)/sizeof(char), filePtr);
    
    //increment line count
    lines++;
  }//end while
  
  //Close the files
  in.close();
  fclose(filePtr);
  
  return lines;
}//end readData
