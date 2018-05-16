#Makefile
#Author: Sivert Andresen Cubedo

CC = g++
FLAGS = -Wall -Werror -std=c++11 -DLINUX

FILES = main MIPFrame.o CrossForkExec.o

all: $(FILES)

main: src/main.cpp MIPFrame.o CrossForkExec.o
	$(CC) $(FLAGS) -o main src/main.cpp MIPFrame.o CrossForkExec.o

MIPFrame.o: src/MIPFrame.cpp include/MIPFrame.hpp
	$(CC) $(FLAGS) -c src/MIPFrame.cpp

CrossForkExec.o: src/CrossForkExec.cpp include/CrossForkExec.hpp
	$(CC) $(FLAGS) -c src/CrossForkExec.cpp

clean:
	rm $(FILES)