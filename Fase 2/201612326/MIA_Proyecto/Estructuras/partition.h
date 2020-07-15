#ifndef PARTITION_H
#define PARTITION_H

struct partition {
    char part_status;
    char part_type;
    char part_fit;
    int part_start;
    int part_size;
    char part_name[16];
};

#endif // PARTITION_H
