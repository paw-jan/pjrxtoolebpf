# === Paths ===
SRC_DIR := src
BUILD_DIR := build

# === Tools ===
CLANG := clang
GCC := gcc
BPFOBJ := $(BUILD_DIR)/ebpf_kprobe_input.o
BPFSKEL := $(BUILD_DIR)/ebpf_kprobe_input.skel.h
BPFPROG := $(SRC_DIR)/ebpf_kprobe_input.c
USERPROG_SRC := $(SRC_DIR)/kprobe_umain.c
USERPROG := $(BUILD_DIR)/kprobe_umain

BPFTool := bpftool

# === Flags ===
BPF_CFLAGS := -O2 -g -target bpf
LIBBPF_CFLAGS := $(shell pkg-config --cflags libbpf)
LIBBPF_LDFLAGS := $(shell pkg-config --libs libbpf) -lelf -lz

# === Targets ===
.PHONY: all clean dirs

all: dirs $(USERPROG)

dirs:
	mkdir -p $(BUILD_DIR)

# --- build eBPF object ---
$(BPFOBJ): $(BPFPROG)
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@

# --- generate skeleton ---
$(BPFSKEL): $(BPFOBJ)
	$(BPFTool) gen skeleton $< -o $@

# --- build userspace ---
$(USERPROG): $(USERPROG_SRC) $(BPFSKEL)
	$(GCC) -O2 $< -o $@ $(LIBBPF_CFLAGS) $(LIBBPF_LDFLAGS)

clean:
	rm -rf $(BUILD_DIR)