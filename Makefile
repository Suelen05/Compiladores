# Makefile para compilar no Windows com o compilador MSVC
CXX = cl
CXXFLAGS = /EHsc 							# Habilita tratamento de exceções em C++

OUTDIR = exec 								# Diretório de saída para o executável
TARGET = $(OUTDIR)\microcompilador.exe		# Diretório do executável
OBJDIR = $(OUTDIR)
SRC = main\main.cpp 						# Arquivo principal

all: $(TARGET)								# Compila o projeto

$(TARGET): $(SRC)							# Cria o diretório de saída se não existir
	if not exist $(OUTDIR) mkdir $(OUTDIR)
	$(CXX) $(CXXFLAGS) /Fe$(TARGET) /Fo$(OBJDIR)\ $(SRC)

clean:
	del $(TARGET) $(OBJDIR)\*.obj 2>nul
