#ifndef DIRENT_H
#define DIRENT_H

#define TYPE_FILE 1
#define TYPE_DIR  2

typedef struct {
    const char*  name;
    unsigned int entries;
} DIR;

DIR opendir(const char* name);
void closedir(DIR* dir);

#endif //DIRENT_H