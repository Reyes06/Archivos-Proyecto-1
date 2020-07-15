#ifndef CHMOD_H
#define CHMOD_H

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

class Chmod
{
private:
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    Chmod();
    static int visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario);
    static int visitar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario);
    static int visitar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario);
    static int visitar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario);
    static int visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario);

    static int cambiar_Permisos_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario);
    static void cambiar_Permisos_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario);
    static void cambiar_Permisos_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario);
    static void cambiar_Permisos_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario);
    static void cambiar_Permisos_Bloque_Carpeta(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario);


    static int startsWith(char *pathCompleto, char *fileFolderName);
    static int isEndOfPath(char *pathCompleto);
public:
    static int ejecutarChmod(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // CHMOD_H
