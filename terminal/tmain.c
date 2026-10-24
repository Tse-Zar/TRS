#include <stdio.h>
#include <stdbool.h>
#include <string.h>

static char current[100] = "space/user";

static void command_parser(char* com) {
    if(strcmp(com, "view\0") == 0) {
        cli_clear();
        const char* statue[24] = {"    %%%", "   =====","  &%&%%%&", "  %& < <% ",
                                    "   &\\__/","    \\ |____","   .', ,  ()","  / -.  _)| "," |_(_.    |",
                                    " '-'\\  )  |"," trs )    |","    /  .  ).","   /    _. |"," /'---':.-'|",
                                    "(__.' /    /"," \\   ( /  /","  \\ /  _  | ","   \\  |  '|","   | . \\  |",
                                    "   |(     | ","   |  \\ \\ |","    \\  )\\ |","   __)/ / \\","--\"--(_.Ooo'----" };
        for(int i = 0; i < 25; ++i) {
            set_crsr(55, 0);
            set_color(0xf38ba8, 0x181513);
            printf(statue[i]);
            printf("\n");
        }
        printf("==============================================================================================================================");
        set_color(0xFFBF00, 0x181513);
    }

    if(strcmp(com, "clr") == 0) {
        cli_clear();
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
        scanf(command, 16);
        command_parser(command);
        printf("\n");
    }
    
    return 0;
}