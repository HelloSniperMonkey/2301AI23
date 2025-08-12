// kernel.c
#define VIDEO_MEMORY 0xB8000
#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 25

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

volatile uint16_t* video = (uint16_t*) VIDEO_MEMORY;

int cursor_pos = 0;

void clear_screen() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        video[i] = (0x07 << 8) | ' ';
    }
    cursor_pos = 0;
}

void print_char(char c) {
    if (c == '\n') {
        cursor_pos += SCREEN_WIDTH - (cursor_pos % SCREEN_WIDTH);
    } else {
        video[cursor_pos++] = (0x07 << 8) | c;
    }
    if (cursor_pos >= SCREEN_WIDTH * SCREEN_HEIGHT) {
        // Scroll up by one line
        for (int i = 0; i < SCREEN_WIDTH * (SCREEN_HEIGHT - 1); i++) {
            video[i] = video[i + SCREEN_WIDTH];
        }
        // Clear last line
        for (int i = SCREEN_WIDTH * (SCREEN_HEIGHT - 1); i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
            video[i] = (0x07 << 8) | ' ';
        }
        cursor_pos -= SCREEN_WIDTH;
    }
}

void print_string(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

void print_prompt() {
    print_string("\n> ");
}

// Simple string compare (null terminated)
int strcmp(const char* s1, const char* s2) {
    while (*s1 && *s1 == *s2) {
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

int strlen(const char* s) {
    int len = 0;
    while (s[len]) len++;
    return len;
}

// Simple string copy
void strcpy(char* dest, const char* src) {
    while ((*dest++ = *src++));
}

// Read keyboard input from port 0x60 (polling)
char read_char() {
    char c = 0;
    while (1) {
        unsigned char status;
        __asm__ __volatile__ (
            "inb $0x64, %0"
            : "=a"(status)
        );
        if (status & 1) {  // Output buffer full
            __asm__ __volatile__ (
                "inb $0x60, %0"
                : "=a"(c)
            );
            break;
        }
    }
    return c;
}

// Translate scan code to ASCII (simplified, only letters, digits and some punctuation)
char scan_code_to_ascii(char scan) {
    // Simple US keyboard map for common keys
    static const char map[128] = {
        0,  27, '1','2','3','4','5','6','7','8','9','0','-','=','\b',
        '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n',
        0, /* Ctrl */
        'a','s','d','f','g','h','j','k','l',';','\'','`',
        0, /* Left Shift */
        '\\','z','x','c','v','b','n','m',',','.','/',
        0, /* Right Shift */
        '*',
        0, /* Alt */
        ' ', /* Space */
        0, /* Caps lock and above ignored */
    };
    if (scan < 128) return map[(int)scan];
    return 0;
}

// Shell input buffer
#define INPUT_BUF_SIZE 128
char input_buffer[INPUT_BUF_SIZE];
int input_index = 0;

void shell_loop() {
    print_string("Welcome to DummyOS Shell!\nType 'help' for commands.\n");
    print_prompt();

    while (1) {
        char c = 0;
        // Read raw scan code (poll keyboard)
        c = read_char();

        // Key released? skip (scan codes with high bit set are release codes)
        if (c & 0x80) continue;

        char ch = scan_code_to_ascii(c);
        if (ch == 0) continue;

        if (ch == '\b') {  // Backspace
            if (input_index > 0) {
                input_index--;
                print_char('\b');
                print_char(' ');
                print_char('\b');
            }
            continue;
        }
        if (ch == '\n') {  // Enter
            print_char('\n');
            input_buffer[input_index] = 0; // null-terminate

            // Process command
            if (strcmp(input_buffer, "help") == 0) {
                print_string("Available commands: help, clear, echo, exit\n");
            } else if (strcmp(input_buffer, "clear") == 0) {
                clear_screen();
            } else if (input_buffer[0] == 'e' && input_buffer[1] == 'c' &&
                       input_buffer[2] == 'h' && input_buffer[3] == 'o' && input_buffer[4] == ' ') {
                print_string(input_buffer + 5);
                print_char('\n');
            } else if (strcmp(input_buffer, "exit") == 0) {
                print_string("Goodbye!\n");
                // Hang here (no shutdown)
                while(1) { __asm__("hlt"); }
            } else {
                print_string("Unknown command. Type 'help' for commands.\n");
            }

            input_index = 0;
            print_prompt();
        } else {
            if (input_index < INPUT_BUF_SIZE - 1) {
                input_buffer[input_index++] = ch;
                print_char(ch);
            }
        }
    }
}

void main() {
    clear_screen();
    shell_loop();
}
