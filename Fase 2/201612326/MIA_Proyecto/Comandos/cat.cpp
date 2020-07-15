#include "cat.h"

Cat::Cat()
{

}

int Cat::ejecutarCat(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

    if(!usuario->active){
        std::cout<<"No hay ningun usuario logueado"<<std::endl;
        return 0;
    }

    std::map<TipoParametro, struct parametro> parametros = (*comando.parametros);

    //Validar que el path sea de un archivo
    int esArchivo = false;
    char *aux = parametros[T_FILE].valor;
    while(aux[0] != '\0'){
        if(aux[0] == '.'){
            esArchivo = true;
            break;
        }
        aux++;
    }
    if(!esArchivo){
        std::cout<<"El parametro FILE debe de ser un archivo"<<std::endl;
        return 0;
    }

    //Verificar que la particion existe
    struct partition particion = listaMontaje->getPrimaryMountPartition(usuario->partition_id);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado la particion '"<<usuario->partition_id<<"'";
        return 0;
    }

    //Abrir el archivo
    char *disk_location = listaMontaje->getDiskLocation(usuario->partition_id);
    FILE *archivo = fopen(disk_location,"rb+");


    //Recuperar el superbloque
    struct superBlock *superBloque = new struct superBlock;
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);


    //Buscar el archivo/carpeta en el directorio de archivos y cambiarle permisos
    int resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_FILE].valor,usuario);

    fclose(archivo);

    if(!resultado){
       std::cout<<"El archivo '"<<parametros[T_FILE].valor<<"' no se ha podido leer"<<std::endl;
       return 0;
    }

    return 1;
}

int Cat::visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path, struct user* usuario){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    path++;

    int encontrado = 0;
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                encontrado = visitar_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, path, usuario);
            } else if(i < 12){
                encontrado = visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, path, usuario);
            } else if(i == 12){
                encontrado = visitar_Puntero_Simple(inodo->i_block[i], inodo->i_type, superBloque, archivo, path, usuario);
            } else if(i == 13){
                encontrado = visitar_Puntero_Doble(inodo->i_block[i], inodo->i_type, superBloque, archivo, path, usuario);
            } else if(i == 14){
                encontrado = visitar_Puntero_Triple(inodo->i_block[i], inodo->i_type, superBloque, archivo, path, usuario);
            }

            if(encontrado == 1)
                return 1; //Se cambiaron los permisos correctamente

            if(encontrado == 0)
                continue; //No se ha hallado el archivo, continuear buscando
        }
    }
    std::cout<<"No se encontro el archivo/carpeta '"<<path<<"'"<<std::endl;
    return 0;
}

int Cat::visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path, struct user* usuario){
    struct folderBlock *folder = new struct folderBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
    fread(folder,sizeof(folderBlock),1,archivo);

    //Setear el valor de inicio dependiendo de ignorar o no los primeros registros del folder
    int inicio;
    switch(condicion){
    case NO_IGNORAR_PRIMEROS_REGISTROS:
        inicio = 0;
        break;
    case IGNORAR_PRIMEROS_REGISTROS:
        inicio = 2;
        break;
    }

    //Recorrer el folderBlock
    for(int i = inicio; i < 4; i++){
        if(folder->b_content[i].b_inode == -1){
            continue;
        } else {
            if(startsWith(path,folder->b_content[i].b_name)){

                if(isFile(path)){
                    //Archivo
                    leer_Inodo(folder->b_content[i].b_inode,superBloque,archivo,usuario);
                    return 1;
                } else {
                    //Carpeta
                    while(path[0] != '/')
                        path++;

                    return visitar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,path,usuario);
                }
            }
        }
    }
    return 0;
}

int Cat::visitar_Puntero_Simple(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, bloquePunteros->b_pointers[i],i_type,superBloque,archivo,path,usuario)){
            return 1;
        }
    }
    return 0;
}

int Cat::visitar_Puntero_Doble(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Simple(bloquePunteros->b_pointers[i],i_type,superBloque,archivo,path,usuario)){
            return 1;
        }
    }
    return 0;
}

int Cat::visitar_Puntero_Triple(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Doble(bloquePunteros->b_pointers[i],i_type,superBloque,archivo,path,usuario)){
            return 1;
        }
    }
    return 0;
}

void Cat::leer_Inodo(int index_inode,struct superBlock *superBloque, FILE *archivo, struct user *usuario){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + index_inode * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    //CORREGIR: Tomar en cuenta le usuario para los permisos de lectura

    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                leer_Bloque_Carpeta_Archivo(inodo->i_block[i], superBloque, archivo);
            } else if(i < 12){
                leer_Bloque_Carpeta_Archivo(inodo->i_block[i], superBloque, archivo);
            } else if(i == 12){
                leer_Puntero_Simple(inodo->i_block[i], superBloque, archivo);
            } else if(i == 13){
                leer_Puntero_Doble(inodo->i_block[i], superBloque, archivo);
            } else if(i == 14){
                leer_Puntero_Triple(inodo->i_block[i], superBloque, archivo);
            }
        }
    }
    std::cout<<std::endl;
}

void Cat::leer_Bloque_Carpeta_Archivo(int indice_bloque, struct superBlock *superBloque, FILE *archivo){

    struct fileBlock *file = new struct fileBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
    fread(file,sizeof(folderBlock),1,archivo);

    for(int i = 0; i < 64; i++){
        if(file->b_content[i] == '\0'){
            break;
        }
        std::cout<<file->b_content[i];
    }
}

void Cat::leer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        leer_Bloque_Carpeta_Archivo(bloquePunteros->b_pointers[i],superBloque,archivo);
    }
}

void Cat::leer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        leer_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo);
    }
}

void Cat::leer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        leer_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo);
    }
}


int Cat::startsWith(char *pathCompleto, char *fileFolderName){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == *fileFolderName){
            pathCompleto++;
            fileFolderName++;
        } else {
            return 0;
        }
    }
    return 1;
}

int Cat::isFile(char *pathCompleto){
    while(*pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

