LLVMPATH = /Users/baonguyen2604/Desktop/rpi/clang+llvm-16.0.0-arm64-apple-darwin22.0/bin
CLANGFLAGS = -fpic -ffreestanding -mcpu=cortex-a53+nosimd
CSRCFLAGS = -O0 -g -Wall -Wextra --target=aarch64-elf
ASRCFLAGS = $(CSRCFLAGS) -D__ASSEMBLY__
LDFLAGS = -m aarch64elf -nostdlib

# Location of the files
KER_SRC = ../src/kernel
KER_HEAD = ../include
COMMON_SRC = ../src/common
OBJ_DIR = objects
KERSOURCES = $(wildcard $(KER_SRC)/*.c)
COMMONSOURCES = $(wildcard $(COMMON_SRC)/*.c)
ASMSOURCES = $(wildcard $(KER_SRC)/*.S)
OBJECTS = $(patsubst $(KER_SRC)/%.c, $(OBJ_DIR)/%.o, $(KERSOURCES))
OBJECTS += $(patsubst $(COMMON_SRC)/%.c, $(OBJ_DIR)/%.o, $(COMMONSOURCES))
OBJECTS += $(patsubst $(KER_SRC)/%.S, $(OBJ_DIR)/%.o, $(ASMSOURCES))
HEADERS = $(wildcard $(KER_HEAD)/*.h)

IMG_NAME=kernel8.img
ELF_NAME=kernel8.elf
ASM_NAME=kernel8.asm

build: $(OBJECTS) $(HEADERS)
	echo $(OBJECTS)
	$(LLVMPATH)/ld.lld $(LDFLAGS) $(OBJECTS) -T linker.ld -o $(ELF_NAME)
	$(LLVMPATH)/llvm-objcopy -O binary $(ELF_NAME) $(IMG_NAME)
	$(LLVMPATH)/llvm-objdump -S $(ELF_NAME) > $(ASM_NAME)

$(OBJ_DIR)/%.o: $(KER_SRC)/%.c
	mkdir -p $(@D)
	$(LLVMPATH)/clang $(CLANGFLAGS) $(CSRCFLAGS) -I$(KER_HEAD) -c $< -o $@

$(OBJ_DIR)/%.o: $(KER_SRC)/%.S
	mkdir -p $(@D)
	$(LLVMPATH)/clang $(CLANGFLAGS) $(ASRCFLAGS) -I$(KER_HEAD) -c $< -o $@

$(OBJ_DIR)/%.o: $(COMMON_SRC)/%.c
	mkdir -p $(@D)
	$(LLVMPATH)/clang $(CLANGFLAGS) $(CSRCFLAGS) -I$(KER_HEAD) -c $< -o $@

clean:
	rm -rf $(OBJ_DIR)
	rm $(IMG_NAME)
	rm $(ELF_NAME)

qemu: build
	qemu-system-aarch64 -m 512 -M raspi3ap -serial stdio -kernel kernel8.img

qemu-miniuart: build
	qemu-system-aarch64 -m 512 -M raspi3ap -serial null -serial stdio -kernel kernel8.img

qemu-gdb-miniuart: build
	qemu-system-aarch64 -m 512 -M raspi3ap -serial null -serial stdio -kernel kernel8.img -s -S

qemu-gdb: build
	qemu-system-aarch64 -m 512 -M raspi3ap -serial stdio -kernel kernel8.img -s -S