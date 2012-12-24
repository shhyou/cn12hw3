CXX       = g++
CXXFLAGS  = -std=c++11 -Wall -Wshadow -Wextra
DEBUG     =
HEADERS   = icmp.h tcp.h log.h
OBJS      = icmp.o tcp.o log.o
CPPS      = icmp.cpp tcp.cpp log.cpp snd.cpp rcv.cpp
TARGET    = snd rcv
#TARGET    =

.PHONY: all clean
.SUFFIXES:

all: $(TARGET) $(OBJS)

clean:
	rm -f $(TARGET) $(OBJS)

%.o: %.cpp $(HEADERS)
	$(CXX) -c -o $@ $< $(CXXFLAGS) $(DEBUG)

snd: snd.cpp $(OBJS) $(HEADERS)
	$(CXX) -o $@ $< $(OBJS) $(CXXFLAGS) $(DEBUG)

rcv: rcv.cpp $(OBJS) $(HEADERS)
	$(CXX) -o $@ $< $(OBJS) $(CXXFLAGS) $(DEBUG)

%: %.cpp $(OBJS) $(HEADERS)
	$(CXX) -o $@ $< $(OBJS) $(CXXFLAGS) $(DEBUG)

