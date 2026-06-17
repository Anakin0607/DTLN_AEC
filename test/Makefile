CXX := g++
CXXFLAGS := -std=c++11 -Wall -O2 -I./include

LDFLAGS := -L./lib -Wl,-rpath,'$$ORIGIN/lib'
LDLIBS := -lDTLN_AEC -ltensorflowlite_c

all: AecSampleTest

AecSampleTest: AecSample.cpp wav_reader.cc
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS) $(LDLIBS)

.PHONY: clean
clean:
	rm -f $(OBJS) $(TARGET)