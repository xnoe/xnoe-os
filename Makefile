CFLAGS = -g -std=gnu11 -m32 -mgeneral-regs-only -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fno-stack-protector -Wno-pointer-to-int-cast -Isrc/
CXXFLAGS = -g -m32 -fno-use-cxa-atexit -mgeneral-regs-only -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fpermissive -fno-pie -fno-stack-protector -Isrc/
LDFLAGS = 

DISK_IMG_FILES = build/kernel/kernel.bin hello.txt alpha.txt

KERNEL_CPP_SRCS = $(shell find src/kernel/ -name '*.cpp')
KERNEL_ASM_SRCS = $(shell find src/kernel/ -name '*.asm')
KERNEL_CPP_OBJS = $(patsubst src/%.cpp,build/%.o,$(KERNEL_CPP_SRCS))
KERNEL_ASM_OBJS = $(patsubst src/%.asm,build/%.o,$(KERNEL_ASM_SRCS))

KERNEL_OBJS = build/kernel/isr.o $(KERNEL_CPP_OBJS) $(KERNEL_ASM_OBJS)

STAGE2_C_SRCS = $(wildcard src/bootstage2/*.c)
STAGE2_C_OBJS = $(patsubst src/%.c,build/%.o,$(STAGE2_C_SRCS))

STAGE2_OBJS = build/c_code_entry.o $(STAGE2_C_OBJS)

PROGRAM_COMMON = build/programs/entry.o build/common/common.o

SRC_DIRS = $(shell find src/ -type d)
BUILD_DIRS = $(subst src/,build/,$(SRC_DIRS))

PROGRAMS = $(shell find src/programs/* -type d)
$(foreach program,$(PROGRAMS),\
	$(eval $(program)_C_SRCS := $(shell find $(program) -name '*.c')) \
	$(eval $(program)_CPP_SRCS := $(shell find $(program) -name '*.cpp')) \
	$(eval $(program)_OBJS := $(patsubst src/%.c,build/%.o,$($(program)_C_SRCS)) \
		$(patsubst src/%.cpp,build/%.o,$($(program)_CPP_SRCS))) \
	$(eval DISK_IMG_FILES += $(subst src/,build/,$(program))/$(shell basename $(program).bin)) \
)

.PHONY: run debug prepare clean cleanbuild

run: disk.img
	qemu-system-i386 disk.img

cleanbuild: clean disk.img
	qemu-system-i386 disk.img

debug: clean disk.img
	qemu-system-i386 -s -S -no-reboot -no-shutdown disk.img & gdb --command=gdbscript

disk.img: prepare build/boot/boot.bin build/bootstage2/boot.bin $(DISK_IMG_FILES)
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=build/boot/boot.bin of=disk.img conv=notrunc
	dd obs=512 seek=1 if=build/bootstage2/boot.bin of=disk.img conv=notrunc
	mount disk.img img.d
	mkdir img.d/etc/
	cp $(DISK_IMG_FILES) img.d/
	sleep 0.1
	umount img.d
	chmod 777 disk.img

prepare:
	mkdir -p img.d
	mkdir -p $(BUILD_DIRS)
	mountpoint img.d | grep not || umount img.d

clean:
	rm -rf build

build/boot/boot.bin: src/boot/boot.asm
	nasm $< -o $@

# Boot Stage 2
build/bootstage2/boot.bin: src/bootstage2/bootstage2.ld $(STAGE2_OBJS)
	ld $(LDFLAGS) -T $< $(STAGE2_OBJS)

build/bootstage2/%.o: src/bootstage2/%.c
	gcc $(CFLAGS) -o $@ -c $<

# Kernel
build/%.bin: build/%.elf
	objcopy -O binary $< $@

build/kernel/kernel.elf: src/kernel/kernel.ld $(KERNEL_OBJS)
	ld $(LDFLAGS) -T $< $(KERNEL_OBJS)

build/%.o: src/%.c
	gcc $(CFLAGS) -o $@ -c $<

build/%.o: src/%.cpp
	g++ $(CXXFLAGS) -o $@ -c $<

build/%.o: src/%.asm
	nasm -felf32 $< -o $@

build/kernel/isr.o: src/kernel/isr.S
	nasm -felf32 $< -o $@

src/kernel/isr.S: src/kernel/isr.S.base src/kernel/gen_isr_asm.sh
	cd src/kernel/ && ./gen_isr_asm.sh

# Program

.SECONDEXPANSION:
build/programs/%.bin: src/programs/userspace.ld build/programs/entry.o $$(src/programs/$$(basename $$(notdir $$*))_OBJS) $(PROGRAM_COMMON)
	ld -o $@ -T $^