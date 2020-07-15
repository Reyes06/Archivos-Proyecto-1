#include "ren.h"

Ren::Ren()
{

}

int Ren::ejecutarRen(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

    if(!usuario->active){
        std::cout<<"No hay ningun usuario logueado"<<std::endl;
        return 0;
    }

    std::map<TipoParametro, struct parametro> parametros = (*comando.parametros);

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


    //Buscar el archivo en el directorio de archivos y Renar su contenido
    int resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_NAME].valor,usuario);

    fclose(archivo);

    if(resultado == 1){
       std::cout<<"Archivo renombrado correctamente"<<std::endl;
       return 1;
    } else {
        std::cout<<"El archivo no se ha renombrado"<<std::endl;
        return 0;
    }
}

int Ren::visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *path, char *newName, struct user* usuario){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    path++;

    int permisos_cambiados = 0;
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                permisos_cambiados = visitar_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, path,newName, usuario);
            } else if(i < 12){
                permisos_cambiados = visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, path,newName, usuario);
            } else if(i == 12){
                permisos_cambiados = visitar_Puntero_Simple(inodo->i_block[i], superBloque, archivo, path,newName, usuario);
            } else if(i == 13){
                permisos_cambiados = visitar_Puntero_Doble(inodo->i_block[i], superBloque, archivo, path,newName, usuario);
            } else if(i == 14){
                permisos_cambiados = visitar_Puntero_Triple(inodo->i_block[i], superBloque, archivo, path,newName, usuario);
            }

            if(permisos_cambiados == 1)
                return 1; //Se cambiaron los permisos correctamente

            if(permisos_cambiados == -1)
                return -1; //El usuario no es el propietario del archivo/carpeta, error

            if(permisos_cambiados == 0)
                continue; //No se ha hallado el archivo,newNameinuear buscando
        }
    }
    std::cout<<"No se encontro el archivo '"<<path<<"'"<<std::endl;
    return 0;
}

int Ren::visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque,  struct superBlock *superBloque, FILE *archivo,  char *path, char *newName, struct user* usuario){
    struct folderBlock *folder = new struct folderBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
    fread(folder,sizeof(folderBlock),1,archivo);

    //Cambiar permisos al archivo o carpeta
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
                    //Verificar que el nuevo nombre sea de un archivo
                    if(!isFile(newName)){
                        std::cout<<"El nuevo nombre no pertenece a un archivo"<<std::endl;
                        return -1;
                    }
                    //Cambiar el nombre al archivo
                    if(verificar_Permisos_Inodo(folder->b_content[i].b_inode,superBloque,archivo,newName,usuario)){

                        strcpy(folder->b_content[i].b_name, newName);
                        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
                        fwrite(folder,sizeof(folderBlock),1,archivo);
                        return 1;

                    } else {
                        std::cout<<"No se tienen permisos de escritura sobre el archivo"<<std::endl;
                        return -1;
                    }
                } else if (isEndOfPath(path)){
                    //Verificar que el nuevo nombre sea de una carpeta
                    if(isFile(newName)){
                        std::cout<<"El nuevo nombre no pertenece a una carpeta"<<std::endl;
                        return -1;
                    }
                    //Cambiar el nombre a la carpeta
                    if(verificar_Permisos_Inodo(folder->b_content[i].b_inode,superBloque,archivo,newName,usuario)){

                        strcpy(folder->b_content[i].b_name, newName);
                        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
                        fwrite(folder,sizeof(folderBlock),1,archivo);
                        return 1;

                    } else {
                        std::cout<<"No se tienen permisos de escritura sobre el archivo"<<std::endl;
                        return -1;
                    }
                }else {
                    //Visitar carpeta
                    while(path[0] != '/')
                        path++;

                    return visitar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,path,newName,usuario);

                }
            }
        }
    }
    return 0;
}

int Ren::visitar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *newName, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        int res = visitar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,path,newName, usuario);
        if(res != 0){
            return res;
        }
    }
    return 0;
}

int Ren::visitar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *newName, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }

        int res = visitar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,path,newName, usuario);
        if(res != 0){
            return res;
        }
    }
    return 0;
}

int Ren::visitar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *newName, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        int res = visitar_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,path,newName, usuario);
        if(res != 0){
            return res;
        }
    }
    return 0;
}

int Ren::verificar_Permisos_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *cont, struct user* usuario){

    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    //CORREGIR: Verificar permisos

    return 1;
}

int Ren::isFile(char *pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}


int Ren::isEndOfPath(char *pathCompleto){
    while( *pathCompleto != '/'){
        if(*pathCompleto == '\0'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Ren::startsWith(char *pathCompleto, char *fileFolderName){
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
