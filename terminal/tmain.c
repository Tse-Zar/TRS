#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <colors.h>
#include <dirent.h>

static char current[100] = "space/user";

static void command_parser(char* com) {
    if(strcmp(com, "view\0") == 0) {
        cli_clear();
        const char* statue[24] = {"    %%%", "   =====","  &%&%%%&", "  %& < <% ",
                                    "   &\\__/","    \\ |____","   .', ,  ()","  / -.  _)| "," |_(_.    |",
                                    " '-'\\  )  |"," trs )    |","    /  .  ).","   /    _. |"," /'---':.-'|",
                                    "(__.' /    /"," \\   ( /  /","  \\ /  _  | ","   \\  |  '|","   | . \\  |",
                                    "   |(     | ","   |  \\ \\ |","    \\  )\\ |","   __)/ / \\","--\"--(_.Ooo'----" };
        for(int i = 0; i < 24; ++i) {
            set_crsr(55, 0);
            set_color(_Red, _TermBG);
            printf(statue[i]);
            printf("\n");
        }
        printf("==============================================================================================================================");
        set_color(0xFFBF00, 0x181513);
    }

    if(strcmp(com, "clr\0") == 0) {
        cli_clear();
    }

    if(strcmp(com, "pwd\0") == 0) {
        set_color(_Sapphire, _TermBG);
        printf("\n\t");
        printf(current);
        set_color(_Yantar, _TermBG);
    }

   /* if(strcmp(com, "ls\0") == 0) {
        DIR dir = opendir(current);
        
        for(unsigned int i = 0; i <= dir.entries; ++i) {
            FILE f;
            if(de.type == TYPE_FILE) set_color(_Peach, _TermBG);
            else set_color(_Blue, _TermBG);

            printf("\n\t");
            printf(f.name);
        }

        closedir(&dir);
    }*/

    if(strcmp(com, "cd\0") == 0) {

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