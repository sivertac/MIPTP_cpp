#Makefile
#Author: Sivert Andresen Cubedo

CC = g++
FLAGS = -Wall -Werror -std=c++11 -DLINUX

FILES =	main MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o EthernetFrame.o AddressTypes.o LinuxException.o

all: $(FILES)

#binary
main: src/main.cpp MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o EthernetFrame.o AddressTypes.o LinuxException.o
	$(CC) $(FLAGS) -o main src/main.cpp MIPFrame.o CrossForkExec.o CrossIPC.o RawSock.o EthernetFrame.o AddressTypes.o LinuxException.o

#obljects
MIPFrame.o: src/MIPFrame.cpp include/MIPFrame.hpp
	$(CC) $(FLAGS) -c src/MIPFrame.cpp

CrossForkExec.o: src/CrossForkExec.cpp include/CrossForkExec.hpp
	$(CC) $(FLAGS) -c src/CrossForkExec.cpp

CrossIPC.o: src/CrossIPC.cpp include/CrossIPC.hpp
	$(CC) $(FLAGS) -c src/CrossIPC.cpp

RawSock.o: src/RawSock.cpp include/RawSock.hpp
	$(CC) $(FLAGS) -c src/RawSock.cpp

EthernetFrame.o: src/EthernetFrame.cpp include/EthernetFrame.hpp
	$(CC) $(FLAGS) -c src/EthernetFrame.cpp

AddressTypes.o: src/AddressTypes.cpp include/AddressTypes.hpp
	$(CC) $(FLAGS) -c src/AddressTypes.cpp

LinuxException.o: src/LinuxException.cpp include/LinuxException.hpp
	$(CC) $(FLAGS) -c src/LinuxException.cpp

clean:
	rm $(FILES)


