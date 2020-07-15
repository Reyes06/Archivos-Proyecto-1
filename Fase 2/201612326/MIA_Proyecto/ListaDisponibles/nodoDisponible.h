#ifndef NODO_H
#define NODO_H

#include "Estructuras/partition.h"
#include "Estructuras/ebr.h"

struct nodoDisponible {
    struct partition particion;
    struct nodoDisponible *siguiente;
};
#endif // NODO_H
