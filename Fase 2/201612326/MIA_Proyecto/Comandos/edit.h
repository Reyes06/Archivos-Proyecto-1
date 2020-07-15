#ifndef EDIT_H
#define EDIT_H

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

class Edit
{
private:
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    Edit();
    static int visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);
    static int visitar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);
    static int visitar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);
    static int visitar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);
    static int visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario);

    static int editar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *cont, struct user* usuario);
    static int editar_Puntero_Simple(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&cont);
    static int editar_Puntero_Doble(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&cont);
    static int editar_Puntero_Triple(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&cont);
    static int editar_Bloque_Archivo(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&cont);

    static int esta_Lleno_Bloque_Archivo(int indice_bloque_archivo, struct superBlock *superBloque, FILE *archivo);
    static int esta_Lleno_Simple(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo);
    static int esta_Lleno_Doble(int indice_bloque_punteros_doble, struct superBlock *superBloque, FILE *archivo);
    static int startsWith(char *pathCompleto, char *fileFolderName);
    static int isFile(char *pathCompleto);
public:
    static int ejecutarEdit(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // EDIT_H
