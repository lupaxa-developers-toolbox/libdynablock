# Static library build. The c skill syntax-checks and runs cppcheck.
# It does not compile or link.

override CFLAGS += -std=c11 -Wall -Wextra -Wpedantic -Werror -Iinclude
override CPPCHECK_FLAGS += -Iinclude --std=c11

DYNABLOCK_BUILD := build
DYNABLOCK_LIB := $(DYNABLOCK_BUILD)/libdynablock.a
DYNABLOCK_OBJ := $(DYNABLOCK_BUILD)/dynablock.o
DYNABLOCK_TEST := $(DYNABLOCK_BUILD)/test_dynablock

.PHONY: lib test check clean

lib: $(DYNABLOCK_LIB)

$(DYNABLOCK_OBJ): src/dynablock.c include/dynablock.h
	mkdir -p $(DYNABLOCK_BUILD)
	$(CC) $(CFLAGS) -c -o $@ src/dynablock.c

$(DYNABLOCK_LIB): $(DYNABLOCK_OBJ)
	$(AR) rcs $@ $(DYNABLOCK_OBJ)

$(DYNABLOCK_TEST): tests/test_dynablock.c include/dynablock.h $(DYNABLOCK_LIB)
	$(CC) $(CFLAGS) -o $@ tests/test_dynablock.c $(DYNABLOCK_LIB) $(LDFLAGS)

test: $(DYNABLOCK_TEST)
	./$(DYNABLOCK_TEST)

check: $(if $(filter c,$(SKILLS)),c-check) test

clean:
	rm -rf $(DYNABLOCK_BUILD)
