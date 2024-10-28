CC = gcc
CFLAGS = -Wall -Wextra -O0 -g3
INCLUDES = -I.
EXAMPLES = fib timer sum
BUILD_DIR = build

all: $(EXAMPLES)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

%: examples/%.c coroutine.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(BUILD_DIR)/$@ $<

run: $(EXAMPLES)
	for example in $(EXAMPLES); do $(BUILD_DIR)/$$example; done

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
