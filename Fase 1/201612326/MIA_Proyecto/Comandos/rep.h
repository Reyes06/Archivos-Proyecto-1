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

int ejecutarRep(struct comando comando, ListaMontaje *listaMontaje);
int graficarMbr(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje);
int graficarDisk(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje);

int graficarListaMontaje(ListaMontaje *listaMontaje);
int verificarSubCadena( char *cadena, char *subcadena );


#endif // REP_H
