#Makefile
#Author: Sivert Andresen Cubedo

CC = g++
FLAGS = -Wall -Werror -std=c++11 -DLINUX

BINARIES = main MIP_deamon routing_deamon
OBJECTS = MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o AddressTypes.o LinuxException.o EventPoll.o DistanceVectorTable.o TimerWrapper.o

FILES =	$(BINARIES) $(OBJECTS)

all: $(FILES)

#binary
main: src/main.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o main src/main.cpp $(OBJECTS)

MIP_deamon: src/MIP_deamon.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o MIP_deamon src/MIP_deamon.cpp $(OBJECTS)

routing_deamon: src/routing_deamon.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o routing_deamon src/routing_deamon.cpp $(OBJECTS) -pthread

#objects
MIPFrame.o: src/MIPFrame.cpp include/MIPFrame.hpp
	$(CC) $(FLAGS) -c src/MIPFrame.cpp

EventPoll.o: src/EventPoll.cpp include/EventPoll.hpp
	$(CC) $(FLAGS) -c  src/EventPoll.cpp

CrossForkExec.o: src/CrossForkExec.cpp include/CrossForkExec.hpp
	$(CC) $(FLAGS) -c src/CrossForkExec.cpp

CrossIPC.o: src/CrossIPC.cpp include/CrossIPC.hpp
	$(CC) $(FLAGS) -c src/CrossIPC.cpp

RawSock.o: src/RawSock.cpp include/RawSock.hpp
	$(CC) $(FLAGS) -c src/RawSock.cpp

AddressTypes.o: src/AddressTypes.cpp include/AddressTypes.hpp
	$(CC) $(FLAGS) -c src/AddressTypes.cpp

LinuxException.o: src/LinuxException.cpp include/LinuxException.hpp
	$(CC) $(FLAGS) -c src/LinuxException.cpp

DistanceVectorTable.o: src/DistanceVectorTable.cpp include/DistanceVectorTable.hpp
	$(CC) $(FLAGS) -c src/DistanceVectorTable.cpp

TimerWrapper.o: src/TimerWrapper.cpp include/TimerWrapper.hpp
	$(CC) $(FLAGS) -c src/TimerWrapper.cpp

clean:
	rm $(FILES)


