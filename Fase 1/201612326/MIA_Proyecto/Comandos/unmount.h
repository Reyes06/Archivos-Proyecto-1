#ifndef UNMOUNT_H
#define UNMOUNT_H

#include "Analizador/comando.h"
#include "ListaMontaje/ListaMontaje.h"
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>

int ejecutarUnmount(struct comando comando, ListaMontaje *listaMontaje);

#endif // UNMOUNT_H
