#ifndef MKUSR_H
#define MKUSR_H

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

class Mkusr
{
private:
    enum Opcion{
        BUSCAR_USUARIO,
        BUSCAR_GRUPO
    };
    static int recorrer_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char* newGroupName, int &groupCount, Opcion op);
    static int recorrer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char* newGroupName, int &groupCount, Opcion op);
    static int recorrer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char* newGroupName, int &groupCount, Opcion op);
    static int recorrer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char* newGroupName, int &groupCount, Opcion op);


    static int existsUser(char *buffer, char *newNameGroup);
    static int isUser(char *buffer);
    static int existsGroup(char *buffer, char *newNameGroup);
    static int isGroup(char *buffer);

    static int crear_Bloque_Directo(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer);
    static int crear_Puntero_Simple(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer);
    static int crear_Puntero_Doble(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer);
    static int crear_Puntero_Triple(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer);

    static int esta_Lleno_Bloque_Archivo(int indice_bloque_archivo, struct superBlock *superBloque, FILE *archivo);
    static int esta_Lleno_Apuntador_Simple(int indice_bloque_archivo, struct superBlock *superBloque, FILE *archivo);
    static int esta_Lleno_Apuntador_Doble(int indice_bloque_archivo, struct superBlock *superBloque, FILE *archivo);
    static int esta_Lleno_Apuntador_Triple(int indice_bloque_archivo, struct superBlock *superBloque, FILE *archivo);
public:
    Mkusr();
    static int ejecutarMkusr(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);

};

#endif // MKUSR_H
