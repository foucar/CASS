CPPFLAGS = $(o) 

#CC = icc
#LD = g++
CPPFLAGS += -m64
CPPFLAGS += -I$(ROOTSYS)/include 
CFLAGS += $(CPPFLAGS)
LDFLAGS+= $(CFLAGS) -lm 
LDFLAGS+= -L$(ROOTSYS)/lib -lCore -lCint -lRIO -lRint -lNet -lHist -lMathCore -lMatrix
LDFLAGS+= -L/usr/lib64 -lrt -ldl
target = lucassview

SOURCES.cpp =	main.cpp

OBJECTS = $(SOURCES.cpp:%.cpp=%.o)

all:	$(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(target) 

MyClient/MyClientDict.cpp: ./MyClient/MyClient.h
	cd ./MyClient/; \
	$(ROOTSYS)/bin/rootcint -f MyClientDict.cpp -c MyClient.h; \
	cd ..;

clean:
	@echo "cleaning project"
	@rm -f `find . -name "*.o"`
	@rm -f `find . -name "*Dict.*"`
