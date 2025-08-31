; bootloader.asm — Stage 1 Bootloader
; Assemble with: nasm -f bin bootloader.asm -o bootloader.bin

BITS 16
ORG 0x7C00          ; BIOS loads bootloader at 0x7C00

start:
    ; Print "Loading Stage 2..."
    mov si, loading_msg
.print:
    lodsb
    or al, al
    jz .load_stage2
    mov ah, 0x0E
    int 0x10
    jmp .print

.load_stage2:
    ; Set up registers to read stage 2 (1 sector) from disk
    mov ah, 0x02        ; BIOS function: read sectors
    mov al, 1           ; Number of sectors to read
    mov ch, 0           ; Cylinder 0
    mov cl, 2           ; Sector 2 (we already loaded sector 1 at boot)
    mov dh, 0           ; Head 0
    mov dl, 0x00        ; Drive number (0x00 = floppy)
    mov bx, 0x8000      ; Segment offset to load stage 2
    mov es, bx
    xor bx, bx

    int 0x13            ; BIOS disk interrupt

    jc disk_error       ; If carry flag set, read failed

    ; Jump to the code we just loaded
    jmp 0x8000:0000

disk_error:
    mov si, error_msg
.err_print:
    lodsb
    or al, al
    jz .hang
    mov ah, 0x0E
    int 0x10
    jmp .err_print

.hang:
    jmp $

loading_msg db "Loading Stage 2...", 0
error_msg   db "Disk read error!", 0

times 510 - ($ - $$) db 0
dw 0xAA55
