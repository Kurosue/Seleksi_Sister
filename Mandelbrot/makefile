CC = gcc
GPP = g++
CFLAGS = -Wall -Wextra -fopenmp -O2 -I./lib -lOpenCL
CXXFLAGS = -lsfml-graphics -lsfml-window -lsfml-system -fopenmp -lOpenCL -I./lib
LDFLAGS = -lm

BIN_DIR = bin
SRC_DIR = src

.PHONY: build cli gui clear clean

cli: build
	@echo "Compile Mandelbrot CLI..."
	@$(CC) $(CFLAGS) $(SRC_DIR)/main.c $(SRC_DIR)/fractal.c -o $(BIN_DIR)/main_cli $(LDFLAGS)
	@echo "Running..."
	@$(BIN_DIR)/main_cli

gui: build
	@echo "Compile Mandelbrot GUI..."
	@$(GPP) $(SRC_DIR)/main.cpp $(SRC_DIR)/fractal.c -o $(BIN_DIR)/main_gui $(CXXFLAGS) $(LDFLAGS)
	@echo "Running..."
	@$(BIN_DIR)/main_gui

clear: 
	@echo "Clearing image files..."
	@rm -rf image/*

clean:
	@echo "Cleaning up build files..."
	@rm -rf $(BIN_DIR)/*