#include "chown.h"

Chown::Chown()
{

}

int Chown::ejecutarChown(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

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

    //Buscar el archivo/carpeta en el directorio de archivos y cambiarle permisos
    int resultado;
    if(parametros[T_R].valor[0] == '1'){
        resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,1,parametros[T_USR].valor,usuario);
    } else {
        resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,0,parametros[T_USR].valor,usuario);
    }

    fclose(archivo);

    if(resultado == 1){
       std::cout<<"Permisos cambiados correctamente"<<std::endl;
       return 1;
    } else {
        std::cout<<"Los permisos no se han cambiado"<<std::endl;
        return 0;
    }
}

int Chown::visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    path++;

    //Verificar si lo se desea es cambiar los permisos de la carpeta actual
    if(path[0] == '\0'){
        if(cambiar_Propietario_Inodo(indice_inodo,superBloque,archivo,recursive,newOwn, usuario)){
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
                permisos_cambiados = visitar_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, path, recursive, newOwn, usuario);
            } else if(i < 12){
                permisos_cambiados = visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, path, recursive, newOwn, usuario);
            } else if(i == 12){
                permisos_cambiados = visitar_Puntero_Simple(inodo->i_block[i], superBloque, archivo, path, recursive, newOwn, usuario);
            } else if(i == 13){
                permisos_cambiados = visitar_Puntero_Doble(inodo->i_block[i], superBloque, archivo, path, recursive, newOwn, usuario);
            } else if(i == 14){
                permisos_cambiados = visitar_Puntero_Triple(inodo->i_block[i], superBloque, archivo, path, recursive, newOwn, usuario);
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

int Chown::visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario){
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
                    if(cambiar_Propietario_Inodo(folder->b_content[i].b_inode,superBloque,archivo,recursive,newOwn, usuario)){
                        return 1;
                    } else {
                        return -1;
                    }
                } else {
                    //La carpeta actual no es la ultima del 'path'. Recorrer sub-carpeta
                    while(path[0] != '/'){
                        path++;
                    }
                    return visitar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,path,recursive,newOwn, usuario);
                }
            }
        }
    }
    return 0;
}

int Chown::visitar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,path,recursive,newOwn, usuario)){
            return 1;
        }
    }
    return 0;
}

int Chown::visitar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,path,recursive,newOwn, usuario)){
            return 1;
        }
    }
    return 0;
}

int Chown::visitar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, int recursive, char *newOwn, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,path,recursive,newOwn, usuario)){
            return 1;
        }
    }
    return 0;
}

int Chown::cambiar_Propietario_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    if(usuario->uid == 1 || inodo->i_uid == usuario->uid){   //usuario->uid == 1 ->USUARIO ROOT

        //Cambiar propietario
        int newUserId = searchUserId(newOwn,superBloque,archivo);
        if(newUserId == 0){
            std::cout<<"El usuario '"<<newOwn<<"' no existe"<<std::endl;
            return 0;
        }

        inodo->i_uid = newUserId;
        fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
        fwrite(inodo,sizeof(inodeTable),1,archivo);

        //Cambiar permisos de forma recursiva
        if(recursive == 1 && inodo->i_type != 1){
            for(int i = 0; i < 15; i++) {
                if(inodo->i_block[i] == -1){
                    continue;
                } else {
                    if(i == 0){
                        cambiar_Permisos_Bloque_Carpeta(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, recursive, newOwn, usuario);
                    } else if(i < 12){
                        cambiar_Permisos_Bloque_Carpeta(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, recursive, newOwn, usuario);
                    } else if(i == 12){
                        cambiar_Permisos_Puntero_Simple(inodo->i_block[i], superBloque, archivo, recursive, newOwn, usuario);
                    } else if(i == 13){
                        cambiar_Permisos_Puntero_Doble(inodo->i_block[i], superBloque, archivo, recursive, newOwn, usuario);
                    } else if(i == 14){
                        cambiar_Permisos_Puntero_Triple(inodo->i_block[i], superBloque, archivo, recursive, newOwn, usuario);
                    }
                }
            }
        }
        //Completado correctamente
        return 1;

    }

    //No se cuenta con permisos
    return 0;
}

void Chown::cambiar_Permisos_Bloque_Carpeta(Ignorar condicion, int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario){
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
            cambiar_Propietario_Inodo(folder->b_content[i].b_inode,superBloque,archivo,recursive,newOwn,usuario);
        }
    }

}

void Chown::cambiar_Permisos_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        cambiar_Permisos_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,recursive,newOwn, usuario);
    }
}

void Chown::cambiar_Permisos_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, int recursive, char *newOwn, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        cambiar_Permisos_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,recursive,newOwn, usuario);
    }
}

