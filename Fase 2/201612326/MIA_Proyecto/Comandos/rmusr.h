#ifndef RMUSR_H
#define RMUSR_H

#include "Analizador/comando.h"
#include "Estructuras/superBlock.h"
#include "Estructuras/fileBlock.h"
#include "Estructuras/folderBlock.h"
#include "Estructuras/pointersBlock.h"
#include "Estructuras/inodeTable.h"
#include "Estructuras/user.h"
#include "Estructuras/partition.h"
#include "ListaMontaje/ListaMontaje.h"
#include "getters.h"
#include <iostream>
#include <stdlib.h>

class Rmusr
{
private:
    enum Opcion {
        BUSCAR,
        BORRAR_GRUPO
    };
    static int recorrer_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, struct user *usuario, int &gid, Opcion op);
    static int recorrer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, struct user *usuario, int &gid, Opcion op);
    static int recorrer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, struct user *usuario, int &gid, Opcion op);
    static int recorrer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, struct user *usuario, int &gid, Opcion op);
    static int isUser(char *buffer);
    static int existsUser(char *buffer, char *newNameGroup);
    Rmusr();
public:
    static int ejecutarRmusr(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // RMUSR_H
