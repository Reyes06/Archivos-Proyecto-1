#ifndef MKDIR_H
#define MKDIR_H

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

class Mkdir
{
private:
    enum Condicion{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };

    Mkdir();
    static int visitar_Inodo(int indice_inodo, int indice_inodo_padre, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, char *fileSize);
    static int visitar_Bloque_Carpeta_Archivo_EXISTE(Condicion condicion, int indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize);
    static int visitar_Puntero_Simple_EXISTE(int indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize);
    static int visitar_Puntero_Doble_EXISTE(int indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize);
    static int visitar_Puntero_Triple_EXISTE(int indice,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize);

    static int visitar_Bloque_Carpeta_Archivo_CREAR(Condicion condicion, int &indice, int indice_inodo_padre,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize);
    static int visitar_Puntero_Simple_CREAR(int &indice, int indice_inodo_padre,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize);
    static int visitar_Puntero_Doble_CREAR(int &indice, int indice_inodo_padre,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize);
    static int visitar_Puntero_Triple_CREAR(int &indice, int indice_inodo_padre,  struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize);

    static int startsWith(char *pathCompleto, char *fileFolderName);
    static void removeFolderName(char *&pathCompleto);
    static int isEndOfPath(char *pathCompleto);
    static int isFile(char *pathCompleto);

    static int estaLleno_Bloque_Carpeta(int indice_bloque_carpeta, struct superBlock *superBloque, FILE *archivo);
    static int estaLleno_Simple(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo);
    static int estaLleno_Doble(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo);
public:
    static int ejecutarMkdir(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // MKDIR_H
