#ifndef CHOWN_H
#define CHOWN_H

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

class Chown
{
private:
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    enum Opcion{
        BUSCAR,
        OBTENER
    };
    Chown();
    static int visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario);
    static int visitar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario);
    static int visitar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario);
    static int visitar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario);
    static int visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario);

    static int cambiar_Propietario_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario);
    static void cambiar_Permisos_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario);
    static void cambiar_Permisos_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario);
    static void cambiar_Permisos_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario);
    static void cambiar_Permisos_Bloque_Carpeta(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario);

    static int startsWith(char *pathCompleto, char *fileFolderName);
    static int isEndOfPath(char *pathCompleto);
    static int isUser(char *buffer);
    static int existsUser(char *buffer, char *newNameGroup);

    static int searchUserId(char *userName, struct superBlock *superBloque, FILE *archivo);
    static int buscar_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op);
    static int buscar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op);
    static int buscar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op);
    static int buscar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op);

public:
    static int ejecutarChown(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // CHOWN_H
