#ifndef GETTERS_H
#define GETTERS_H

#include <stdlib.h>
#include <iostream>
#include <string.h>
#include "Estructuras/journal.h"
#include "Estructuras/superBlock.h"
#include "Estructuras/inodeTable.h"
#include "Estructuras/fileBlock.h"
#include "Estructuras/pointersBlock.h"
#include "Estructuras/folderBlock.h"

class Getters
{
public:
    static struct inodeTable *getNewFolderInode(int i_uid, int i_gid, int indice_bloque_hijo);
    static struct inodeTable *getNewFileInode(int i_uid, int i_gid);
    static struct folderBlock *getNewFolderBlock(int inodoActual, int inodoPadre);
    static struct folderBlock *getNewFolderBlock();
    static struct superBlock *getNewSuperBlock(int n, int part_start);
    static struct journal *getNewJournal();
    static int getIndexNewInode(struct superBlock *superBloque, FILE *archivo);
    static int getIndexNewBlock(struct superBlock *superBloque, FILE *archivo);
    static struct pointersBlock *getNewPointersBlock();
    static struct fileBlock *getNewFileBlock();
    Getters();
};


#endif // GETTERS_H
