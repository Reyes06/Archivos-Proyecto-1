#ifndef CP_H
#define CP_H

#include <stdlib.h>
#include <iostream>
#include "Comandos/mkdir.h"
#include "Comandos/mkfile.h"
#include "Estructuras/user.h"
#include "Estructuras/superBlock.h"
#include "Estructuras/inodeTable.h"
#include "Estructuras/fileBlock.h"
#include "Estructuras/folderBlock.h"
#include "Estructuras/pointersBlock.h"
#include "ListaMontaje/ListaMontaje.h"
#include "Analizador/comando.h"
#include "getters.h"

class Cp
{
private:
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    Cp();
    static int buscar_Origen_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);
    static int buscar_Origen_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);
    static int buscar_Origen_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);
    static int buscar_Origen_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);
    static int buscar_Origen_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);

    static void copiar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);
    static void copiar_Puntero_Simple(int indice_bloque, char tipo_inodo, struct superBlock *superBloque, FILE *archivo, char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);
    static void copiar_Puntero_Doble(int indice_bloque, char tipo_inodo, struct superBlock *superBloque, FILE *archivo, char *pathActual, const  char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);
    static void copiar_Puntero_Triple(int indice_bloque, char tipo_inodo, struct superBlock *superBloque, FILE *archivo, char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);
    static void copiar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, char tipo_inodo, struct superBlock *superBloque, FILE *archivo, char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario);

    static int isFile(char *pathCompleto);
    static int startsWith(char *pathCompleto, char *pathDestiny);
    static int isEndOfPath(char *pathCompleto);

public:
    static int ejecutarCp(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // CP_H
