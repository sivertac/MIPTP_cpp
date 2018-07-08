#Makefile
#Author: Sivert Andresen Cubedo

CC = g++
FLAGS = -Wall -Werror -std=c++11 -DLINUX -pthread

BINARIES =	main \
			MIP_deamon routing_deamon transport_deamon \
			file_receiver file_sender \
			file_compare file_generate

OBJECTS =	MIPFrame.o CrossForkExec.o CrossIPC.o \
			RawSock.o AddressTypes.o LinuxException.o \
			EventPoll.o DistanceVectorTable.o TimerWrapper.o \
			MIPTPFrame.o TransportInterface.o ClientHandler.o

FILES =	$(BINARIES) $(OBJECTS)

all: $(FILES)

#binary
main: src/main.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o main src/main.cpp $(OBJECTS)

MIP_deamon: src/MIP_deamon.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o MIP_deamon src/MIP_deamon.cpp $(OBJECTS)

routing_deamon: src/routing_deamon.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o routing_deamon src/routing_deamon.cpp $(OBJECTS)

transport_deamon: src/transport_deamon.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o transport_deamon src/transport_deamon.cpp $(OBJECTS)

file_receiver: src/file_receiver.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o file_receiver src/file_receiver.cpp $(OBJECTS)

file_sender: src/file_sender.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o file_sender src/file_sender.cpp $(OBJECTS)

file_compare: src/file_compare.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o file_compare src/file_compare.cpp $(OBJECTS)

file_generate: src/file_generate.cpp $(OBJECTS)
	$(CC) $(FLAGS) -o file_generate src/file_generate.cpp $(OBJECTS)

#objects
MIPFrame.o: src/MIPFrame.cpp include/MIPFrame.hpp
	$(CC) $(FLAGS) -c src/MIPFrame.cpp

EventPoll.o: src/EventPoll.cpp include/EventPoll.hpp
	$(CC) $(FLAGS) -c src/EventPoll.cpp

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

MIPTPFrame.o: src/MIPTPFrame.cpp include/MIPTPFrame.hpp
	$(CC) $(FLAGS) -c src/MIPTPFrame.cpp

TransportInterface.o: src/TransportInterface.cpp include/TransportInterface.hpp
	$(CC) $(FLAGS) -c src/TransportInterface.cpp

ClientHandler.o: src/ClientHandler.cpp include/ClientHandler.hpp
	$(CC) $(FLAGS) -c src/ClientHandler.cpp

clean:
	rm $(FILES)
