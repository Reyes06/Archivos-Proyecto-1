#ifndef NODODISCO_H
#define NODODISCO_H

#include "nodoParticion.h"

struct nodoDisco{
    char disk_location [200];
    char letraDisco;
    struct nodoParticion *cabeceraParticiones;
    struct nodoDisco *siguiente;
};

#endif // NODODISCO_H
