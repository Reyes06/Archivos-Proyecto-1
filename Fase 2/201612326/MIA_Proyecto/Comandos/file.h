#ifndef FILE_H
#define FILE_H

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

class File
{
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    File();
    static int visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path);
    static int visitar_Puntero_Simple(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path);
    static int visitar_Puntero_Doble(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path);
    static int visitar_Puntero_Triple(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path);
    static int visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path);

    static void leer_Inodo(int index_inode,struct superBlock *superBloque, FILE *archivo);
    static void leer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo);
    static void leer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo);
    static void leer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo);
    static void leer_Bloque_Carpeta_Archivo(int indice_bloque, struct superBlock *superBloque, FILE *archivo);

    static int startsWith(char *pathCompleto, char *fileFolderName);
    static int isFile(char *pathCompleto);
    static int verificarSubCadena( char *cadena, char *subcadena );
public:
    static int ejecutarFile(struct comando comando, ListaMontaje *listaMontaje);
};

#endif // FILE_H
