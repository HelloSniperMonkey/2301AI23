# Rebuild everything from scratch
echo "Building DummyOS..."

# 1. Build bootloader (with fixes)
nasm -f bin -o boot.bin boot.asm

# 2. Build kernel  
i686-elf-gcc -ffreestanding -nostdlib -fno-builtin -m16 -Os -Wall \
    -c -o kernel.o kernel.c
i686-elf-ld -melf_i386 -Ttext 0x8000 -nostdlib -e _start \
    -o kernel.elf kernel.o
i686-elf-objcopy -O binary kernel.elf kernel.bin

# 3. Create floppy image
dd if=/dev/zero of=floppy.img bs=512 count=2880

# 4. Write boot sector (sector 1)
dd if=boot.bin of=floppy.img bs=512 seek=0 count=1 conv=notrunc

# 5. Write kernel starting at sector 2
dd if=kernel.bin of=floppy.img bs=512 seek=1 conv=notrunc

# 6. Run
qemu-system-i386 -fda floppy.img