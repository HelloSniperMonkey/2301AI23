
; stage2.asm
BITS 16
ORG 0x0000          ; Offset relative to 0x8000:0

start:
    ; Set DS to the same as CS (which is 0x8000)
    mov ax, 0x8000
    mov ds, ax

    ; Now safe to use DS:SI
    mov si, welcome_msg

.print:
    lodsb           ; AL = [DS:SI], SI++
    or al, al       ; Check for null terminator
    jz .hang
    mov ah, 0x0E
    int 0x10
    jmp .print

.hang:
    jmp $

welcome_msg db "Welcome to Stage 2!", 0

times 512 - ($ - $$) db 0
