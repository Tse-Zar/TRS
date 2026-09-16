#ifndef VFS_H
#define VFS_H

#include <std.h>

#define VFS_NAME_MAX  31
#define VFS_MAX_NODES 128
#define VFS_PATH_MAX  128
#define VFS_FILE_MAX  4096

#define VFS_M_READ  0x4
#define VFS_M_WRITE 0x2
#define VFS_M_EXEC  0x1

typedef enum { 
    VFS_FILE = 1,
    VFS_DIR  = 2
} vfs_type;

typedef struct vfs_node {
    char          name[VFS_NAME_MAX + 1];
    vfs_type      type;
    unsigned      mode;
    unsigned      sizel;
    unsigned      refcnt;
    unsigned char data[VFS_FILE_MAX];

    struct vfs_node* parent;
    struct vfs_node* child;
    struct vfs_node* next;
} vfs_node_t;

void vfs_init(void);
vfs_node_t* vfs_root(void);
vfs_node_t* vfs_create(vfs_node_t* dir, const char* name, vfs_type type, unsigned mode);
vfs_node_t* vfs_lookup(const char* path);

#endif // VFS_H