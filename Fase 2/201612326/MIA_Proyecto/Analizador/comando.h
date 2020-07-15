#ifndef COMANDO_H
#define COMANDO_H

#include <map>
#include "parametro.h"

enum TipoComando{
    T_MKDISK,
    T_RMDISK,
    T_FDISK,
    T_MOUNT,
    T_UNMOUNT,
    T_EXEC,
    T_REP,
    T_COMENTARIO,
    T_MKFS,
    T_LOGIN,
    T_LOGOUT,
    T_MKGRP,
    T_RMGRP,
    T_MKUSR,
    T_RMUSR,
    T_CHMOD,
    T_MKFILE,
    T_CAT,
    T_REM,
    T_EDIT,
    T_REN,
    T_MKDIR,
    T_CP,
    T_MV,
    T_FIND,
    T_CHOWN,
    T_CHGRP,
    T_PAUSE
};

struct comando {
    TipoComando tipo;
    std::map<TipoParametro, struct parametro>* parametros;
    char comentario[150];
};


#endif // COMANDO_H
