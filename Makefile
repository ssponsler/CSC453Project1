OBJS	= schedule.o
SOURCE	= schedule.c
HEADER	= schedule.h
OUT	= schedule
CC	 = gcc
FLAGS	 = -g -c -Wall
LFLAGS	 = 

all: $(OBJS)
	$(CC) -g $(OBJS) -o $(OUT) $(LFLAGS)

schedule.o: schedule.c
	$(CC) $(FLAGS) schedule.c 


clean:
	rm -f $(OBJS) $(OUT)