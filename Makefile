CFLAGS = -g -std=gnu11 -m32 -mgeneral-regs-only -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fno-stack-protector -Wno-pointer-to-int-cast
CXXFLAGS = -g -m32 -fno-use-cxa-atexit -mgeneral-regs-only -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fpermissive -fno-pie -fno-stack-protector -I.
LDFLAGS = 

DISK_IMG_FILES = build/kernel/kernel.bin hello.txt alpha.txt \
								 build/hello/hello.bin

KERNEL_CPP_SRCS = $(wildcard src/kernel/*.cpp) $(wildcard src/kernel/*/*.cpp) 
KERNEL_ASM_SRCS = $(wildcard src/kernel/*.asm)
KERNEL_CPP_OBJS = $(patsubst src/%.cpp,build/%.o,$(KERNEL_CPP_SRCS))
KERNEL_ASM_OBJS = $(patsubst src/%.asm,build/%.o,$(KERNEL_ASM_SRCS))

KERNEL_OBJS = build/kernel/isr.o $(KERNEL_CPP_OBJS) $(KERNEL_ASM_OBJS)

STAGE2_C_SRCS = $(wildcard src/boot_stage2/*.c)
STAGE2_C_OBJS = $(patsubst src/%.c,build/%.o,$(STAGE2_C_SRCS))

STAGE2_OBJS = build/c_code_entry.o $(STAGE2_C_OBJS)

PROGRAM_COMMON = build/program_code_entry.o build/common/common.o

PROGRAM_C_SRCS = $(wildcard src/program/*.c)
PROGRAM_C_OBJS = $(patsubst src/%.c,build/%.o,$(PROGRAM_C_SRCS))
PROGRAM_OBJS = $(PROGRAM_COMMON) $(PROGRAM_C_OBJS)

HELLO_C_SRCS = $(wildcard src/hello/*.c)
HELLO_C_OBJS = $(patsubst src/%.c,build/%.o,$(HELLO_C_SRCS))
HELLO_OBJS = $(PROGRAM_COMMON) $(HELLO_C_OBJS)

WORLD_C_SRCS = $(wildcard src/world/*.c)
WORLD_C_OBJS = $(patsubst src/%.c,build/%.o,$(WORLD_C_SRCS))
WORLD_OBJS = $(PROGRAM_COMMON) $(WORLD_C_OBJS)

.PHONY: run debug prepare clean

run: disk.img
	qemu-system-i386 disk.img

debug: disk.img
	qemu-system-i386 -s -S -no-reboot -no-shutdown disk.img & gdb --command=gdbscript

disk.img: prepare build/boot/boot.bin build/boot_stage2/boot.bin $(DISK_IMG_FILES) build/world/world.bin
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=build/boot/boot.bin of=disk.img conv=notrunc
	dd obs=512 seek=1 if=build/boot_stage2/boot.bin of=disk.img conv=notrunc
	mount disk.img img.d
	mkdir img.d/etc/
	cp $(DISK_IMG_FILES) img.d/
	cp build/world/world.bin img.d/etc/world.bin
	sleep 0.1
	umount img.d
	chmod 777 disk.img

prepare:
	mkdir -p img.d
	mkdir -p build/boot
	mkdir -p build/boot_stage2
	mkdir -p build/kernel
	mkdir -p build/kernel/datatypes
	mkdir -p build/kernel/stdio
	mkdir -p build/program
	mkdir -p build/hello
	mkdir -p build/world
	mkdir -p build/common
	mountpoint img.d | grep not || umount img.d

clean:
	rm -rf build

build/boot/boot.bin: src/boot/boot.asm
	nasm $< -o $@

# Boot Stage 2
build/boot_stage2/boot.bin: src/boot_stage2/boot_stage2.ld $(STAGE2_OBJS)
	ld $(LDFLAGS) -T $< $(STAGE2_OBJS)

build/boot_stage2/%.o: src/boot_stage2/%.c
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

build/program/program.bin: src/program/program.ld $(PROGRAM_OBJS)
	echo $(PROGRAM_OBJS)
	ld $(LDFLAGS) -T $< $(PROGRAM_OBJS)

build/hello/hello.bin: src/hello/hello.ld $(HELLO_OBJS)
	ld $(LDFLAGS) -T $< $(HELLO_OBJS)

build/world/world.bin: src/world/world.ld $(WORLD_OBJS)
	ld $(LDFLAGS) -T $< $(WORLD_OBJS)