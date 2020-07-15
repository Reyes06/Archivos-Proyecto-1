#ifndef LS_H
#define LS_H

#include <time.h>
#include <fstream>
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

class Ls
{
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    enum Opcion{
        BUSCAR_USUARIO,
        BUSCAR_GRUPO
    };
    struct filaLs{
        char permisos[10];
        char owner[15];
        char grupo[15];
        int size;
        char fecha[11];
        char hora[9];
        char tipo[8];
        char name[16];
    };
    Ls();
    static int visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path);
    static int visitar_Puntero_Simple(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path);
    static int visitar_Puntero_Doble(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path);
    static int visitar_Puntero_Triple(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path);
    static int visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path);

    static void leer_Inodo(int index_inode,struct superBlock *superBloque, FILE *archivochar, char *nombreArchivoCarpeta);
    static void leer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo);
    static void leer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo);
    static void leer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo);
    static void leer_Bloque_Carpeta_Archivo(Ignorar ignorar,int indice_bloque, struct superBlock *superBloque, FILE *archivo);

    static int startsWith(char *pathCompleto, char *fileFolderName);
    static int isFile(char *pathCompleto);
    static int verificarSubCadena( char *cadena, char *subcadena );
    static int isGroup(char *buffer);
    static int isUser(char *buffer);
    static int getGroup(char *buffer, char *userGroupName, int userGroupId);
    static int getUser(char *buffer, char *userGroupName, int userGroupId);
    static struct filaLs obtenerRegistroLs(struct superBlock *superBloque, FILE *archivo, struct inodeTable *inodo, char *fileFolderName);

    static int buscar_Usuario_Grupo(struct superBlock *superBloque, FILE *archivo, char *groupUserName, int &userGroupId, Opcion op);
    static int buscar_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupUserName, int &userGroupId, Opcion op);
    static int buscar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupUserName, int &userGroupId, Opcion op);
    static int buscar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupUserName, int &userGroupId, Opcion op);
    static int buscar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupUserName, int &userGroupId, Opcion op);

    static struct filaLs obtenerRegistroLs(struct inodeTable *inodo, char *fileFolderName);

public:
    static int ejecutarLs(struct comando comando, ListaMontaje *listaMontaje);
};

#endif // LS_H
