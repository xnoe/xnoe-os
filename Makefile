CFLAGS = -m32 -nostdlib -fno-builtin -fno-exceptions -fno-leading-underscore -fno-pie -fno-stack-protector
LDFLAGS = 

disk.img: boot.sector kernel.bin hello.bin print.bin hello.txt boot32.bin kernel32.bin
	dd if=/dev/zero of=disk.img count=43 bs=100k
	dd if=boot.sector of=disk.img conv=notrunc
	mount disk.img img.d
	cp *.bin img.d/
	cp hello.txt img.d/
	umount img.d
	chmod 777 disk.img

boot.sector: boot.asm
	nasm $< -o $@

kernel.bin: kernel.asm
	nasm $< -o $@

hello.bin: hello.asm
	nasm $< -o $@

print.bin: print.asm
	nasm $< -o $@

boot32.bin: boot32.asm
	nasm $< -o $@

kernel32.bin: kernel32.ld kernel32_strap.o kernel32.o
	ld $(LDFLAGS) -T $<

kernel32.o: kernel32.c
	gcc $(CFLAGS) -o $@ -c $<

kernel32_strap.o: kernel32_strap.asm
	nasm -felf32 $< -o $@