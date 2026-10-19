CC      ?= gcc
BASE    = -std=c11 -ffp-contract=off -pthread -Wall -Wextra
# V0: sin vectorizar (gcc). En macOS/clang use: -O2 -fno-vectorize -fno-slp-vectorize
V0FLAGS = $(BASE) -O2 -fno-tree-vectorize
# V1-V3: vectorizacion automatica (gcc en Linux/WSL). En macOS: -O3 -mcpu=native
OPTFLAGS = $(BASE) -O3 -march=native

all: v0
v0: v0.c pgm.h
	$(CC) $(V0FLAGS) -o $@ $<

# Cuando tenga V1: make v1 y revise que diga "loop vectorized"
v1: v1.c pgm.h
	$(CC) $(OPTFLAGS) -fopt-info-vec -o $@ $<

clean:
	rm -f v0 v1 v2 v3
.PHONY: all clean
