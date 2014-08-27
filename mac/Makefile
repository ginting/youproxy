CC = g++
CFLAGS = -I../src 
LDFLAGS = $(CFLAGS)
OBJS = main.o $(patsubst %.cpp,%.o, $(wildcard ../src/*.cpp))

all: youagent
	echo 'Done.'

youagent: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS)


%.o:%.cpp
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf $(OBJS)

