#ifndef EXEC_H
#define EXEC_H

#include "Comandos/mkdisk.h"
#include "Comandos/rmdisk.h"
#include "Comandos/fdisk.h"
#include "Comandos/rep.h"
#include "Comandos/mount.h"
#include "Comandos/unmount.h"
#include "Analizador/parser.h"
#include "Analizador/scanner.h"
#include "Analizador/comando.h"
#include "Estructuras/mbr.h"
#include "ListaMontaje/ListaMontaje.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>

extern struct comando comando;

void modificarRaidPath(char *pathDisco);
int ejecutarExec(struct comando comando, ListaMontaje *listaMontaje);
int parsearArchivo(char *entrada);
int validar(struct comando comando);
int ejecutar(struct comando comando, ListaMontaje *listaMontaje);

#endif // EXEC_H
