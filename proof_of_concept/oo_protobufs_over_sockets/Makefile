STD = -std=c++11

PDIR = protobuf_classes

POBJ1 = $(PDIR)/base_signature.pb.o
POBJ2 = $(PDIR)/request.pb.o
POBJ3 = $(PDIR)/start_backup_request.pb.o
POBJ4 = $(PDIR)/stop_backup_request.pb.o

POBJS = $(POBJ1) $(POBJ2) $(POBJ3) $(POBJ4)

protosocket.o: protosocket.cc protosocket.h
	g++ -I . $(STD) -c -o protosocket.o protosocket.cc 2>&1
server1: server1.cc protosocket.o $(POBJS)
	g++ -I . -L protobuf_classes $(STD) -o server1 $^ -l protobuf -pthread 2>&1
client1: client1.cc protosocket.o $(POBJS)
	g++ -I . -L protobuf_classes $(STD) -o client1 $^ -l protobuf -pthread 2>&1
