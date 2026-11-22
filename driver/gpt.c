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

static const uchar esp_type_guid[16] = {
      0x28, 0x73, 0x2A, 0xC1, 0x1F, 0xF8, 0xD2, 0x11,
    0xBA, 0x4B, 0x00, 0xA0, 0xC9, 0x3E, 0xC9, 0x3B
};

typedef struct {
    uchar      sig[8];
    uint       revision;
    uint       header_size;
    uint       header_crc;
    uint       reserved;
    ullong my_lba;
    ullong alt_lba;
    ullong first_usable;
    ullong last_usable;
    uchar      disk_guid[16];
    ullong parts_lba;
    uint       parts_count;
    uint       parts_size;
    uint       parts_crc;
} __attribute__((packed)) gpt_header_t;

typedef struct {
    uchar      type_guid[16];
    uchar      part_guid[16];
    ullong first_lba;
    ullong last_lba;
    ullong attrs;
    ushort     name[36];
} __attribute__((packed)) gpt_entry_t;

int gpt_find_partition(uint disk_id, const uchar *type_guid, gpt_part_t *out) {
    uchar sec[DISK_SECTOR_SIZE];

    if(read_disk(disk_id, 1, 1, sec) != 0) return -1;

    const gpt_header_t* h = (const gpt_header_t*)sec;
    if(memcmp(h->sig, "EFI PART", 8) != 0) return -1;
    if(h->parts_count == 0 || h->parts_count > 512) return -1;
    if(h->parts_size < sizeof(gpt_entry_t) || h->parts_size > DISK_SECTOR_SIZE) return -1;

    ullong parts_per_sector = DISK_SECTOR_SIZE / h->parts_size;

    for(ullong i = 0; i < h->parts_count; ++i) {
        if(i % parts_per_sector == 0) {
            if(read_disk(disk_id, h->parts_lba + i / parts_per_sector, 1, sec) != 0) return -1;
        }

        const gpt_entry_t* entry = (const gpt_entry_t*)(sec + (i % parts_per_sector) * h->parts_size);

        uchar empty = 1;
        for(uint b = 0; b < 16; ++b) {
            if(entry->type_guid[b]) { empty = 0; break; }
        }
        if(empty) continue;

        if(memcmp(entry->type_guid, type_guid, 16) == 0) {
            out->first_lba = entry->first_lba;
            out->last_lba  = entry->last_lba;
            return 0;
        }
    }

    return -1;
}

int gpt_find_esp(uint disk_id, gpt_part_t *out) {
    return gpt_find_partition(disk_id, esp_type_guid, out);
}