void Chown::cambiar_Permisos_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,   int recursive, char *newOwn, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        cambiar_Permisos_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,recursive,newOwn, usuario);
    }
}


int Chown::startsWith(char *pathCompleto, char *fileFolderName){
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

int Chown::isEndOfPath(char *pathCompleto){
    while( *pathCompleto != '/'){
        if(*pathCompleto == '\0'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Chown::searchUserId(char *userName, struct superBlock *superBloque, FILE *archivo){

    //Recuperar el inodo del archivo users.txt
    struct inodeTable *inodeUsers = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
    fread(inodeUsers,sizeof(inodeTable),1,archivo);

    //Buscar el id del grupo a eliminar
    char *buffer = new char[100];
    buffer[0] = '\0';
    int contador_registros = 0;
    int encontrado = 0;
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(buscar_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 12){
                if(buscar_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 13){
                if(buscar_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 14){
                if(buscar_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR)){
                    encontrado = 1;
                    break;
                }
            }
        }
    }

    if(!encontrado){
        return 0;
    }

    //Buscar el id del grupo a eliminar
    buffer = new char[100];
    buffer[0] = '\0';
    int uid;
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(uid = buscar_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, OBTENER)){
                    return uid;
                }
            } else if (i == 12){
                if(uid = buscar_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, OBTENER)){
                    return uid;
                }
            } else if (i == 13){
                if(uid = buscar_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, OBTENER)){
                    return uid;
                }
            } else if (i == 14){
                if(uid = buscar_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, OBTENER)){
                    return uid;
                }
            }
        }
    }

    return 0;
}

int Chown::buscar_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op){
    //Recuperar el fileBlock
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
    fread(file,sizeof(fileBlock),1,archivo);

    switch(op){
    case Chown::BUSCAR:
        {
            int primer_caracter_libre = strlen(buffer);
            for(size_t i = 0; i < 64;i++){
                if(file->b_content[i] == '\0')
                    break;

                if(file->b_content[i] == '\n'){


                        buffer[primer_caracter_libre] = '\n';
                        buffer[primer_caracter_libre+1] = '\0';


                        if(isUser(buffer)){
                            if(existsUser(buffer, userName)){

                                //Grupo encontrado
                                return 1;
                            }

                        }

                        contador_registros++;
                        buffer[0] = '\0';
                        primer_caracter_libre = 0;

                } else {
                    //Concatenar al buffer
                    buffer[primer_caracter_libre] = file->b_content[i];
                    primer_caracter_libre++;
                }
            }

            buffer[primer_caracter_libre] = '\0';
            return 0;
        }
        break;
    case Chown::OBTENER:
        {
            int primer_caracter_libre = strlen(buffer);
            for(size_t i = 0; i < 64;i++){
                if(file->b_content[i] == '\0')
                    break;

                if(contador_registros == 0){



                    char id[5]; id[0] = '\0';
                    int contador = 0;
                    while(file->b_content[i] != ','){
                        id[contador] = file->b_content[i];
                        i++;
                        contador++;
                    }
                    return atoi(id);




                }
                if(file->b_content[i] == '\n'){
                    contador_registros--;
                }
            }

            buffer[primer_caracter_libre] = '\0';
            return 0;
        }
        break;

    }



}

int Chown::buscar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op){

    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(buscar_Directo(pointers->b_pointers[i],superBloque,archivo,buffer,userName,contador_registros,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Chown::buscar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(buscar_Puntero_Simple(pointers->b_pointers[i],superBloque,archivo,buffer,userName,contador_registros,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Chown::buscar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(buscar_Puntero_Doble(pointers->b_pointers[i],superBloque,archivo,buffer,userName,contador_registros,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Chown::isUser(char *buffer){
    if(buffer[0] == '0')
        return 0;

    char *aux = &buffer[0];
    while(*aux != ','){
        aux++;
    }
    aux++;

    //Obtener el tipo del usuario/grupo
    char tipo = *aux;

    if(tipo == 'u' || tipo == 'U'){
        return 1;
    } else {
        return 0;
    }
}

int Chown::existsUser(char *buffer, char *newNameGroup){
    if(buffer[0] == '0')
        return 0;

    char *aux = &buffer[0];
    while(*aux != ','){
        aux++;
    }
    aux++;

    //Obtener el tipo del usuario/grupo
    char tipo = *aux;

    if(tipo == 'U' || tipo == 'u'){
        aux++;
        aux++;

        while(*aux != ','){
            aux++;
        }

        aux++;

        char grp[16]; grp[0] = '\0';
        int contador = 0;
        while(*aux != ','){
            grp[contador] = *aux;
            aux++;
            contador++;
        }
        grp[contador] = '\0';

        if(strcmp(grp,newNameGroup) == 0){
            return 1;
        }
        return 0;
    } else {
        return 0;
    }
}


