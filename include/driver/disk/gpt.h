/*
    ========================
    = TSEZAR TSEZAR TSEZAR =
    = -------------------- =
    = GPT partition parser =
    ========================
*/

#ifndef GPT_H
#define GPT_H

#include <types.h>

typedef struct {
    ullong first_lba;
    ullong last_lba;
} gpt_part_t;

int gpt_find_partition(uint disk_id, const uchar* type_guid, gpt_part_t* out);
int gpt_find_esp(uint disk_id, gpt_part_t* out);

#endif //GPT_H
