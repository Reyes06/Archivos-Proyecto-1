#ifndef NODOPARTICION_H
#define NODOPARTICION_H

#include "Estructuras/partition.h"
#include "Estructuras/ebr.h"

struct nodoParticion{
    char part_name[16];
    char part_type;
    int numeroMontaje;
    struct nodoParticion *siguiente;
};

#endif // NODOPARTICION_H
