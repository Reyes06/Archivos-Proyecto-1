#ifndef LISTAMONTAJE_H
#define LISTAMONTAJE_H

#include "nodoDisco.h"
#include <iostream>
#include <string.h>
#include "Estructuras/mbr.h"

class ListaMontaje
{
public:
    ListaMontaje();
    int montarParticion(char* disk_location, struct partition partition_name);
    int desmontarParticion(char* id);
    char* getPath(char* id);
    int estaMontado(char *disk_location, char* part_name);
    struct partition* getPartition(char *id);
    std::string getDot();

private:
    struct nodoDisco *cabeceraDiscos;
    std::string getDotPartition(struct nodoParticion *particion, char disk_letter);
};

#endif // LISTAMONTAJE_H
