# Makefile para compilar no Windows com o compilador MSVC
CXX = cl							# Compilador C++
CXXFLAGS = /EHsc					# Habilita tratamento de exceções em C++

TARGET = microcompilador.exe		#Nome do executável
SRC = main/main.cpp					#Arquivos fonte

all: $(TARGET)		

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC)		

clean:
	del $(TARGET) *.obj 			
