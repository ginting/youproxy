CC= g++
CFLAGS= -O2 -s -Wall  -D_UNICODE -D_UNICODE_
LFLAGS = -I. -I$(SRC_DIR) -lpthread

SRC_DIR=./../src

OBJS= main.o $(filter-out  $(SRC_DIR)/systemutil.%, $(patsubst %.cpp,%.o, $(wildcard $(SRC_DIR)/*.cpp)))

all: youagent

youagent: $(OBJS) 
	$(CC) $(CFLAGS) -o $@ $(OBJS)  $(LFLAGS)

$(OBJS):%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<  $(LFLAGS)


.PHONY: clean
clean:
	-rm -rf ./youagent
	-rm -rf $(OBJS)

