#Makefile
#Author: Sivert Andresen Cubedo

CC = g++
FLAGS = -Wall -Werror -std=c++11 -DLINUX

FILES =	main MIP_deamon routing_deamon MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o AddressTypes.o LinuxException.o EventPoll.o

all: $(FILES)

#binary
main: src/main.cpp MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o AddressTypes.o LinuxException.o EventPoll.o
	$(CC) $(FLAGS) -o main src/main.cpp MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o AddressTypes.o LinuxException.o EventPoll.o

MIP_deamon: src/MIP_deamon.cpp MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o AddressTypes.o LinuxException.o EventPoll.o
	$(CC) $(FLAGS) -o MIP_deamon src/MIP_deamon.cpp MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o AddressTypes.o LinuxException.o EventPoll.o

routing_deamon: src/routing_deamon.cpp MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o AddressTypes.o LinuxException.o EventPoll.o
	$(CC) $(FLAGS) -o routing_deamon src/routing_deamon.cpp MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o AddressTypes.o LinuxException.o EventPoll.o

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

clean:
	rm $(FILES)


