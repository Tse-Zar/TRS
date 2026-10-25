#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <colors.h>
#include "inc/comands.h"

static char current[100] = "space/user";

static void command_parser(char* com) {
    if(!com || *com == '\0') return;

    int argc = 0;
    char* argv[17];

    while(*com && argc < 16) {
        while(*com == ' ' || *com == '\t' || *com == '\n') com++;
        if(*com == '\0') break;

        if(*com == '"') {
            com++;
            argv[argc++] = com;
            while(*com && *com != '"') com++;
        } else {
            argv[argc++] = com;
            while(*com && *com != ' ' && *com != '\t' && *com != '\n' && *com != '\r') com++;
        }

        if(*com) {
            *com = '\0';
            com++;
        }
    }

    argv[argc] = NULL;
    if(argc == 0) return;

    if(strcmp(argv[0], "view") == 0) {
        if(argc > 1) { printf("t3rm: too many arguments"); return;}

        view();
    } else if(strcmp(argv[0], "clr") == 0) {
        if(argc > 1) { printf("t3rm: too many arguments"); return;}

        cli_clear();
    } else if(strcmp(argv[0], "pwd") == 0) {
        if(argc > 1) { printf("t3rm: too many arguments"); return;}

        set_color(_Sapphire, _TermBG);
        printf("\n\t");
        printf(current);
        set_color(_Yantar, _TermBG);
    } else if(strcmp(argv[0], "wh") == 0) {
        char path[128] = "/dsk/";
        if(argc > 1) {
            strcat(path, argv[1]);
        } else {
            strcat(path, current);
        }
        wh(path);
        
    } else if(strcmp(argv[0], "cd") == 0) {
        if(argc < 2) {
            strcpy(current, "space/user");
        } else {
            char* trgt = argv[1];

            if(strlen(trgt) == 1 && trgt[0] == '.') return;
            if(strcmp(trgt, "..") == 0) {
                char* last = strrchr(current, '/');
                if(last != NULL) {
                    *last = '\0';
                }
            } else if(trgt[0] == '/') {
                if(strlen(trgt) < sizeof(current)) {
                    strcpy(current, trgt + 1);
                } else {
                    printf("t3rm: path too long");
                }
            } else {
                if(strlen(current) + 1 + strlen(trgt) < sizeof(current)) {
                    if(strlen(current) > 0 && current[strlen(current) - 1] != '/') {
                        strcat(current, "/");
                    }
                    strcat(current, trgt);
                }
            }
        }
    } else if(strcmp(argv[0], "help") == 0) {
        if(argc > 1) { printf("t3rm: too many arguments"); return;}
        printf("\n");
        help();
    }
    else {
        printf("\nt3rm: unknown command");
    }
}

int main() {
    printf(">> ts3zaR t3rm1nal has start3d <<\n");
    printf("  -----------------------------\n");
    printf(">>> W3lc0m3 t0 TRS || Y0u ar3 b3st t0day <3\n\n\n");
    while(true) {
        char command[256];
        printf(current);
        printf(" * ");
        scanf(command, 256);
        command_parser(command);
        printf("\n");
    }
    
    return 0;
}