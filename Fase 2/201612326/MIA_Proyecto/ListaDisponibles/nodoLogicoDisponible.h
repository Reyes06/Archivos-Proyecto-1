#ifndef NODOLOGICODISPONIBLE_H
#define NODOLOGICODISPONIBLE_H

#include "Estructuras/ebr.h"

struct nodoLogicoDisponible {
    struct ebr particion;
    struct nodoLogicoDisponible *siguiente;
};


#endif // NODOLOGICODISPONIBLE_H
