#ifndef LISTAVIRTUALDISPONIBLES_H
#define LISTAVIRTUALDISPONIBLES_H

#include <string.h>
#include "nodoLogicoDisponible.h"
#include <iostream>
#include "Estructuras/partition.h"
#include "Estructuras/mbr.h"
#include "Estructuras/ebr.h"

class ListaLogicoDisponibles
{
private:
    int disk_size;
    int disk_start;

public:
    enum fit {
        FIRSTFIT,
        WORSTFIT,
        BESTFIT
    };
    struct nodoLogicoDisponible *cabecera;
    ListaLogicoDisponibles(int disk_size, int disk_start);
    int insertarParticion(struct ebr nuevaParticion, struct ebr ebrInicial, fit posicionamiento, char* path);
    void insertar(struct ebr particion);
    void generarNodosDisponibles();
};
#endif // LISTAVIRTUALDISPONIBLES_H
