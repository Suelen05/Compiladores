# Simple build using MSVC cl
CXX = cl
CXXFLAGS = /EHsc

TARGET = microc.exe
SRC = main/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC)

clean:
	del $(TARGET) *.obj
