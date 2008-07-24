CXXFLAGS+= -Wall -ansi -g -O -I/usr/local/include
CXXFLAGS+= -D__freebsd__
LD_FLAGS= -L/usr/local/lib -pthread
LD_LIBS= -lomniORB4 -lomniDynamic4

OBJECTS= kademliaSK.o kademliaDynSK.o compare_any.o logging.o sha1.o random.o time.o \
         Broker.o ContactTable.o DataTable.o Id.o Node.o

all: kademlia test

kademlia: ${OBJECTS} main.o
	${CXX} ${LD_FLAGS} ${LD_LIBS} -o kademlia ${OBJECTS} main.o

test: ${OBJECTS} test.o
	${CXX} ${LD_FLAGS} ${LD_LIBS} -o test ${OBJECTS} test.o

sha1.o: sha1.h sha1.c
	${CC} -O3 -fexpensive-optimizations -funroll-loops -c sha1.c

sha1: sha1.o sha1main.o
	${CC} -o sha1 sha1.o sha1main.o

kademlia.hh kademliaSK.cc kademliaDynSK.cc: kademlia.idl
	omniidl -bcxx -Wba -Wbexample kademlia.idl
clean:
	-rm kademlia.hh kademliaSK.cc kademliaDynSK.cc ${OBJECTS}
	-rm kademlia main.o
	-rm test test.o
