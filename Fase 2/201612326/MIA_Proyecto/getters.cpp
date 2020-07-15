#include "getters.h"

Getters::Getters(){

}

struct inodeTable *Getters::getNewFolderInode(int i_uid, int i_gid, int indice_bloque_hijo){

    struct inodeTable *inodo = new struct inodeTable;
    inodo->i_uid = i_uid;
    inodo->i_gid = i_gid;
    inodo->i_size = 0;
    time_t hora = time(0);
    inodo->i_atime = *localtime(&hora);
    inodo->i_ctime = *localtime(&hora);
    inodo->i_mtime = *localtime(&hora);
    inodo->i_block[0] = indice_bloque_hijo;
    for(int i=1; i<15; i++){
        inodo->i_block[i] = -1;
    }
    inodo->i_type = 0;
    inodo->i_perm = 644;

    return inodo;
}

struct inodeTable *Getters::getNewFileInode(int i_uid, int i_gid){

    struct inodeTable *inodo = new struct inodeTable;
    inodo->i_uid = i_uid;
    inodo->i_gid = i_gid;
    inodo->i_size = 0;
    time_t hora = time(0);
    inodo->i_atime = *localtime(&hora);
    inodo->i_ctime = *localtime(&hora);
    inodo->i_mtime = *localtime(&hora);
    for(int i=0; i<15; i++){
        inodo->i_block[i] = -1;
    }
    inodo->i_type = 1;
    inodo->i_perm = 644;

    return inodo;
}

struct folderBlock *Getters::getNewFolderBlock(int inodoActual, int inodoPadre){
    struct folderBlock *carpeta = new struct folderBlock;

    //Setear el actual
    strcpy(carpeta->b_content[0].b_name,".");
    carpeta->b_content[0].b_inode = inodoActual;

    //Setear el padre
    strcpy(carpeta->b_content[1].b_name,"..");
    carpeta->b_content[1].b_inode = inodoPadre;

    for(int i = 2; i < 4; i++){
        strcpy(carpeta->b_content[i].b_name," ");
        carpeta->b_content[i].b_inode = -1;
    }

    return carpeta;
}

struct folderBlock *Getters::getNewFolderBlock(){
    struct folderBlock *carpeta = new struct folderBlock;

    for(int i = 0; i < 4; i++){
        strcpy(carpeta->b_content[i].b_name,"...");
        carpeta->b_content[i].b_inode = -1;
    }

    return carpeta;
}

struct fileBlock *Getters::getNewFileBlock(){
    struct fileBlock *file = new struct fileBlock;
    file->b_content[0] = '\0';
    return file;
}

struct pointersBlock *Getters::getNewPointersBlock(){
    struct pointersBlock *punteros = new struct pointersBlock;
    for(int i = 0; i < 16; i++){
        punteros->b_pointers[i] = -1;
    }
    return punteros;
}

struct superBlock *Getters::getNewSuperBlock(int n, int part_start){
    struct superBlock *superBloque = new struct superBlock;
    superBloque->s_filesystem_type = 2;
    superBloque->s_inodes_count = n;
    superBloque->s_blocks_count = 3*n;
    superBloque->s_free_inodes_count = n;
    superBloque->s_free_blocks_count = 3*n;
    time_t hora = time(0);
    superBloque->s_mtime = *localtime(&hora);
    superBloque->s_umtime = *localtime(&hora);
    superBloque->s_mnt_count = 0;
    superBloque->s_magic = 0xEF53;
    superBloque->s_inode_size = sizeof(inodeTable);
    superBloque->s_block_size = sizeof(fileBlock);
    superBloque->s_first_blo = 0;
    superBloque->s_first_ino = 0;
    superBloque->s_bm_inode_start = part_start + sizeof(superBlock) + n*sizeof(journal);
    superBloque->s_bm_block_start = part_start + sizeof(superBlock) + n*sizeof(journal) + n;
    superBloque->s_inode_start    = part_start + sizeof(superBlock) + n*sizeof(journal) + n + 3*n;
    superBloque->s_block_start    = part_start + sizeof(superBlock) + n*sizeof(journal) + n + 3*n + n*sizeof(inodeTable);

    return superBloque;
}

struct journal *Getters::getNewJournal(){
    struct journal *journal = new struct journal;
    journal->active = 0;
    return journal;
}

int Getters::getIndexNewInode(struct superBlock *superBloque, FILE *archivo){
    //El archivo debe de estar abierto
    char inode_status;
    int contador = 0;
    fseek(archivo,superBloque->s_bm_inode_start,SEEK_SET);
    fread(&inode_status,sizeof(char),1,archivo);

    while(contador < superBloque->s_inodes_count){
        if(inode_status == 0){
            return contador;
        }
        fread(&inode_status,sizeof(char),1,archivo);
        contador++;
    }
    return -1;
}

int Getters::getIndexNewBlock(struct superBlock *superBloque, FILE *archivo){
    //El archivo debe de estar abierto
    char block_status;
    int contador = 0;
    fseek(archivo,superBloque->s_bm_block_start,SEEK_SET);
    fread(&block_status,sizeof(char),1,archivo);

    while(contador < superBloque->s_blocks_count){
        if(block_status == 0){
            return contador;
        }
        fread(&block_status,sizeof(char),1,archivo);
        contador++;
    }
    return -1;
}
