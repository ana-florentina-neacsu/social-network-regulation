CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -g -D_POSIX_C_SOURCE=199309L
LDFLAGS = -lcheck -lm -lpthread -lrt -lsubunit
VALGRIND = valgrind
VALGRIND_FLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose

SRCS = src/max_heap.c src/post.c src/moderation.c
OBJS = $(SRCS:.c=.o)
DEPS = include/max_heap.h include/post.h include/moderation.h

MAIN_SRC = main.c
MAIN_TARGET = test_heap

TEST_DIR = tests
TEST_SRCS = $(TEST_DIR)/test_max_heap.c $(TEST_DIR)/test_perf_max_heap.c
TEST_TARGETS = $(TEST_SRCS:.c=)
TEST_OBJS = $(TEST_SRCS:.c=.o)

all: $(MAIN_TARGET)

$(MAIN_TARGET): $(MAIN_SRC) $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(MAIN_SRC) $(OBJS)

$(TEST_DIR)/test_max_heap: $(TEST_DIR)/test_max_heap.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_DIR)/test_perf_max_heap: $(TEST_DIR)/test_perf_max_heap.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

build-tests: $(TEST_TARGETS)

run-test-suite: $(TEST_TARGETS)
	@echo "=========================================="
	@echo "Running Max Heap Functional Tests"
	@echo "=========================================="
	@$(TEST_DIR)/test_max_heap
	@echo ""
	@echo "=========================================="
	@echo "Running Max Heap Performance Tests"
	@echo "=========================================="
	@$(TEST_DIR)/test_perf_max_heap
	@echo ""
	@echo "=========================================="
	@echo "All tests completed!"
	@echo "=========================================="

run-test-heap: $(TEST_DIR)/test_max_heap
	./$(TEST_DIR)/test_max_heap

run-test-perf: $(TEST_DIR)/test_perf_max_heap
	./$(TEST_DIR)/test_perf_max_heap

valgrind-tests: $(TEST_TARGETS)
	@echo "Running tests with Valgrind..."
	$(VALGRIND) $(VALGRIND_FLAGS) ./$(TEST_DIR)/test_max_heap
	$(VALGRIND) $(VALGRIND_FLAGS) ./$(TEST_DIR)/test_perf_max_heap

clean:
	rm -f $(OBJS) $(MAIN_TARGET) $(TEST_OBJS) $(TEST_TARGETS)
	rm -f src/*.o $(TEST_DIR)/*.o

run: $(MAIN_TARGET)
	./$(MAIN_TARGET)

valgrind: $(MAIN_TARGET)
	$(VALGRIND) $(VALGRIND_FLAGS) ./$(MAIN_TARGET)

memcheck: $(MAIN_TARGET)
	$(VALGRIND) --leak-check=full --show-leak-kinds=all --track-origins=yes \
	--error-exitcode=1 ./$(MAIN_TARGET)

.PHONY: all clean run valgrind memcheck build-tests run-test-suite run-test-heap run-test-perf valgrind-tests