# get_next_line 퍼징 키트
# 사용법: make run SRC_DIR=targets/v2_final  (다른 구현은 SRC_DIR 로 지정)

SRC_DIR  ?= targets/v2_final
GNL_SRCS  = $(SRC_DIR)/get_next_line.c $(SRC_DIR)/get_next_line_utils.c
HARNESS   = harness/fuzz_gnl.c
CC        = clang
CFLAGS    = -g -O1 -Wall -Wextra -Werror -I$(SRC_DIR)
SAN       = -fsanitize=fuzzer,address,undefined -fno-sanitize-recover=all
BS_LIST   = 1 42 100000
TIME      = 300

all: $(addprefix bin/fuzz_bs,$(BS_LIST))

bin/fuzz_bs%: $(HARNESS) $(GNL_SRCS)
	@mkdir -p bin
	$(CC) $(CFLAGS) $(SAN) -D BUFFER_SIZE=$* $(HARNESS) $(GNL_SRCS) -o $@

# BUFFER_SIZE=42 로 TIME초 동안 퍼징
run: bin/fuzz_bs42
	@mkdir -p corpus/bs42 crashes
	./bin/fuzz_bs42 corpus/bs42 seeds -max_len=4096 -max_total_time=$(TIME) -artifact_prefix=crashes/

# BUFFER_SIZE 1, 42, 100000 을 차례로 퍼징
run-all: all
	@mkdir -p crashes
	@for bs in $(BS_LIST); do \
		mkdir -p corpus/bs$$bs; \
		echo "=== BUFFER_SIZE=$$bs ==="; \
		./bin/fuzz_bs$$bs corpus/bs$$bs seeds -max_len=4096 -max_total_time=$(TIME) -artifact_prefix=crashes/bs$$bs- || exit 1; \
	done

# AFL++ 로 퍼징 (afl-clang-fast 필요)
afl:
	@mkdir -p bin
	AFL_USE_ASAN=1 afl-clang-fast -g -O1 -I$(SRC_DIR) -D BUFFER_SIZE=42 -fsanitize=fuzzer $(HARNESS) $(GNL_SRCS) -o bin/afl_bs42
	afl-fuzz -i seeds -o afl_out -V $(TIME) -- ./bin/afl_bs42

clean:
	rm -rf bin

fclean: clean
	rm -rf corpus crashes afl_out

.PHONY: all run run-all afl clean fclean
