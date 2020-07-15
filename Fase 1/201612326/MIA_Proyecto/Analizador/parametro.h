#ifndef PARAMETRO_H
#define PARAMETRO_H

enum TipoParametro {
    T_SIZE,
    T_FIT,
    T_UNIT,
    T_TYPE,
    T_DELETE,
    T_NAME,
    T_ADD,
    T_ID,
    T_PATH,
    T_FS,
    T_USR,
    T_PWD,
    T_GRP,
    T_UGO,
    T_R,
    T_P,
    T_CONT,
    T_FILE,
    T_DEST
};

struct parametro {
    TipoParametro tipo;
    char valor[256];
};

#endif // PARAMETRO_H
