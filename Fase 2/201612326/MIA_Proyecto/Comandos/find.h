#ifndef FIND_H
#define FIND_H

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

class Find
{
private:
    enum Ignorar{
        IGNORAR_PRIMEROS_REGISTROS,
        NO_IGNORAR_PRIMEROS_REGISTROS
    };
    Find();
    static int buscar_Inicio_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador);
    static int buscar_Inicio_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador);
    static int buscar_Inicio_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador);
    static int buscar_Inicio_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador);
    static int buscar_Inicio_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador);

    static int buscar_Fin_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *fileFolderName, std::string &bufferSalida, int contador);
    static int buscar_Fin_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *fileFolderName,std::string &bufferSalida,  int contador);
    static int buscar_Fin_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *fileFolderName, std::string &bufferSalida, int contador);
    static int buscar_Fin_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *fileFolderName, std::string &bufferSalida, int contador);
    static int buscar_Fin_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *fileFolderName, std::string &bufferSalida, int contador);

    static int isFile(char *pathCompleto);
    static int startsWith(char *pathCompleto, char *fileFolderName);
    static int isEndOfPath(char *pathCompleto);

public:
    static int ejecutarFind(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // FIND_H
