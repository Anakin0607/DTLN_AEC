CXX := g++
CXXFLAGS := -std=c++11 -Wall -O2 -I./include

LDFLAGS := -L./lib -Wl,-rpath,'$$ORIGIN/lib'
LDLIBS := -lDTLN_AEC -ltensorflowlite_c

TARGET := AecSample

SRCS := AecSample.cpp

OBJS := $(SRCS:.cpp=.o)

all: AecSample

AecSample: AecSample.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)