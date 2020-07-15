#ifndef MKFS_H
#define MKFS_H

#include "Analizador/comando.h"
#include "ListaMontaje/ListaMontaje.h"

int ejecutarMkfs(struct comando comando, ListaMontaje *lista);
#endif // MKFS_H
