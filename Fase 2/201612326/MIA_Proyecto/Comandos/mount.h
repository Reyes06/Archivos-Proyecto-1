#ifndef MOUNT_H
#define MOUNT_H

#include "Analizador/comando.h"
#include "ListaMontaje/ListaMontaje.h"
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include "Estructuras/ebr.h"
#include "Estructuras/mbr.h"
#include "Estructuras/superBlock.h"

int ejecutarMount(struct comando comando, ListaMontaje *listaMontaje);

#endif // MOUNT_H
