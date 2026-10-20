#include <stdio.h>
#include <stdbool.h>

static char current[100] = "space/user";

/*static void command_parser(char* com) {

}*/

int main() {
    printf(">> ts3zaR t3rm1nal has start3d <<\n");
    printf("  -----------------------------\n");
    printf(">>> W3lc0m3 t0 TRS || Y0u ar3 b3st t0day <3\n\n\n");
    //char command[16];
    while(true) {
        printf(current);
        printf(" * ");
        //scanf(command, 16);


        while(1) {
            __asm__ volatile("hlt");
        }
    }
    return 0;
}