#ifndef MKFILE_H
#define MKFILE_H

#include <string>
#include <fstream>
#include "Analizador/comando.h"
#include "Estructuras/user.h"
#include "Estructuras/superBlock.h"
#include "Estructuras/inodeTable.h"
#include "Estructuras/fileBlock.h"
#include "Estructuras/pointersBlock.h"
#include "Estructuras/folderBlock.h"
#include "ListaMontaje/ListaMontaje.h"
#include "getters.h"
#include <sys/stat.h>

class Mkfile{
private:
    enum Condicion{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };

    Mkfile();
    static int visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, char *fileSize);
    static int visitar_Bloque_Carpeta_Archivo_EXISTE(Condicion condicion, int indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize);
    static int visitar_Puntero_Simple_EXISTE(int indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize);
    static int visitar_Puntero_Doble_EXISTE(int indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize);
    static int visitar_Puntero_Triple_EXISTE(int indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize);

    static int visitar_Bloque_Carpeta_Archivo_CREAR(Condicion condicion, int &indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize, char tipo_inodo);
    static int visitar_Puntero_Simple_CREAR(int &indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize, char tipo_inodo);
    static int visitar_Puntero_Doble_CREAR(int &indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize, char tipo_inodo);
    static int visitar_Puntero_Triple_CREAR(int &indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize, char tipo_inodo);

    static int crear_Bloque_Directo(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer, char tipo_inodo);
    static int crear_Puntero_Simple(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer, char tipo_inodo);
    static int crear_Puntero_Doble(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer, char tipo_inodo);
    static int crear_Puntero_Triple(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer, char tipo_inodo);

    static int startsWith(char *pathCompleto, char *fileFolderName);
    static void removeFolderName(char *&pathCompleto);
    static int isFile(char *pathCompleto);

    static int estaLleno_Bloque_Archivo_Carpeta(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo, char tipo);
    static int estaLleno_Simple(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo, char tipo);
    static int estaLleno_Doble(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo, char tipo);
public:
    static int ejecutarMkfile(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};


#endif // MKFILE_H
