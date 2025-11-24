# Makefile para compilar no Windows com o compilador MSVC
CXX = cl
CXXFLAGS = /EHsc /std:c++17                # Habilita exceções e C++17

OUTDIR = exec
TARGET = $(OUTDIR)\microcompilador.exe
OBJDIR = $(OUTDIR)
SRC = main\main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	if not exist $(OUTDIR) mkdir $(OUTDIR)
	$(CXX) $(CXXFLAGS) /Fe$(TARGET) /Fo$(OBJDIR)\ $(SRC)

clean:
	del $(TARGET) $(OBJDIR)\*.obj 2>nul
