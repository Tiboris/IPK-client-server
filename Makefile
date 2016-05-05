CXX=g++
CXXFLAGS=-O2 -g -Wall -Wextra -pedantic -std=c++11 -pthread	
LDFLAGS=-Wl,-rpath=/usr/local/lib/gcc49/

default: client server

server: server.cpp

client: client.cpp

clean: 
	rm -f client server
