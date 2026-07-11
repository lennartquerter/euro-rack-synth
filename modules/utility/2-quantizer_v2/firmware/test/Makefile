# Host-side unit tests for the HAL-free app code (quantize.c, seq.c)
# plus a host compile check of the generated sequence bank.
# Usage: make        build + run
#        make clean

CC ?= cc
CFLAGS = -std=c11 -Wall -Wextra -Werror -I../Core/Inc

BUILD = build

.PHONY: test clean

test: $(BUILD)/test_quantize $(BUILD)/test_seq $(BUILD)/sequences.o
	./$(BUILD)/test_quantize
	./$(BUILD)/test_seq

$(BUILD)/test_quantize: test_quantize.c ../Core/Src/app/quantize.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

# the engine tests build their own tiny sequences — the generated bank is
# deliberately NOT linked here (proves the engine is bank-agnostic)
$(BUILD)/test_seq: test_seq.c ../Core/Src/app/seq.c ../Core/Src/app/quantize.c
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -o $@ $^

# generated bank must stay pure C — this catches generator regressions
$(BUILD)/sequences.o: ../Core/Src/app/sequences.c ../Core/Inc/app/sequences.h
	@mkdir -p $(BUILD)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD)
