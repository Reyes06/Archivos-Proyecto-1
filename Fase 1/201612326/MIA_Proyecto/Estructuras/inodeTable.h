#ifndef INODETABLE_H
#define INODETABLE_H

#include <time.h>

struct inodeTable{
    int i_uid;
    int i_gid;
    int i_size;
    struct tm i_atime;
    struct tm i_ctime;
    struct tm i_mtime;
    int i_block[15];
    char i_type;
    int i_perm;
};

#endif // INODETABLE_H
