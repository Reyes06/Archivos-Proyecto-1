#include "mv.h"

Mv::Mv()
{

}


int Mv::ejecutarMv(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

    if(!usuario->active){
        std::cout<<"No hay ningun usuario logueado"<<std::endl;
        return 0;
    }

    std::map<TipoParametro, struct parametro> parametros = (*comando.parametros);

    //Verificar que DEST sea una carpeta
    int esArchivo = 0;
    char *aux = parametros[T_DEST].valor;
    while(aux[0] != '\0'){
        if(aux[0] == '.'){
            esArchivo = 1;
            break;
        }
        aux++;
    }
    if(esArchivo){
        std::cout<<"El parametro DEST debe hacer referencia a una carpeta, no un archivo";
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


    //buscar_Origen el archivo en el directorio de archivos y Mvar su contenido
    int resultado = buscar_Origen_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_DEST].valor);

    fclose(archivo);

    if(resultado == 1){
       return 1;
    } else {
        return 0;
    }
}

int Mv::buscar_Origen_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *pathOrigin, char *pathDestiny){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    pathOrigin++;

    int permisos_cambiados = 0;
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                permisos_cambiados = buscar_Origen_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny);
            } else if(i < 12){
                permisos_cambiados = buscar_Origen_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny);
            } else if(i == 12){
                permisos_cambiados = buscar_Origen_Puntero_Simple(inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny);
            } else if(i == 13){
                permisos_cambiados = buscar_Origen_Puntero_Doble(inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny);
            } else if(i == 14){
                permisos_cambiados = buscar_Origen_Puntero_Triple(inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny);
            }

            if(permisos_cambiados == 1)
                return 1; //Se cambiaron los permisos correctamente

            if(permisos_cambiados == -1)
                return -1; //El usuario no es el propietario del archivo/carpeta, error

            if(permisos_cambiados == 0)
                continue; //No se ha hallado el archivo, continuear buscando
        }
    }
    std::cout<<"No se encontro la carpeta '"<<pathOrigin<<"'"<<std::endl;
    return 1;
}

int Mv::buscar_Origen_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque,  struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny){
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
            if(startsWith(pathOrigin,folder->b_content[i].b_name)){

                //Carpeta origen encontrada, buscar carpeta destino
                if(isEndOfPath(pathOrigin)){

                    std::string bufferSalida;
                    if(buscar_Destino_Inodo(0,folder->b_content[i].b_inode,folder->b_content[i].b_name,superBloque,archivo,pathDestiny)){
                        folder->b_content[i].b_inode = -1;
                        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
                        fwrite(folder,sizeof(folderBlock),1,archivo);
                        return 1;
                    } else {
                        std::cout<<"No se encontro el archivo/carpeta '"<<pathDestiny<<"'"<<std::endl;
                        return -1;
                    }

                }

                //Carpeta no encontrada, continuar buscando
                while(pathOrigin[0] != '/'){
                    pathOrigin++;
                }

                return buscar_Origen_Inodo(folder->b_content[i].b_inode,superBloque,archivo,pathOrigin,pathDestiny);

            }
        }
    }
    return 0;
}

int Mv::buscar_Origen_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Origen_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],superBloque,archivo,pathOrigin, pathDestiny)){
            return 1;
        }
    }
    return 0;
}

int Mv::buscar_Origen_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Origen_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,pathOrigin, pathDestiny)){
            return 1;
        }
    }
    return 0;
}

int Mv::buscar_Origen_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Origen_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,pathOrigin, pathDestiny)){
            return 1;
        }
    }
    return 0;
}

int Mv::buscar_Destino_Inodo(int indice_inodo, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo, char *pathDestiny){

    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    pathDestiny++;

    int permisos_cambiados = 0;
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                permisos_cambiados = buscar_Destino_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo, pathDestiny);
            } else if(i < 12){
                permisos_cambiados = buscar_Destino_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo, pathDestiny);
            } else if(i == 12){
                permisos_cambiados = buscar_Destino_Puntero_Simple(inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo, pathDestiny);
            } else if(i == 13){
                permisos_cambiados = buscar_Destino_Puntero_Doble(inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo, pathDestiny);
            } else if(i == 14){
                permisos_cambiados = buscar_Destino_Puntero_Triple(inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo, pathDestiny);
            }

            if(permisos_cambiados == 1)
                return 1; //Se cambiaron los permisos correctamente

            if(permisos_cambiados == -1)
                return -1; //El usuario no es el propietario del archivo/carpeta, error

            if(permisos_cambiados == 0)
                continue; //No se ha hallado el archivo, continuear buscando
        }
    }
    return 0;
}

int Mv::buscar_Destino_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, int file_folder_index, char *file_folder_name,  struct superBlock *superBloque, FILE *archivo,  char *pathDestiny){
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
            if(startsWith(pathDestiny,folder->b_content[i].b_name)){


                //Validar si ste strata de un archivo
                if(isFile(pathDestiny)){
                    std::cout<<"El DEST no puede ser un archivo"<<std::endl;
                    return -1;
                }

                //Carpeta destino encontrada, mover el archivo/carpeta
                if(isEndOfPath(pathDestiny)){



                    if(mover_Archivo_Carpeta_Inodo(folder->b_content[i].b_inode,file_folder_index,file_folder_name,superBloque,archivo)){
                        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
                        fwrite(folder,sizeof(folderBlock),1,archivo);
                        return 1;
                    }
                    return -1;

                }

                //Carpeta no encontrada, continuar buscando
                while(pathDestiny[0] != '/'){
                    pathDestiny++;
                }

                return buscar_Destino_Inodo(folder->b_content[i].b_inode,file_folder_index,file_folder_name, superBloque,archivo,pathDestiny);


            }
        }
    }
    return 0;
}

