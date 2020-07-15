#ifndef REN_H
#define REN_H

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
#include "getters.h"

class Ren
{
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    Ren();
    static int visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);
    static int visitar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);
    static int visitar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);
    static int visitar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);
    static int visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);

    static int verificar_Permisos_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *cont, struct user* usuario);

    static int startsWith(char *pathCompleto, char *fileFolderName);
    static int isFile(char *pathCompleto);
    static int isEndOfPath(char *pathCompleto);
public:
    static int ejecutarRen(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // REN_H
