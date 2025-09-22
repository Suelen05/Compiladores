# Compilador
CXX = cl
# Flags: /EHsc habilita exceções seguras em C++
CXXFLAGS = /EHsc

# Nome do executável
TARGET = lexer.exe
SRC = lexer.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC)

clean:
	del $(TARGET) *.obj
