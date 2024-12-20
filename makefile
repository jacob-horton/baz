SRCS = main.cpp
BIN  = baz

CC   = g++
CFLAGS = -Wall -g

build: $(SRCS)
	@echo "Building..."
	$(CC) $(CFLAGS) -o $(BIN) $^

run: build
	@echo "Running..."
	./$(BIN)

clean:
	@echo "Cleaning up..."
	rm -f $(BIN)

.PHONY: build run clean
