SOURCES = $(shell find ast kaleidoscope lexer logger parser -name '*.cpp')
HEADERS = $(shell find ast kaleidoscope lexer logger parser -name '*.h')
OBJ = ${SOURCES:.cpp=.o}

TESTS = $(wildcard tests/*.mjava)
OUTPUTS := $(foreach test,$(TESTS),outputs/$(test:tests/%.mjava=%).out)
DIFFS := $(foreach test,$(TESTS),diffs_default/$(test:tests/%.mjava=%).diff)

CC = clang++-18 -stdlib=libc++ -std=c++14
CFLAGS = -g -I llvm/include -I llvm/build/include -I ./
LLVMCFLAGS = `llvm-config --cxxflags`
LLVMFLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs all`

.PHONY: main

all: main $(OUTPUTS)

main: main.cpp ${OBJ}
	${CC} ${CFLAGS} ${LLVMFLAGS} ${OBJ} $< -o $@

clean:
	rm -r ${OBJ}

%.o: %.cpp ${HEADERS}
	${CC} ${CFLAGS} ${LLVMCFLAGS} -c $< -o $@

define test_rules
outputs/$(1:tests/%.mjava=%).ll: $(1) main
	@echo "cat $(1) | ./main > $$@"
	-@cat $(1) | ./main > $$@
outputs/$(1:tests/%.mjava=%).s: outputs/$(1:tests/%.mjava=%).ll
	@echo "clang -S -c $$< -o $$@"
	-@clang -S -c $$< -o $$@
outputs/$(1:tests/%.mjava=%).o: outputs/$(1:tests/%.mjava=%).s
	@echo "clang -S -c $$< -o $$@"
	-@clang -c $$< -o $$@
outputs/$(1:tests/%.mjava=%).exe: outputs/$(1:tests/%.mjava=%).o tests/$(1:tests/%.mjava=%).c
	@echo "clang $$^ -o $$@"
	-@clang $$^ -o $$@
outputs/$(1:tests/%.mjava=%).out: outputs/$(1:tests/%.mjava=%).exe
	@echo "$$< > $$@"
	-@$$< > $$@
endef
$(foreach test,$(TESTS),$(eval $(call test_rules,$(test))))