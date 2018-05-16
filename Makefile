#Makefile
#Author: Sivert Andresen Cubedo

CC = g++
FLAGS = -Wall -Werror -std=c++11 -DLINUX

FILES = main MIPFrame.o CrossForkExec.o CrossIPC.o

all: $(FILES)

main: src/main.cpp MIPFrame.o CrossForkExec.o CrossIPC.o
	$(CC) $(FLAGS) -o main src/main.cpp MIPFrame.o CrossForkExec.o CrossIPC.o

MIPFrame.o: src/MIPFrame.cpp include/MIPFrame.hpp
	$(CC) $(FLAGS) -c src/MIPFrame.cpp

CrossForkExec.o: src/CrossForkExec.cpp include/CrossForkExec.hpp
	$(CC) $(FLAGS) -c src/CrossForkExec.cpp

CrossIPC.o: src/CrossIPC.cpp include/CrossIPC.hpp
	$(CC) $(FLAGS) -c src/CrossIPC.cpp

clean:
	rm $(FILES)