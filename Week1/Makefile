GLFW_PREFIX := $(shell brew --prefix glfw)

CC := clang
CFLAGS := -std=c11 -Wall -Wextra -Iexternal/glad/include -I$(GLFW_PREFIX)/include
LDFLAGS := -L$(GLFW_PREFIX)/lib -lglfw -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo

SRC := src/main.c external/glad/src/gl.c
BIN := triangle

.PHONY: all run clean

all: $(BIN)

$(BIN): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(BIN) $(LDFLAGS)

run: $(BIN)
	./$(BIN)

clean:
	rm -f $(BIN)
