SOURCES = $(shell find ast kaleidoscope lexer logger parser -name '*.cpp')
HEADERS = $(shell find ast kaleidoscope lexer logger parser -name '*.h')
OBJ = ${SOURCES:.cpp=.o}

TESTS = $(wildcard tests/*.mjava)
OUTPUTS := $(foreach test,$(TESTS),outputs/$(test:tests/%.mjava=%).out)
DIFFS := $(foreach test,$(TESTS),diffs_default/$(test:tests/%.mjava=%).diff)

EXAMPLES = $(wildcard examples/*.cpp)
EXAMPLE_OUTPUTS = $(foreach example,$(EXAMPLES),examples_outputs/$(example:examples/%.cpp=%).codegen.out)

CC = clang++-18 -stdlib=libc++ -std=c++14
CFLAGS = -g -I llvm/include -I llvm/build/include -I ./
LLVMCFLAGS = `llvm-config --cxxflags`
LLVMFLAGS = `llvm-config --cxxflags --ldflags --system-libs --libs all`

all: main $(EXAMPLE_OUTPUTS) $(OUTPUTS) 

main: main.cpp ${OBJ}
	${CC} ${CFLAGS} ${LLVMFLAGS} ${OBJ} $< -o $@

clean:
	rm -r ${OBJ} outputs/* examples_outputs/*

%.o: %.cpp ${HEADERS}
	${CC} ${CFLAGS} ${LLVMCFLAGS} -c $< -o $@

define example_rules
examples_outputs/$(1:examples/%.cpp=%).exe: $(1) ${OBJ}
	${CC} ${CFLAGS} ${LLVMFLAGS} ${OBJ} $$< -o $$@
examples_outputs/$(1:examples/%.cpp=%).codegen.ll: examples_outputs/$(1:examples/%.cpp=%).exe
	@echo "$$< > $$@"
	-@$$< > $$@
examples_outputs/$(1:examples/%.cpp=%).codegen.s: examples_outputs/$(1:examples/%.cpp=%).codegen.ll
	@echo "clang -S -c $$< -o $$@"
	-@clang -S -c $$< -o $$@
examples_outputs/$(1:examples/%.cpp=%).codegen.exe: examples_outputs/$(1:examples/%.cpp=%).codegen.s
	@echo "clang $$< -o $$@"
	-@clang $$< -o $$@
examples_outputs/$(1:examples/%.cpp=%).codegen.out: examples_outputs/$(1:examples/%.cpp=%).codegen.exe
	@echo "$$< > $$@"
	-@$$< > $$@
endef
$(foreach example,$(EXAMPLES),$(eval $(call example_rules,$(example))))

define test_rules
outputs/$(1:tests/%.mjava=%).ll: $(1) main
	@echo "cat $(1) | ./main > $$@"
	-@cat $(1) | ./main > $$@
outputs/$(1:tests/%.mjava=%).s: outputs/$(1:tests/%.mjava=%).ll
	@echo "clang -S -c $$< -o $$@"
	-@clang -S -c $$< -o $$@
outputs/$(1:tests/%.mjava=%).o: outputs/$(1:tests/%.mjava=%).s
	@echo "clang -c $$< -o $$@"
	-@clang -c $$< -o $$@
outputs/$(1:tests/%.mjava=%).exe: outputs/$(1:tests/%.mjava=%).o tests/$(1:tests/%.mjava=%).c
	@echo "clang $$^ -o $$@"
	-@clang $$^ -o $$@
outputs/$(1:tests/%.mjava=%).out: outputs/$(1:tests/%.mjava=%).exe
	@echo "$$< > $$@"
	-@$$< > $$@
endef
$(foreach test,$(TESTS),$(eval $(call test_rules,$(test))))