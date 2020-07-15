#ifndef MV_H
#define MV_H

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

class Mv
{
private:
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    Mv();
    static int buscar_Origen_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny);
    static int buscar_Origen_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny);
    static int buscar_Origen_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny);
    static int buscar_Origen_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny);
    static int buscar_Origen_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny);

    static int buscar_Destino_Inodo(int indice_inodo, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo,  char *pathDestiny);
    static int buscar_Destino_Puntero_Simple(int indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo,  char *pathDestiny);
    static int buscar_Destino_Puntero_Doble(int indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo,  char *pathDestiny);
    static int buscar_Destino_Puntero_Triple(int indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo,  char *pathDestiny);
    static int buscar_Destino_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo,  char *pathDestiny);

    static int mover_Archivo_Carpeta_Inodo(int indice_inodo, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo);
    static int mover_Archivo_Carpeta_Puntero_Simple(int &indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo);
    static int mover_Archivo_Carpeta_Puntero_Doble(int &indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo);
    static int mover_Archivo_Carpeta_Puntero_Triple(int &indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo);
    static int mover_Archivo_Carpeta_Bloque_Archivo_Carpeta(Ignorar condicion, int &indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo);

    static int isFile(char *pathCompleto);
    static int startsWith(char *pathCompleto, char *pathDestiny);
    static int isEndOfPath(char *pathCompleto);

public:
    static int ejecutarMv(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // MV_H
