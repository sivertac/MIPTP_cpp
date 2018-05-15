#Makefile

CC = g++
FLAGS = -Wall -Werror -std=c++11

FILES = main MIPFrame.o

main: main.cpp MIPFrame.o
	$(CC) $(FLAGS) -o main main.cpp MIPFrame.o

MIPFrame.o: MIPFrame.cpp MIPFrame.hpp
	$(CC) $(FLAGS) -c MIPFrame.cpp

clean:
	rm $(FILES)