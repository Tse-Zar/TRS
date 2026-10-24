/*
    ========================
    = TSEZAR TSEZAR TSEZAR =
    = -------------------- =
    = GPT partition parser =
    ========================
*/

#ifndef GPT_H
#define GPT_H

typedef struct {
    unsigned long long frist_lba;
    unsigned long long last_lba;
} gpt_part_t;

int gpt_find_partition(unsigned int disk_id, const unsigned char* type_guid, gpt_part_t* out);
int gpt_find_esp(unsigned int disk_id, gpt_part_t* out);

#endif //GPT_H
