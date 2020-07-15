#ifndef MKFS_H
#define MKFS_H

#include "Analizador/comando.h"
#include "Estructuras/mbr.h"
#include "Estructuras/ebr.h"
#include "getters.h"
#include "ListaMontaje/ListaMontaje.h"
#include "Comandos/mkfile.h"
#include <fstream>

int ejecutarMkfs(struct comando comando, ListaMontaje *lista);
struct superBlock *obtenerDefaultSuperBloque(int partition_size);
struct journal *obtenerDefaultJournal();
void crearArchivoUsers(char* idParticion, ListaMontaje *lista);

#endif // MKFS_H
