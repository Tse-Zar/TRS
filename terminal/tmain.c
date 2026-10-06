#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_COMMAND_SIZE 1000
int main() {
    printf("{{ ts3zaR t3rm1nal has start3d }}\n");
    
    char* command = malloc(MAX_COMMAND_SIZE + 1);

    while(1) {
        scanf("%s", command);
        if(strcmp(command, "clear") == 0) {
            console_clear();
        }
    }
    return 0;
}