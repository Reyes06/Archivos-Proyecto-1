#ifndef REP_H
#define REP_H

#include "Analizador/comando.h"
#include "Comandos/mount.h"
#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string.h>
#include "ListaMontaje/ListaMontaje.h"
#include "Analizador/parametro.h"
#include "Comandos/mkdisk.h"
#include "ListaDisponibles/listaDisponibles.h"
#include "Estructuras/superBlock.h"
#include "Estructuras/inodeTable.h"
#include "Estructuras/journal.h"
#include "Estructuras/fileBlock.h"
#include "Estructuras/folderBlock.h"
#include "Estructuras/pointersBlock.h"
#include "Comandos/file.h"
#include "Comandos/ls.h"

enum Condicion{
    IGNORAR_PRIMEROS_REGISTROS,
    NO_IGNORAR_PRIMEROS_REGISTROS
};

int ejecutarRep(struct comando comando, ListaMontaje *listaMontaje);
int graficarMbr(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje);
int graficarDisk(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje);

int graficarListaMontaje(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje);
int verificarSubCadena( char *cadena, char *subcadena );
int graficarInode(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje);
int graficarBmInode(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje);
int graficarBmBlock(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje);
int graficarSb(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje);
int graficarJournaling(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje);
int graficarTree(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje);
int graficarBlock(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje);

void visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group);
void visitar_Puntero_Simple(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group);
void visitar_Puntero_Doble(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group);
void visitar_Puntero_Triple(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group);
void visitar_Bloque_Carpeta_Archivo(Condicion condicion, int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group);

void visitar_Inodo_BLOCK(int indice_inodo, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs);
void visitar_Bloque_Carpeta_Archivo_BLOCK(Condicion condicion, int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs);
void visitar_Puntero_Simple_BLOCK(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs);
void visitar_Puntero_Doble_BLOCK(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs);
void visitar_Puntero_Triple_BLOCK(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs);

#endif // REP_H
