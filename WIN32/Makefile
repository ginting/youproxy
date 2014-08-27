CC= g++
CFLAGS= -O2 -s -Wall -D__WIN32__  -mwindows -D_UNICODE -D_UNICODE_
LDFLAGS= -I../src -I./posixthread/include -L./posixthread/lib/x86 -L./posixthread/dll/x86 -L.  -lpthreadGC2 -lkernel32 -luser32 -lgdi32 -lcomdlg32 -lwinspool -lwinmm -lshell32 -lcomctl32 -lole32 -loleaut32 -luuid -lrpcrt4 -ladvapi32 -lwsock32 -lodbc32 -lWininet
OBJS= $(patsubst %.cpp,%.o, $(wildcard ../src/*.cpp))


all: YouProxy

YouProxy: $(OBJS) res.o main.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS):%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<  $(LDFLAGS)
main.o: main.cpp main.h
	$(CC) $(CFLAGS) -c -o $@ $<  $(LDFLAGS)


res.o: main.rc ../res/Logo.ico
	windres -imain.rc -o $@ -I../res

.PHONY: clean
clean:
	rm -rf *.exe
	rm -rf $(OBJS)
	rm -rf res.o
