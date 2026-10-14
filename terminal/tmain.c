#include <stdio.h>
#include <stdbool.h>

static char current[100] = "space/user";

static void command_parser(char* com) {

}

int main() {
    printf("{{ ts3zaR t3rm1nal has start3d }}\n");
    printf("   ===========================   \n\n");

    char command[16];
    while(true) {
        printf(current);
        printf(" >> ");
        scanf("%s", command);
    }
    return 0;
}