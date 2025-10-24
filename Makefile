# === Paths ===
SRC_DIR := src
BUILD_DIR := build

# === Tools ===
CLANG := clang
GCC := g++
BPFOBJ := $(BUILD_DIR)/ebpf_kprobe_input.o
BPFSKEL := $(BUILD_DIR)/ebpf_kprobe_input.skel.h
BPFPROG := $(SRC_DIR)/ebpf_kprobe_input.c
USERPROG_SRC := $(SRC_DIR)/umain_kprobe.c
USERPROG := $(BUILD_DIR)/umain_kprobe
BPFTool := bpftool

# === Kernel headers (from host) ===
HOST_KERNEL ?= $(shell echo $$HOST_KERNEL)
KDIR := /usr/src/linux-headers-$(HOST_KERNEL)

# === Arch ===
BPF_ARCH := $(shell uname -m)
ifeq ($(BPF_ARCH),x86_64)
    BPF_DEFINE := -D__TARGET_ARCH_x86
    ARCH_DIR := x86
else ifeq ($(BPF_ARCH),aarch64)
    BPF_DEFINE := -D__TARGET_ARCH_arm64
    ARCH_DIR := arm64
else
    $(warning Unknown architecture $(BPF_ARCH), please extend Makefile)
endif

# === Flags ===
BPF_CFLAGS := -O2 -g -target bpf -nostdinc -I$(SRC_DIR) $(BPF_DEFINE) \
    -I$(KDIR)/arch/$(ARCH_DIR)/include \
    -I$(KDIR)/arch/$(ARCH_DIR)/include/uapi \
    -I$(KDIR)/arch/$(ARCH_DIR)/include/generated \
    -I$(KDIR)/arch/$(ARCH_DIR)/include/generated/uapi \
    -I$(KDIR)/include \
    -I$(KDIR)/include/uapi \
    -I$(KDIR)/include/generated/uapi \
    -I$(KDIR)/tools/bpf/resolve_btfids/libbpf/include


LIBBPF_CFLAGS := $(shell pkg-config --cflags libbpf)
LIBBPF_LDFLAGS := $(shell pkg-config --libs libbpf) -lelf -lz

# === Targets ===
.PHONY: all clean dirs

all: dirs $(USERPROG)

dirs:
	mkdir -p $(BUILD_DIR)

# --- generate vmlinux.h from host BTF ---
$(SRC_DIR)/vmlinux.h:
	@echo "Generating vmlinux.h from host BTF..."
	$(BPFTool) btf dump file /sys/kernel/btf/vmlinux format c > $@

# --- build eBPF object ---
$(BPFOBJ): $(BPFPROG) $(SRC_DIR)/vmlinux.h
	$(CLANG) $(BPF_CFLAGS) -c $< -o $@

# --- generate skeleton ---
$(BPFSKEL): $(BPFOBJ)
	$(BPFTool) gen skeleton $< > $@

# --- build userspace ---
$(USERPROG): $(USERPROG_SRC) $(BPFSKEL) src/event_logger.cpp
	$(CXX) -O2 -I$(BUILD_DIR) -Isrc $^ -o $@ $(LIBBPF_CFLAGS) $(LIBBPF_LDFLAGS)

clean:
	rm -rf $(BUILD_DIR) $(SRC_DIR)/vmlinux.h
