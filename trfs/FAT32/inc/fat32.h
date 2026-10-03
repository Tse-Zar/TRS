#ifndef FAT32_H
#define FAT32_H

typedef struct {
    unsigned char jmp_boot[3];
    unsigned char OEMname[8];
    unsigned short bytes_per_sec;
    unsigned char sec_per_clus;
    unsigned short sec_reserved;
    unsigned char nums_FATs;
    unsigned short root_ent_cnt;
    unsigned short tot_sec;
    unsigned char media;
    unsigned short FATsz;
    unsigned short sec_per_trk;
    unsigned short num_heads;
    unsigned int hidd_sec;
    unsigned int total_sec;
    
    unsigned int FAT32_size;
    unsigned short ext_flags;
    unsigned short fs_ver;
    unsigned int root_clus;
    unsigned short fs_info;
    unsigned short bkboot_sec;
    unsigned char reserved[12];
    unsigned char drv_num;
    unsigned char reserved1;
    unsigned char boot_sig;
    unsigned int volID;
    unsigned char volLab[11];
    unsigned char fil_sys_type[8];
} __attribute__((packed)) FAT32_BPB;

void* file_read();
void close_file();
unsigned long long file_size();

#endif // FAT32_H