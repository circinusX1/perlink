
CXX=${CROSS_COMPILE}g++

src = $(wildcard *.cpp)
obj = $(src:.cpp=.o)

LDFLAGS = -lpthread  -lsqlite3

CXXFLAGS += -std=gnu++11 -Wall -W -D_REENTRANT -fPIC -static-libstdc++ 

perlink: $(obj)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) perlink

