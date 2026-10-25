#include "inc/comands.h"
#include <stdio.h>
#include <colors.h>
#include <dirent.h>

#define COMANDS_COUNT 5

const char comand_name[COMANDS_COUNT][16]  = {"help", "view", "pwd", "clr", "wh"};
const char comand_disc[COMANDS_COUNT][256] = {"show this text\n", "show represent\n", "print work directory\n", "clear console\n",
                                              "what`s here - info abot current directory\n"};
int view(void) {
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
    return 0;
}

int wh(const char* current) {
    DIR dir;
    if(opendir(&dir, current) != 0) {
        printf("wh: cannot open directory\n");
        return -1;
    }

    DirInfo info;
    while(readdir(&dir, &info) == 0) {
        if(info.attr & ATTR_DIR)
            set_color(_Blue, _TermBG);
        else
            set_color(_Peach, _TermBG);

        printf("\n\t");
        printf(info.name);
    }

    set_color(_Yantar, _TermBG);
    closedir(&dir);
    return 0;
}

int help(void) {
    printf("\n>> W3lc0m3 t0 t3rm h3lp <<\n------------------------------------------------------\n");
    
    for(int i = 0; i < COMANDS_COUNT; ++i) {
        printf("- "); printf(comand_name[i]);
        printf("\t # "); printf(comand_disc[i]);
    }
    return 0;
}