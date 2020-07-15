#ifndef LISTAMONTAJE_H
#define LISTAMONTAJE_H

#include "Estructuras/superBlock.h"
#include "Estructuras/mbr.h"
#include <stdlib.h>
#include <time.h>
#include "nodoDisco.h"
#include <iostream>
#include <string.h>
#include "Estructuras/mbr.h"

class ListaMontaje
{
public:
    ListaMontaje();
    int montarParticion(char* disk_location, char* part_name, char part_type);
    int desmontarParticion(char* id);
    char* getDiskLocation(char* id);
    char* getPartName(char *id);
    int estaMontado(char *disk_location, char* part_name);
    struct partition getPrimaryMountPartition(char *id);
    std::string getDot();

private:
    struct nodoDisco *cabeceraDiscos;
    std::string getDotPartition(struct nodoParticion *particion, char disk_letter);
};

#endif // LISTAMONTAJE_H
