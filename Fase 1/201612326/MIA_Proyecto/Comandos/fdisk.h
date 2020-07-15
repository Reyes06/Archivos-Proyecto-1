#ifndef FDISK_H
#define FDISK_H

#include <cctype>

#include <string.h>
#include <stdio.h>
#include <iostream>
#include "Analizador/comando.h"
#include <sys/stat.h>
#include "Estructuras/mbr.h"
#include "Estructuras/ebr.h"
#include "ListaDisponibles/listaDisponibles.h"
#include "ListaDisponibles/listaLogicoDisponibles.h"
#include "ListaMontaje/ListaMontaje.h"

int ejecutarFdisk(struct comando comando, ListaMontaje *listaMontaje, int isRAID);
int crearParticion(std::map<TipoParametro, struct parametro> parametros);
int modificarParticion(std::map<TipoParametro, struct parametro> parametros);
int borrarParticion(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje, int isRAID);
int crearParticionPrimaria(std::map<TipoParametro, struct parametro> parametros, struct mbr *MBR);
int crearParticionLogica(std::map<TipoParametro, struct parametro> parametros, struct mbr *MBR);
int crearParticionExtendida(std::map<TipoParametro, struct parametro> parametros, struct mbr *MBR);
int modificarPriExt(struct partition *particion, int unidadesIncreDecre, int indice, struct mbr *MBR, char* path);
int modificarLog(struct ebr *particion, int unidadesIncreDecre, char* path, int size_extend_partition);
#endif // FDISK_H