int Mv::buscar_Destino_Puntero_Simple(int indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo,  char *pathDestiny){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Destino_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i], file_folder_index, file_folder_name,superBloque,archivo,pathDestiny)){
            return 1;
        }
    }
    return 0;
}

int Mv::buscar_Destino_Puntero_Doble(int indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo,  char *pathDestiny){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Destino_Puntero_Simple(bloquePunteros->b_pointers[i], file_folder_index, file_folder_name,superBloque,archivo,pathDestiny)){
            return 1;
        }
    }
    return 0;
}

int Mv::buscar_Destino_Puntero_Triple(int indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo,  char *pathDestiny){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Destino_Puntero_Doble(bloquePunteros->b_pointers[i], file_folder_index, file_folder_name,superBloque,archivo,pathDestiny)){
            return 1;
        }
    }
    return 0;
}

int Mv::mover_Archivo_Carpeta_Inodo(int indice_inodo, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo){

    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    int permisos_cambiados = 0;
    for(int i = 0; i < 15; i++) {
        if(i == 0){
            permisos_cambiados = mover_Archivo_Carpeta_Bloque_Archivo_Carpeta(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo);
        } else if(i < 12){
            permisos_cambiados = mover_Archivo_Carpeta_Bloque_Archivo_Carpeta(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo);
        } else if(i == 12){
            permisos_cambiados = mover_Archivo_Carpeta_Puntero_Simple(inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo);
        } else if(i == 13){
            permisos_cambiados = mover_Archivo_Carpeta_Puntero_Doble(inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo);
        } else if(i == 14){
            permisos_cambiados = mover_Archivo_Carpeta_Puntero_Triple(inodo->i_block[i], file_folder_index, file_folder_name, superBloque, archivo);
        }

        if(permisos_cambiados == 1){
            fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
            fwrite(inodo,sizeof(inodeTable),1,archivo);
            return 1; //Se cambiaron los permisos correctamente
        }


        if(permisos_cambiados == -1)
            return -1; //El usuario no es el propietario del archivo/carpeta, error

        if(permisos_cambiados == 0)
            continue; //No se ha hallado el archivo, continuear buscando

    }

    return 0;
}

int Mv::mover_Archivo_Carpeta_Bloque_Archivo_Carpeta(Ignorar condicion, int &indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo){

    struct folderBlock *folder;

    if(indice_bloque == -1){
        char uno = 1;
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);
        folder = Getters::getNewFolderBlock();
        fseek(archivo, superBloque->s_bm_block_start + indice_bloque,SEEK_SET );
        fread(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
        fwrite(folder,sizeof(folderBlock),1,archivo);
    } else {
        folder = new struct folderBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
        fread(folder,sizeof(folderBlock),1,archivo);
    }


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
            folder->b_content[i].b_inode = file_folder_index;
            strcpy(folder->b_content[i].b_name,file_folder_name);
            fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
            fwrite(folder,sizeof(folderBlock),1,archivo);
            return 1;
        }
    }
    return 0;
}

int Mv::mover_Archivo_Carpeta_Puntero_Simple(int &indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo){

    struct pointersBlock *bloquePunteros;

    if(indice_bloque == -1){
        char uno = 1;
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);
        bloquePunteros = Getters::getNewPointersBlock();
        fseek(archivo, superBloque->s_bm_block_start + indice_bloque,SEEK_SET );
        fread(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET );
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }



    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(mover_Archivo_Carpeta_Bloque_Archivo_Carpeta(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i], file_folder_index, file_folder_name,superBloque,archivo)){
            return 1;
        }
    }
    return 0;
}

int Mv::mover_Archivo_Carpeta_Puntero_Doble(int &indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo){

    struct pointersBlock *bloquePunteros;

    if(indice_bloque == -1){
        char uno = 1;
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);
        bloquePunteros = Getters::getNewPointersBlock();
        fseek(archivo, superBloque->s_bm_block_start + indice_bloque,SEEK_SET );
        fread(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET );
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }


    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(mover_Archivo_Carpeta_Puntero_Simple(bloquePunteros->b_pointers[i], file_folder_index, file_folder_name,superBloque,archivo)){
            return 1;
        }
    }
    return 0;
}

int Mv::mover_Archivo_Carpeta_Puntero_Triple(int &indice_bloque, int file_folder_index, char *file_folder_name, struct superBlock *superBloque, FILE *archivo){

    struct pointersBlock *bloquePunteros;

    if(indice_bloque == -1){
        char uno = 1;
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);
        bloquePunteros = Getters::getNewPointersBlock();
        fseek(archivo, superBloque->s_bm_block_start + indice_bloque,SEEK_SET );
        fread(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET );
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }


    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(mover_Archivo_Carpeta_Puntero_Doble(bloquePunteros->b_pointers[i], file_folder_index, file_folder_name,superBloque,archivo)){
            return 1;
        }
    }
    return 0;
}


int Mv::startsWith(char *pathCompleto, char *pathDestiny){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == *pathDestiny){
            pathCompleto++;
            pathDestiny++;
        } else {
            return 0;
        }
    }
    return 1;
}

int Mv::isFile(char *pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Mv::isEndOfPath(char *pathCompleto){
    while( *pathCompleto != '/'){
        if(*pathCompleto == '\0'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}
