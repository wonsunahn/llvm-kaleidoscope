SOURCES = $(shell find ast kaleidoscope lexer logger parser -name '*.cpp')
HEADERS = $(shell find ast kaleidoscope lexer logger parser -name '*.h')
OBJ = ${SOURCES:.cpp=.o}

CC = clang++-18 -stdlib=libc++ -std=c++14
CFLAGS = -g -O3 -I llvm/include -I llvm/build/include -I ./
LLVMCFLAGS = `llvm-config --cxxflags`
LLVMFLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs all`

.PHONY: main

main: main.cpp ${OBJ}
	${CC} ${CFLAGS} ${LLVMFLAGS} ${OBJ} $< -o $@

clean:
	rm -r ${OBJ}

%.o: %.cpp ${HEADERS}
	${CC} ${CFLAGS} ${LLVMCFLAGS} -c $< -o $@
