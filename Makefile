CFLAGS = -m32 -mgeneral-regs-only -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fno-stack-protector -Wno-pointer-to-int-cast
LDFLAGS = 

DISK_IMG_FILES = kernel.bin hello.bin print.bin boot32.bin kernel32.bin
KERNEL32_OBJS = screenstuff.o io.o idt.o keyboard.o strings.o atapio.o kernel32_strap.o kernel32.o

run: disk.img
	qemu-system-x86_64 disk.img

disk.img: clean boot.sector $(DISK_IMG_FILES)
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=boot.sector of=disk.img conv=notrunc
	mount disk.img img.d
	cp *.bin img.d/
	cp hello.txt img.d/
	umount img.d
	chmod 777 disk.img

clean:
	rm $(DISK_IMG_FILES) $(KERNEL32_OBJS) boot.sector disk.img || true

boot.sector: boot.asm
	nasm $< -o $@

%.bin: %.asm
	nasm $< -o $@

kernel32.bin: kernel32.ld $(KERNEL32_OBJS)
	ld $(LDFLAGS) -T $< $(KERNEL32_OBJS)

%.o: %.asm
	nasm -felf32 $< -o $@

%.o: %.c
	gcc $(CFLAGS) -o $@ -c $<