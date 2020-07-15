#ifndef RMDISK_H
#define RMDISK_H

#include <string.h>
#include <iostream>
#include "Analizador/comando.h"
#include <sys/stat.h>
#include "Estructuras/mbr.h"

int ejecutarRmdisk(struct comando comando);

#endif // RMDISK_H
