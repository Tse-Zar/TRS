/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = GPT partition parser ------------- =
    = ====================================
*/

#include <gpt.h>
#include <disk.h>
#include <string.h>

static const unsigned char esp_type_guid[16] = {
      0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11,
    0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
};

typedef struct {
    unsigned char      sig[8];
    unsigned int       revision;
    unsigned int       header_size;
    unsigned int       header_crc;
    unsigned int       reserved;
    unsigned long long my_lba;
    unsigned long long alt_lba;
    unsigned long long first_usable;
    unsigned long long last_usable;
    unsigned char      disk_guid[16];
    unsigned long long parts_lba;
    unsigned int       parts_count;
    unsigned int       parts_size;
    unsigned int       parts_crc;
} __attribute__((packed)) gpt_header_t;

typedef struct {
    unsigned char      type_guid[16];
    unsigned char      part_guid[16];
    unsigned long long first_lba;
    unsigned long long last_lba;
    unsigned long long attrs;
    unsigned short     name[36];
} __attribute__((packed)) gpt_entry_t;

int gpt_find_partition(unsigned int disk_id, const unsigned char *type_guid, gpt_part_t *out) {
    unsigned char sec[DISK_SECTOR_SIZE];

    if(read_disk(disk_id, 1, 1, sec) != 0) return -1;

    const gpt_header_t* h = (const gpt_header_t*)sec;
    if(memcmp(h->sig, "EFI PART", 8) != 0) return -1;
    if(h->parts_count == 0 || h->parts_count > 512) return -1;
    if(h->parts_size < sizeof(gpt_entry_t) || h->parts_size > DISK_SECTOR_SIZE) return -1;

    unsigned long long parts_per_sector = DISK_SECTOR_SIZE / h->parts_size;

    for(unsigned long long i = 0; i < h->parts_count; ++i) {
        if(i % parts_per_sector == 0) {
            if(read_disk(disk_id, h->parts_lba + i / parts_per_sector, 1, sec) != 0) return -1;
        }

        const gpt_entry_t* entry = (const gpt_entry_t*)(sec + (i % parts_per_sector) * h->parts_count);

        unsigned char empty = 1;
        for(unsigned b = 0; b < 16; ++b) {
            if(entry->type_guid[b]) { empty = 0; break; }
        }
        if(empty) continue;

        if(memcmp(entry->type_guid, type_guid, 16) == 0) {
            out->frist_lba = entry->first_lba;
            out->last_lba  = entry->last_lba;
            return 0;
        }
    }

    return -1;
}

int gpt_find_esp(unsigned int disk_id, gpt_part_t *out) {
    return gpt_find_partition(disk_id, esp_type_guid, out);
}