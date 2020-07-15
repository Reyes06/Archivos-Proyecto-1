#ifndef EBR_H
#define EBR_H

struct ebr {
    char part_status;
    char part_fit;
    int part_start;
    int part_size;
    int part_next;
    char part_name[16];
};

#endif // EBR_H
