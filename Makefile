
GCC=gcc
GPP=g++

#FLAGS=-Wall -g -D DEBUG
FLAGS=-Wall -g 
GCCFLAGS=  -g
CPPFLAGS= -D DEBUG
LIBS= -lm

# User definitions must be here

#executable
EXEC = serveur.x

#.h
INCS = #reseau.h \

#.c
SOURCES = serveur.c  
#reseau.c \

OBJS = $(SOURCES:.c=.o)


# Building the world
all: $(EXEC) 

$(EXEC): $(INCS) $(OBJS) 
	$(GCC) $(GCCFLAGS) $(OBJS) $(LIBS) -o $(EXEC) 


.SUFFIXES:
.SUFFIXES: .c .cc .o

.cc.o:
	$(GPP) $(FLAGS) -c $<

.c.o:
	$(GCC) $(FLAGS) -c $<


# Clean up
clean:
	rm -f *~ .*~ \#*\# *.o *.tmp* Chaine*.txt *.png *.plt Reseau*.txt
	rm -f $(EXEC) 

# Dependencies
depend: 

	$(GCC) -M $(CPPFLAGS) $(SOURCES) > .depend
include .depend



