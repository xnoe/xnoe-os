CFLAGS = -m32 -mgeneral-regs-only -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fno-stack-protector -Wno-pointer-to-int-cast
LDFLAGS = 

DISK_IMG_FILES = kernel.bin
KERNEL32_OBJS = screenstuff.o io.o idt.o keyboard.o strings.o atapio.o c_code_entry.o kernel.o

run: disk.img
	qemu-system-x86_64 disk.img

disk.img: clean boot.sector boot.stage2 $(DISK_IMG_FILES)
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=boot.sector of=disk.img conv=notrunc
	dd obs=512 seek=1 if=boot.stage2 of=disk.img conv=notrunc
	mount disk.img img.d
	cp *.bin img.d/
	cp hello.txt img.d/
	umount img.d
	chmod 777 disk.img

clean:
	rm $(DISK_IMG_FILES) $(KERNEL32_OBJS) boot.sector disk.img || true

boot.sector: boot.asm
	nasm $< -o $@

boot.stage2: boot_stage2.ld boot.stage2.o
	ld $(LDFLAGS) -T $< boot.stage2.o

boot.stage2.o: src/boot_stage2/main.c io.o atapio.o strings.o c_code_entry.o screenstuff.o
	gcc $(CFLAGS) -o $@ -c $<

%.bin: %.asm
	nasm $< -o $@

kernel.bin: kernel.ld $(KERNEL32_OBJS)
	ld $(LDFLAGS) -T $< $(KERNEL32_OBJS)

%.o: src/kernel/%.asm
	nasm -felf32 $< -o $@

%.o: src/%.asm
	nasm -felf32 $< -o $@

%.o: src/kernel/%.c
	gcc $(CFLAGS) -o $@ -c $<