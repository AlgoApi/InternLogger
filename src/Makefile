C = gcc
CFLAGS = -std=c++17 -Wall -Werror -Wextra -fPIC -g -x c++
SAN_FLAGS = -lstdc++ -pthread -Wl,-rpath=\$$ORIGIN/../lib

BUILD_DIR = ../build
OBJ_DIR = $(BUILD_DIR)/obj
LIB_DIR = $(BUILD_DIR)/lib
BIN_DIR = $(BUILD_DIR)/bin

SRC = logger.cpp mainframe.cpp
HEADERS = logger.h mainframe.h
EXECUTABLE = main

TEST_ARGS = ../materials/test_output.txt info < ../materials/test_input.txt

.PHONY: all all_lint all_test clean_all rebuild directories check logger_so sanitize valgrind only_main only_test_main

all: directories main

all_test: directories test_main

all_lint: check valgrind sanitize

directories:
	mkdir -p $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)

# ----------  .o  ----------
logger_o:
	$(C) $(CFLAGS) -c logger.cpp -o $(OBJ_DIR)/logger.o

mainframe_o:
	$(C) $(CFLAGS) -pthread -c mainframe.cpp -o $(OBJ_DIR)/mainframe.o

test_mainframe_o:
	$(C) $(CFLAGS) -pthread -c mainframe.cpp -DTEST_H -o $(OBJ_DIR)/mainframe.o

# ---------- .so  ----------
logger_so: logger_o
	$(C) -shared $(OBJ_DIR)/logger.o -lstdc++ -o $(LIB_DIR)/liblogger.so

# ---------- bin ----------
main: mainframe_o logger_so
	$(C) $(OBJ_DIR)/mainframe.o -L$(LIB_DIR) -llogger $(SAN_FLAGS) -o $(BIN_DIR)/$(EXECUTABLE)

only_main: mainframe_o
	$(C) $(OBJ_DIR)/mainframe.o -L$(LIB_DIR) -llogger $(SAN_FLAGS) -o $(BIN_DIR)/$(EXECUTABLE)

test_main: test_mainframe_o logger_so
	$(C) $(OBJ_DIR)/mainframe.o -L$(LIB_DIR) -llogger $(SAN_FLAGS) -o $(BIN_DIR)/$(EXECUTABLE)_test

only_test_main: test_mainframe_o logger_so
	$(C) $(OBJ_DIR)/mainframe.o -L$(LIB_DIR) -llogger $(SAN_FLAGS) -o $(BIN_DIR)/$(EXECUTABLE)_test

# ---------- Sanitizes ----------
sanitize: sanitize_address sanitize_leak sanitize_undefined sanitize_unreachable

sanitize_address: clean_all directories
	$(C) $(CFLAGS) -fsanitize=address -c logger.cpp -o $(OBJ_DIR)/logger.o
	$(C) $(CFLAGS) -fsanitize=address -c mainframe.cpp -o $(OBJ_DIR)/mainframe.o
	$(C) -shared $(OBJ_DIR)/logger.o -fsanitize=address -o $(LIB_DIR)/liblogger.so
	$(C) $(OBJ_DIR)/mainframe.o -L$(LIB_DIR) -llogger -fsanitize=address $(SAN_FLAGS) -o $(BIN_DIR)/main_address
	- ./$(BIN_DIR)/main_address $(TEST_ARGS)

sanitize_leak: clean_all directories
	$(C) $(CFLAGS) -fsanitize=leak -c logger.cpp -o $(OBJ_DIR)/logger.o
	$(C) $(CFLAGS) -fsanitize=leak -c mainframe.cpp -o $(OBJ_DIR)/mainframe.o
	$(C) -shared $(OBJ_DIR)/logger.o -fsanitize=leak -o $(LIB_DIR)/liblogger.so
	$(C) $(OBJ_DIR)/mainframe.o -L$(LIB_DIR) -llogger -fsanitize=leak $(SAN_FLAGS) -o $(BIN_DIR)/main_leak
	- ./$(BIN_DIR)/main_leak $(TEST_ARGS)

sanitize_undefined: clean_all directories
	$(C) $(CFLAGS) -fsanitize=undefined -c logger.cpp -o $(OBJ_DIR)/logger.o
	$(C) $(CFLAGS) -fsanitize=undefined -c mainframe.cpp -o $(OBJ_DIR)/mainframe.o
	$(C) -shared $(OBJ_DIR)/logger.o -fsanitize=undefined -o $(LIB_DIR)/liblogger.so
	$(C) $(OBJ_DIR)/mainframe.o -L$(LIB_DIR) -llogger -fsanitize=undefined $(SAN_FLAGS) -o $(BIN_DIR)/main_undefined
	- ./$(BIN_DIR)/main_undefined $(TEST_ARGS)

sanitize_unreachable: clean_all directories
	$(C) $(CFLAGS) -fsanitize=unreachable -c logger.cpp -o $(OBJ_DIR)/logger.o
	$(C) $(CFLAGS) -fsanitize=unreachable -c mainframe.cpp -o $(OBJ_DIR)/mainframe.o
	$(C) -shared $(OBJ_DIR)/logger.o -fsanitize=unreachable -o $(LIB_DIR)/liblogger.so
	$(C) $(OBJ_DIR)/mainframe.o -L$(LIB_DIR) -llogger -fsanitize=unreachable $(SAN_FLAGS) -o $(BIN_DIR)/main_unreachable
	- ./$(BIN_DIR)/main_unreachable $(TEST_ARGS)

# ---------- Lint ----------
format:
	clang-format -n $(SRC) $(HEADERS)

cppcheck:
	cppcheck --enable=all --suppress=missingIncludeSystem --check-level=exhaustive ../src/

check: format cppcheck

# ---------- Valgrind ----------
valgrind: clean_all all
	- valgrind -s --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file="valgrind.log" ../build/bin/$(EXECUTABLE) $(TEST_ARGS)

clean:
	rm -rf $(OBJ_DIR)

rebuild: clean_all all

clean_all:
	rm -rf $(OBJ_DIR) $(LIB_DIR) $(BIN_DIR)
