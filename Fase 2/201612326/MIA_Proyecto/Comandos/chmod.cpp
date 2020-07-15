#include "chmod.h"

Chmod::Chmod()
{

}

int Chmod::ejecutarChmod(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

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

    //Validar los permisos
    if(strlen(parametros[T_UGO].valor) == 3){
        if(parametros[T_UGO].valor[0] > '7' || parametros[T_UGO].valor[1] > '7' || parametros[T_UGO].valor[2] > '7'){
            std::cout<<"El numero maximo en los permisos es 7"<<std::endl;
            return 0;
        }
        if(parametros[T_UGO].valor[0] < '0' || parametros[T_UGO].valor[1] < '0' || parametros[T_UGO].valor[2] < '0'){
            std::cout<<"El numero minimo en los permisos es 0"<<std::endl;
            return 0;
        }
    } else {
        std::cout<<"Los permisos deben ser 3 numeros"<<std::endl;
        return 0;
    }

    //Buscar el archivo/carpeta en el directorio de archivos y cambiarle permisos
    int resultado;
    if(parametros[T_R].valor[0] == '1'){
        resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,1,atoi(parametros[T_UGO].valor),usuario);
    } else {
        resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,0,atoi(parametros[T_UGO].valor),usuario);
    }

    fclose(archivo);

    if(resultado){
       std::cout<<"Permisos cambiados correctamente"<<std::endl;
       return 1;
    } else {
        std::cout<<"Los permisos no se han cambiado"<<std::endl;
        return 0;
    }
}

int Chmod::visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    path++;

    //Verificar si lo se desea es cambiar los permisos de la carpeta actual
    if(path[0] == '\0'){
        if(cambiar_Permisos_Inodo(indice_inodo,superBloque,archivo,recursive,permisos, usuario)){
            return 1;
        } else {
            std::cout<<"El usuario '"<<usuario->usr<<"' no es el propietario de esta carpeta"<<std::endl;
            return -1;
        }
    }

    int permisos_cambiados = 0;
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                permisos_cambiados = visitar_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, path, recursive, permisos, usuario);
            } else if(i < 12){
                permisos_cambiados = visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, path, recursive, permisos, usuario);
            } else if(i == 12){
                permisos_cambiados = visitar_Puntero_Simple(inodo->i_block[i], superBloque, archivo, path, recursive, permisos, usuario);
            } else if(i == 13){
                permisos_cambiados = visitar_Puntero_Doble(inodo->i_block[i], superBloque, archivo, path, recursive, permisos, usuario);
            } else if(i == 14){
                permisos_cambiados = visitar_Puntero_Triple(inodo->i_block[i], superBloque, archivo, path, recursive, permisos, usuario);
            }

            if(permisos_cambiados == 1)
                return 1; //Se cambiaron los permisos correctamente

            if(permisos_cambiados == -1)
                return -1; //El usuario no es el propietario del archivo/carpeta, error

            if(permisos_cambiados == 0)
                continue; //No se ha hallado el archivo, continuear buscando
        }
    }
    std::cout<<"No se encontro el archivo '"<<path<<"'"<<std::endl;
    return 0;
}

int Chmod::visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario){
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
                if(isEndOfPath(path)){
                    //La carpeta actual es la ultima en el 'path'. Cambiar permisos
                    if(cambiar_Permisos_Inodo(folder->b_content[i].b_inode,superBloque,archivo,recursive,permisos, usuario)){
                        return 1;
                    } else {
                        std::cout<<"El usuario '"<<usuario->usr<<"' no es el propietario de '"<<folder->b_content[i].b_name<<"'"<<std::endl;
                        return -1;
                    }
                } else {
                    //La carpeta actual no es la ultima del 'path'. Recorrer sub-carpeta
                    while(path[0] != '/'){
                        path++;
                    }
                    return visitar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,path,recursive,permisos, usuario);
                }
            }
        }
    }
    return 0;
}

int Chmod::visitar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        int res = visitar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,path,recursive,permisos, usuario);

        if(res == 0)
            continue;
        else
            return res;

    }
    return 0;
}

int Chmod::visitar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        int res = visitar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,path,recursive,permisos, usuario);

        if(res == 0)
            continue;
        else
            return res;

    }
    return 0;
}

int Chmod::visitar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, int permisos, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        int res = visitar_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,path,recursive,permisos, usuario);

        if(res == 0)
            continue;
        else
            return res;

    }
    return 0;
}

int Chmod::cambiar_Permisos_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    if(usuario->uid == 1 || inodo->i_uid == usuario->uid){   //usuario->uid == 1 ->USUARIO ROOT

        //Cambiar permisos
        inodo->i_perm = permisos;
        fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
        fwrite(inodo,sizeof(inodeTable),1,archivo);

        //Cambiar permisos de forma recursiva
        if(recursive == 1 && inodo->i_type != 1){
            for(int i = 0; i < 15; i++) {
                if(inodo->i_block[i] == -1){
                    continue;
                } else {
                    if(i == 0){
                        cambiar_Permisos_Bloque_Carpeta(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, recursive, permisos, usuario);
                    } else if(i < 12){
                        cambiar_Permisos_Bloque_Carpeta(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, recursive, permisos, usuario);
                    } else if(i == 12){
                        cambiar_Permisos_Puntero_Simple(inodo->i_block[i], superBloque, archivo, recursive, permisos, usuario);
                    } else if(i == 13){
                        cambiar_Permisos_Puntero_Doble(inodo->i_block[i], superBloque, archivo, recursive, permisos, usuario);
                    } else if(i == 14){
                        cambiar_Permisos_Puntero_Triple(inodo->i_block[i], superBloque, archivo, recursive, permisos, usuario);
                    }
                }
            }
        }
        //Completado correctamente
        return 1;

    } else {
        //No se cuenta con permisos
        return 0;
    }
}

void Chmod::cambiar_Permisos_Bloque_Carpeta(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario){
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
            cambiar_Permisos_Inodo(folder->b_content[i].b_inode,superBloque,archivo,recursive,permisos,usuario);
        }
    }

}

void Chmod::cambiar_Permisos_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        cambiar_Permisos_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,recursive,permisos, usuario);
    }
}

void Chmod::cambiar_Permisos_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, int permisos, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        cambiar_Permisos_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,recursive,permisos, usuario);
    }
}

void Chmod::cambiar_Permisos_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,   int recursive, int permisos, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        cambiar_Permisos_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,recursive,permisos, usuario);
    }
}


int Chmod::startsWith(char *pathCompleto, char *fileFolderName){
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

int Chmod::isEndOfPath(char *pathCompleto){
    while( *pathCompleto != '/'){
        if(*pathCompleto == '\0'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}
