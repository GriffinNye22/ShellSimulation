debug = -g

all: createBin server client

clean:
	\rm -f *.o
	\rm -f *.bin
	\rm client
	\rm server
	\rm ser.log
	\touch ser.log
	./createBin gameRevenue.csv gameRevenue.bin

createBin: createBin.o
	g++ -o createBin createBin.o $(debug)

client: client.o DataRecord.o msgPackets.o SharedMemoryManager.o
	g++ -std=c++1z -o client client.o $(debug)

server: server.o msgPackets.o SemaphoreSet.o LogBinRWSemMonitor.o 
	g++ -std=c++1z -o server server.o msgPackets.o SemaphoreSet.o LogBinRWSemMonitor.o $(debug)

createBin.o: createBin.cpp 
	g++ -c createBin.cpp $(debug)

DataRecord.o: DataRecord.cpp
	g++ -c DataRecord.cpp $(debug)

LogBinRWMonitor.o: LogBinRWSemMonitor.cpp
	g++ -c LogBinRWSemMonitor.cpp $(debug)

msgPackets.o: msgPackets.cpp
	g++ -c msgPackets.cpp $(debug)

SemaphoreSet.o: SemaphoreSet.cpp
	g++ -c SemaphoreSet.cpp $(debug)

SharedMemoryManager.o: SharedMemoryManager.cpp
	g++ -c SharedMemoryManager.cpp $(debug)

client.o: client.cpp 
	g++ -c client.cpp $(debug) 

server.o: server.cpp
	g++ -c server.cpp $(debug)
