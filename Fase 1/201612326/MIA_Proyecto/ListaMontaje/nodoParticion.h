#ifndef NODOPARTICION_H
#define NODOPARTICION_H

#include "Estructuras/partition.h"
#include "Estructuras/ebr.h"

struct nodoParticion{
    struct partition particion;
    int numeroMontaje;
    struct nodoParticion *siguiente;
};

#endif // NODOPARTICION_H
