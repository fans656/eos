#include "io.h"
#include "util.h"
#include "draw.h"
#include "interrupt.h"
#include "filesystem.h"
#include "malloc.h"
#include "constants.h"
#include "loader.h"
#include "time.h"

void main();

void kernel_entry() {
    asm volatile (
            "mov ax, 16;"
            "mov ds, ax;"
            "mov es, ax;"
            "mov ss, ax;"
            "mov ebp, 0;"
            "mov esp, %0;" :: "i"(STACK_END)
            );
    clear_screen();
    init_pit();
    setup_idt();
    init_memory();
    graphic_init();
    init_filesystem();
    enable_paging();
    main();
    hlt();
}

char cmd[4096];
int i_cmd;

char exe_fpath[256];

void console() {
    printf("\n");
    execute("/bin/art");
    while (true) {
        printf("\n$ ");

        begin_input();
        i_cmd = 0;
        while (true) {
            int ch = getchar();
            if (ch == KEY_ENTER) {
                putchar('\n');
                cmd[i_cmd] = 0;
                break;
            }
            cmd[i_cmd++] = ch;
            putchar(ch);
        }
        end_input();
        
        int bin_end = 0;
        while (cmd[bin_end] != ' ') {
            ++bin_end;
        }
        cmd[bin_end] = 0;
        
        strcpy("/bin/", exe_fpath);
        strcpy(cmd, exe_fpath + 5);

        execute(exe_fpath);
    }
}

void main() {
    char* addr0 = (char*)0;
    printf("Content at vaddr 0x00000000:\n");
    print_mem(addr0, 128);

    printf("\n");

    printf("Content at vaddr 0x40000000:\n");
    char* addr1 = (char*)(1 * GB);
    print_mem(addr1, 128);
}
