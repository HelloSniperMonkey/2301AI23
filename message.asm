[BITS 16]
[ORG 0x7C00]

start:
    cli                 ; Clear interrupts
    xor ax, ax
    mov ds, ax          ; Set data segment to 0
    mov ss, ax          ; Set stack segment to 0
    mov sp, 0x7C00      ; Set stack pointer to top of bootloader

    mov si, msg_welcome
    call print_string   ; Print welcome message

    ; Read kernel from floppy disk
    mov ah, 0x02         ; BIOS read sector function
    mov al, 10           ; Number of sectors to read (adjust as needed)
    mov ch, 0x00         ; Cylinder number
    mov cl, 0x02         ; Sector number (start at sector 2)
    mov dh, 0x00         ; Head number
    mov bx, 0x1000       ; Load address (0x1000)
    int 0x13             ; BIOS interrupt to read sectors
    jc load_error        ; Jump if there was an error

    jmp 0x1000           ; Jump to the kernel entry point

load_error:
    mov si, err_msg
    call print_string
    jmp $               ; Hang forever

print_string:
    mov ah, 0x0E        ; BIOS teletype output
.print_char:
    lodsb               ; Load byte from SI into AL
    cmp al, 0           ; Check for null terminator
    je .done
    int 0x10            ; Print character
    jmp .print_char
.done:
    ret

msg_welcome:
    db "I am soumyajyoti mohanta", 0x0D, 0x0A, 0
err_msg:
    db "Disk read error!", 0x0D, 0x0A, 0

times 510 - ($ - $$) db 0  ; Pad to 510 bytes
dw 0xAA55                  ; Boot signature