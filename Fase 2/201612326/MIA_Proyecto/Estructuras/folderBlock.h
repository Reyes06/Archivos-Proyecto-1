#ifndef FOLDERBLOCK_H
#define FOLDERBLOCK_H

struct content{
    char b_name[12];
    int b_inode;
};

struct folderBlock{
    struct content b_content[4];
};

#endif // FOLDERBLOCK_H
