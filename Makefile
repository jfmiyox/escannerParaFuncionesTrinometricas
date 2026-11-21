CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2
TARGET   := evaluador
TEST_BIN := test_unitarios
BUILDDIR := build

CORE_SRCS := src/engine/char_source.cpp \
             src/engine/afd_scanner.cpp \
             src/languages/trig/trig_language.cpp \
             src/parser/trig_parser.cpp \
             src/application/scanner_service.cpp \
             src/application/evaluator_service.cpp \
             src/infrastructure/file_loader.cpp \
             src/infrastructure/token_printer.cpp \
             src/infrastructure/error_reporter.cpp

CLI_SRCS  := src/cli/main.cpp $(CORE_SRCS)
TEST_SRCS := tests/test_unitarios.cpp $(CORE_SRCS)

CLI_OBJS  := $(CLI_SRCS:%.cpp=$(BUILDDIR)/%.o)
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILDDIR)/%.o)

all: $(TARGET)

$(TARGET): $(CLI_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

static: $(CLI_SRCS)
	$(CXX) $(CXXFLAGS) -static -o $(TARGET) $^

$(TEST_BIN): $(TEST_OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

$(BUILDDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

test: $(TEST_BIN) $(TARGET)
	./$(TEST_BIN)
	@echo ""
	@echo "=== Ejecucion lote de expresiones validas ==="
	./$(TARGET) --lote tests/expresiones_validas.txt -v x=1.0 -v y=0.5 -v z=2.0

clean:
	rm -rf $(BUILDDIR) $(TARGET) $(TEST_BIN)

rebuild: clean all

.PHONY: all static test clean rebuild
