#===============================================================================
# Top-level Makefile wrapper for si-sycl (uses CMake)
#===============================================================================

BUILD_DIR = build

.PHONY: all clean

all:
	mkdir -p $(BUILD_DIR) && cd $(BUILD_DIR) && cmake .. && $(MAKE)

clean:
	rm -rf $(BUILD_DIR)

run: all
	$(LAUNCHER) ./$(BUILD_DIR)/main
