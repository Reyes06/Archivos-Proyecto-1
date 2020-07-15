#ifndef LISTA_H
#define LISTA_H

#include "nodoDisponible.h"
#include <iostream>
#include "Estructuras/partition.h"
#include "Estructuras/mbr.h"
#include "Estructuras/ebr.h"
#include "ListaDisponibles/listaLogicoDisponibles.h"
#include <fstream>
#include <iomanip>

class ListaDisponibles
{
private:
    struct nodoDisponible *cabecera;
    int disk_size;
    int disk_start;
    void insertar(struct partition particion);
    void generarNodosDisponibles();
public:
    enum fit {
        FIRSTFIT,
        WORSTFIT,
        BESTFIT
    };
    ListaDisponibles(int disk_size);
    int insertarParticion(struct partition nuevaParticion, struct mbr *MBR, fit posicionamiento);
    void getDotDisk(char *path, std::ofstream &fs, struct mbr *MBR);
};

#endif // LISTA_H
