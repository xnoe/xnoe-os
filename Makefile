CFLAGS = -g -std=gnu11 -m32 -mgeneral-regs-only -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fno-stack-protector -Wno-pointer-to-int-cast
CXXFLAGS = -g -m32 -fno-use-cxa-atexit -mgeneral-regs-only -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fpermissive -fno-pie -fno-stack-protector -I.
LDFLAGS = 

DISK_IMG_FILES = build/kernel/kernel.bin hello.txt alpha.txt
KERNEL_OBJS = build/kernel/entry.o build/kernel/screenstuff.o build/kernel/io.o build/kernel/idt.o build/kernel/keyboard.o \
							build/kernel/strings.o build/kernel/atapio.o build/kernel/kmain.o build/kernel/paging.o build/kernel/allocate.o \
							build/kernel/gdt.o build/kernel/memory.o build/kernel/process.o build/kernel/datatypes/hash.o \
							build/kernel/terminal.o build/kernel/global.o build/kernel/kernel.o
STAGE2_OBS = build/c_code_entry.o build/boot_stage2/io.o build/boot_stage2/atapio.o build/boot_stage2/strings.o build/boot_stage2/screenstuff.o build/boot_stage2/stage2.o build/boot_stage2/paging.o

run: disk.img
	qemu-system-x86_64 disk.img

debug: disk.img
	qemu-system-x86_64 -s -S -no-reboot -no-shutdown disk.img & gdb --command=gdbscript

disk.img: clean prepare build/boot/boot.bin build/boot_stage2/boot.bin $(DISK_IMG_FILES)
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=build/boot/boot.bin of=disk.img conv=notrunc
	dd obs=512 seek=1 if=build/boot_stage2/boot.bin of=disk.img conv=notrunc
	mount disk.img img.d
	cp $(DISK_IMG_FILES) img.d/
	umount img.d
	chmod 777 disk.img

prepare:
	mkdir -p img.d
	mkdir -p build/boot
	mkdir -p build/boot_stage2
	mkdir -p build/kernel
	mkdir -p build/kernel/datatypes
	mountpoint img.d | grep not || umount img.d

clean:
	rm -rf build

build/boot/boot.bin: src/boot/boot.asm
	nasm $< -o $@

# Boot Stage 2
build/boot_stage2/boot.bin: src/boot_stage2/boot_stage2.ld $(STAGE2_OBS)
	ld $(LDFLAGS) -T $< $(STAGE2_OBS)

build/boot_stage2/%.o: src/boot_stage2/%.c
	gcc $(CFLAGS) -o $@ -c $<

# Kernel
build/kernel/kernel.bin: build/kernel/kernel.elf
	objcopy -O binary build/kernel/kernel.elf build/kernel/kernel.bin

build/kernel/kernel.elf: src/kernel/kernel.ld $(KERNEL_OBJS)
	ld $(LDFLAGS) -T $< $(KERNEL_OBJS)

build/boot_stage2/stage2.o: src/boot_stage2/main.c 
	gcc $(CFLAGS) -o $@ -c $<

build/kernel/%.o: src/kernel/%.c
	gcc $(CFLAGS) -o $@ -c $<

build/kernel/%.o: src/kernel/%.cpp
	g++ $(CXXFLAGS) -o $@ -c $<

build/kernel/%.o: src/kernel/%.asm
	nasm -felf32 $< -o $@

# Generic
build/%.o: src/%.asm
	nasm -felf32 $< -o $@