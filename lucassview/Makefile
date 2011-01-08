CPPFLAGS = $(o) 

#CC = icc
LD = g++
CPPFLAGS += -g -g -pipe
CPPFLAGS += -I$(ROOTSYS)/include
CFLAGS += $(CPPFLAGS)
LDFLAGS+= $(CFLAGS) -lm 
LDFLAGS+= -L$(ROOTSYS)/lib -lCore -lCint -lRIO -lRint -lNet -lHist -lMathCore -lMatrix
LDFLAGS+= -L/usr/lib64 -lrt -ldl -lstdc++
LDFLAGS+= -lgsoap++ -lgsoap
target = ../bin/lucassview

SOURCES.cpp = soapCASSsoapProxy.cpp \
	soapC.cpp \
	main.cpp \
  tcpclient.cpp \
  histo_updater_dict.cpp \
  histo_updater.cpp \
  id_list.cpp

OBJECTS = $(SOURCES.cpp:%.cpp=%.o)

all:	$(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $(target) 

soapCASSsoapProxy.cpp: ../cass/soapserver.h
	soapcpp2 -C -i ../cass/soapserver.h;

soapC.cpp: soapCASSsoapProxy.cpp

histo_updater_dict.cpp: histo_updater.h
	$(ROOTSYS)/bin/rootcint -f histo_updater_dict.cpp -c histo_updater.h histo_updater_linkdef.h;

clean:
	@echo "cleaning project"
	@rm -f `find . -name "*.o"`
	@rm -f `find . -name "*.xml"`
	@rm -f `find . -name "*soap*"`
	@rm -f `find . -name "ns.xsd"`
	@rm -f `find . -name "*_dict*"`
