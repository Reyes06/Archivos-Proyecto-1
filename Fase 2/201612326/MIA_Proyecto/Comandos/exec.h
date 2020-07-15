#ifndef EXEC_H
#define EXEC_H

#include "Comandos/mkdisk.h"
#include "Comandos/rmdisk.h"
#include "Comandos/fdisk.h"
#include "Comandos/rep.h"
#include "Comandos/mount.h"
#include "Comandos/unmount.h"

#include "Comandos/mkfs.h"
#include "Comandos/login.h"
#include "Comandos/logout.h"
#include "Comandos/mkgrp.h"
#include "Comandos/rmgrp.h"
#include "Comandos/mkusr.h"
#include "Comandos/rmusr.h"
#include "Comandos/chmod.h"
#include "Comandos/mkfile.h"
#include "Comandos/cat.h"
#include "Comandos/edit.h"
#include "Comandos/ren.h"
#include "Comandos/mkdir.h"
#include "Comandos/cp.h"
#include "Comandos/mv.h"
#include "Comandos/chown.h"
#include "Comandos/chgrp.h"
#include "Comandos/find.h"

#include "Analizador/parser.h"
#include "Analizador/scanner.h"
#include "Analizador/comando.h"
#include "Estructuras/mbr.h"
#include "Estructuras/user.h"
#include "ListaMontaje/ListaMontaje.h"

#include <iostream>
#include <fstream>
#include <sys/stat.h>

extern struct comando comando;

void modificarRaidPath(char *pathDisco);
int ejecutarExec(struct comando comando, ListaMontaje *listaMontaje);
int parsearArchivo(char *entrada);
int validar(struct comando comando);
int ejecutar(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);

#endif // EXEC_H
