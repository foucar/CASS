CPPFLAGS = $(o) 

#CC = icc
#LD = g++
CPPFLAGS += -m64
CPPFLAGS += -I$(ROOTSYS)/include 
CFLAGS += $(CPPFLAGS)
LDFLAGS+= $(CFLAGS) -lm 
LDFLAGS+= -L$(ROOTSYS)/lib -lCore -lCint -lRIO -lRint -lNet -lHist -lMathCore -lMatrix
LDFLAGS+= -L/usr/lib64 -lrt -ldl
target = AGATRemote

SOURCES.cpp =	./MyClient/MyClientDict.cpp \
                ./settings.cpp \
                ./MyAnalyzer/MyAnalyzer.cpp \
                ./MyBuffer/MyBuffer.cpp \
                ./MyClient/MyClient.cpp \
                ./MyDetektorHitSorter/helperfunctionsforDetSorter.cpp \
                ./MyDetektorHitSorter/MyDetektorHitSorter.cpp \
                ./MyDetektorHitSorter/MyDetektorHitSorterQuad.cpp \
                ./MyDetektorHitSorter/MyDetektorHitSorterSimple.cpp \
                ./MyEvent/MyOriginalEvent/MyOriginalEvent.cpp \
                ./MyEvent/MyOriginalEvent/MyOriginalChannel/MyOriginalChannel.cpp \
                ./MyEvent/MyOriginalEvent/MyPuls/MyPuls.cpp \
                ./MyEvent/MySignalAnalyzedEvent/MySignalAnalyzedEvent.cpp \
                ./MyEvent/MySignalAnalyzedEvent/MyPeak/MyPeak.cpp \
                ./MyEvent/MySignalAnalyzedEvent/MySignalAnalyzedChannel/MyChannelSection.cpp \
                ./MyEvent/MySignalAnalyzedEvent/MySignalAnalyzedChannel/MySignalAnalyzedChannel.cpp \
                ./MyEvent/MySignalAnalyzedEvent/MyVoltage/MyVoltage.cpp \
                ./MyEvent/MySortedEvent/MySortedEvent.cpp \
                ./MyEvent/MySortedEvent/MyDetektor/MyDetektor.cpp \
                ./MyEvent/MySortedEvent/MyDetektor/MyDetektorHit.cpp \
                ./MyEvent/MySortedEvent/MyDetektor/MyLayerProperty.cpp \
                ./MyEvent/MySortedEvent/MyDetektor/MyTimeRange.cpp \
                ./MyEvent/MySortedEvent/MyPunkt/MyPunkt.cpp \
                ./MyMomentaCalculator/MyMomentaCalculator.cpp \
                ./MyParticle/MyParticle.cpp \
                ./MyParticle/MyParticleContainer.cpp \
                ./MyParticle/MyParticleHit.cpp \
                ./MyParticle/MyParticleInfo.cpp \
                ./MyParticle/MySpectrometer/MySpectrometer.cpp \
                ./MyParticle/MySpectrometer/MySpectrometerRegion.cpp \
                ./MyPulsHistoFiller/MyPulsHistoFiller.cpp \
                ./MyRootManager/MyHistos.cpp \
                ./MyRootManager/MyRootManager.cpp \
                ./MySettings/MySettings.cpp \
                ./MySignalAnalyzer/helperfunctionsforSTDC.cpp \
                ./MySignalAnalyzer/MySignalAnalyzer.cpp \
                ./MySignalAnalyzer/MySoftTDCCFD.cpp \
                ./MySignalAnalyzer/MySoftTDCCoM.cpp \
                ./Projekt/main.cpp

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
