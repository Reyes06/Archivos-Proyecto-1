#ifndef CHGRP_H
#define CHGRP_H

#include <stdlib.h>
#include <iostream>
#include "Estructuras/user.h"
#include "Estructuras/superBlock.h"
#include "Estructuras/inodeTable.h"
#include "Estructuras/fileBlock.h"
#include "Estructuras/folderBlock.h"
#include "Estructuras/pointersBlock.h"
#include "ListaMontaje/ListaMontaje.h"
#include "Analizador/comando.h"

class Chgrp
{
private:
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    enum Opcion{
        BUSCAR_USUARIO,
        BUSCAR_GRUPO,
        OBTENER_ID
    };
    Chgrp();
    static int change_User_Group(char *userName, char *groupName, struct superBlock *superBloque, FILE *archivo, struct user *usuario);
    static int modificar_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, int &contador_registros);
    static int modificar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, int &contador_registros);
    static int modificar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, int &contador_registros);
    static int modificar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, int &contador_registros);

    static int isGroup(char *buffer);
    static int existsGroup(char *buffer, char *newNameGroup);
    static int isUser(char *buffer);
    static int existsUser(char *buffer, char *newNameGroup);

    static int searchUserId(char *userName, struct superBlock *superBloque, FILE *archivo);
    static int searchGroupId(char *userName, struct superBlock *superBloque, FILE *archivo);
    static int buscar_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op);
    static int buscar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op);
    static int buscar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op);
    static int buscar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op);

public:
    static int ejecutarChgrp(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // CHGRP_H
