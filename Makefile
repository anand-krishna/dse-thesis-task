CXX = g++

CXXFLAGS = -Wall

all: server client

client: client.cpp
	$(CXX) $(CXXFLAGS) -o client client.cpp -pthread -lrt

server: server.cpp w_opt_rwlock hash_table
	$(CXX) $(CXXFLAGS) -o server server.cpp hash_table.o w_opt_rwlock.o -pthread -lrt

w_opt_rwlock:
	$(CXX) $(CXXFLAGS) -c -o w_opt_rwlock.o w_opt_rwlock.cpp -pthread -lrt

hash_table:
	$(CXX) $(CXXFLAGS) -c -o hash_table.o -Wall hash_table.cpp

clean:
	rm hash_table.o w_opt_rwlock.o client server