#ifndef MKDISK_H
#define MKDISK_H

#include <string.h>
#include <stdio.h>
#include <iostream>
#include "Analizador/comando.h"
#include <sys/stat.h>
#include "Estructuras/mbr.h"

int ejecutarMkdisk(struct comando comando);
struct mbr *getDefaultMBR(std::map<TipoParametro, struct parametro> parametros);
int crearDirectorio(char *path);

#endif // MKDISK_H
