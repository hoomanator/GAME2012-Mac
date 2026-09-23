GLFW_PREFIX := $(shell brew --prefix glfw)

CC  := clang
CXX := clang++
CPPFLAGS := -Iexternal/glad/include -I$(GLFW_PREFIX)/include
CFLAGS   := -std=c11 -Wall -Wextra
CXXFLAGS := -std=c++17 -Wall -Wextra
LDFLAGS  := -L$(GLFW_PREFIX)/lib -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

CXX_SRC := $(wildcard src/*.cpp)
C_SRC   := external/glad/src/gl.c
OBJ     := $(CXX_SRC:.cpp=.o) $(C_SRC:.c=.o)
BIN     := triangle

.PHONY: all run clean

all: $(BIN)

$(BIN): $(OBJ)
	$(CXX) $(OBJ) -o $(BIN) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN) $(OBJ)
