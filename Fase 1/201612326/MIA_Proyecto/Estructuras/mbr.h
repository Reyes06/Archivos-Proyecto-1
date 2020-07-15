#ifndef MBR_H
#define MBR_H

#include "Estructuras/partition.h"
#include <time.h>

struct mbr{
    int mbr_tamano;
    struct tm mbr_fecha_creacion;
    int mbr_disk_signature;
    char disk_fit;
    struct partition mbr_partition[4];
};

#endif // MBR_H
