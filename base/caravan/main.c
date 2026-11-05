/*
    Симулятор выживания в постапокалисптическом мире. Простенькая текстовая игра с обязательным отслеживанием параметров персонажа.
*/

#include <stdio.h>
#include <stdbool.h>
#include <colors.h>

static int caps   = 100;
static int health = 10;
static int water = 40, food = 40;
static unsigned int inventory_max = 10;

int main() {
    while(true) {
        cli_clear();
        set_color(_Yantar, _TermBG);
        printf("=== CARAVAN ===");
        printf("location: \n");
        printf("day: ");
        printf("caps: "); printf("\thp: "); printf("\tw/f: ");

        return 0;
    }
}