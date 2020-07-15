#include "cp.h"

Cp::Cp()
{

}


int Cp::ejecutarCp(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

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


    //buscar_Origen el archivo en el directorio de archivos y Cpar su contenido
    int resultado = buscar_Origen_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_DEST].valor,listaMontaje,usuario);

    fclose(archivo);

    if(resultado == 1){
       return 1;
    } else {
        return 0;
    }
}

int Cp::buscar_Origen_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){
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
                permisos_cambiados = buscar_Origen_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny,listaMontaje,usuario);
            } else if(i < 12){
                permisos_cambiados = buscar_Origen_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny,listaMontaje,usuario);
            } else if(i == 12){
                permisos_cambiados = buscar_Origen_Puntero_Simple(inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny,listaMontaje,usuario);
            } else if(i == 13){
                permisos_cambiados = buscar_Origen_Puntero_Doble(inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny,listaMontaje,usuario);
            } else if(i == 14){
                permisos_cambiados = buscar_Origen_Puntero_Triple(inodo->i_block[i], superBloque, archivo, pathOrigin, pathDestiny,listaMontaje,usuario);
            }

            if(permisos_cambiados == 1)
                return 1; //Se cambiaron los permisos correctamente

            if(permisos_cambiados == -1)
                return -1; //El usuario no es el propietario del archivo/carpeta, error

            if(permisos_cambiados == 0)
                continue; //No se ha hallado el archivo, continuear buscando
        }
    }
    return 1;
}

int Cp::buscar_Origen_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque,  struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){
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

                //Carpeta origen encontrada
                if(isEndOfPath(pathOrigin)){

                    if(isFile(pathOrigin)){

                        strcat(pathDestiny,"/");
                        strcat(pathDestiny,folder->b_content[i].b_name);

                        //Truca el contenido de la carpeta 'temp.txt'
                        FILE *a = fopen("temp.txt","w");
                        fclose(a);

                        //Guardar el contenido del archivo un una carpeta fisica llamada 'temp.txt'
                        char pathActual[100];
                        pathActual[0] = '\0';

                        //Copia el contenido del archivo
                        copiar_Inodo(folder->b_content[i].b_inode, superBloque,archivo,pathActual,pathDestiny,listaMontaje,usuario);

                        //Copiar archivo
                        struct comando comando;
                        std::map<TipoParametro,struct parametro> parametros;
                        char buffer[100]; buffer[0] = '\0';
                        strcat(buffer,pathDestiny);
                        strcat(buffer,pathActual);
                        strcat(parametros[T_PATH].valor, buffer);
                        strcat(parametros[T_CONT].valor,"temp.txt");
                        comando.tipo = T_MKDIR;
                        comando.parametros = &parametros;
                        Mkfile::ejecutarMkfile(comando,listaMontaje,usuario);

                        return 1;
                    } else {
                        if(pathDestiny[0] != '/'){
                            strcat(pathDestiny,"/");
                        }

                        strcat(pathDestiny,folder->b_content[i].b_name);
                        struct comando comando;
                        std::map<TipoParametro,struct parametro> parametros;
                        strcat(parametros[T_PATH].valor,pathDestiny);
                        comando.tipo = T_MKDIR;
                        comando.parametros = &parametros;
                        Mkdir::ejecutarMkdir(comando,listaMontaje,usuario);

                        char pathActual[100];
                        pathActual[1] = '\0';
                        copiar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,pathActual,pathDestiny,listaMontaje,usuario);

                        return 1;
                    }

                }

                //Carpeta no encontrada, continuar buscando
                while(pathOrigin[0] != '/'){
                    pathOrigin++;
                }

                return buscar_Origen_Inodo(folder->b_content[i].b_inode,superBloque,archivo,pathOrigin,pathDestiny,listaMontaje,usuario);

            }
        }
    }
    return 0;
}

int Cp::buscar_Origen_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Origen_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],superBloque,archivo,pathOrigin, pathDestiny,listaMontaje,usuario)){
            return 1;
        }
    }
    return 0;
}

int Cp::buscar_Origen_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Origen_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,pathOrigin, pathDestiny,listaMontaje,usuario)){
            return 1;
        }
    }
    return 0;
}

int Cp::buscar_Origen_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *pathOrigin, char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Origen_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,pathOrigin, pathDestiny,listaMontaje,usuario)){
            return 1;
        }
    }
    return 0;
}

void Cp::copiar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){

    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                copiar_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, pathActual, pathDestiny,listaMontaje,usuario);
            } else if(i < 12){
                copiar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, pathActual, pathDestiny,listaMontaje,usuario);
            } else if(i == 12){
                copiar_Puntero_Simple(inodo->i_block[i], inodo->i_type, superBloque, archivo, pathActual, pathDestiny,listaMontaje,usuario);
            } else if(i == 13){
                copiar_Puntero_Doble(inodo->i_block[i], inodo->i_type, superBloque, archivo, pathActual, pathDestiny,listaMontaje,usuario);
            } else if(i == 14){
                copiar_Puntero_Triple(inodo->i_block[i], inodo->i_type, superBloque, archivo, pathActual, pathDestiny,listaMontaje,usuario);
            }
        }
    }
}

void Cp::copiar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, char tipo_inodo, struct superBlock *superBloque, FILE *archivo,  char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){

    if(tipo_inodo == 1){
        //INODO DE TIPO ARCHIVO

        struct fileBlock *file = new struct fileBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(fileBlock),SEEK_SET );
        fread(file,sizeof(fileBlock),1,archivo);

        FILE *a = fopen("temp.txt","a");
        fwrite(file->b_content,64*sizeof(char),1,a);
        fclose(a);

    } else {
        //INODO DE TIPO CARPETA

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


                if(isFile(folder->b_content[i].b_name)){

                    //Concatenar el nombre del archivo al 'pathActual'
                    strcat(pathActual,"/");
                    strcat(pathActual,folder->b_content[i].b_name);

                    //Truca el contenido de la carpeta 'temp.txt'
                    FILE *a = fopen("temp.txt","w");
                    fclose(a);

                    //Guardar el contenido del archivo un una carpeta fisica llamada 'temp.txt'
                    copiar_Inodo(folder->b_content[i].b_inode, superBloque,archivo,pathActual,pathDestiny,listaMontaje,usuario);

                    //Copiar archivo
                    struct comando comando;
                    std::map<TipoParametro,struct parametro> parametros;
                    char buffer[100]; buffer[0] = '\0';
                    strcat(buffer,pathDestiny);
                    strcat(buffer,pathActual);
                    strcat(parametros[T_PATH].valor, buffer);
                    strcat(parametros[T_CONT].valor,"temp.txt");
                    comando.tipo = T_MKDIR;
                    comando.parametros = &parametros;
                    Mkfile::ejecutarMkfile(comando,listaMontaje,usuario);

                    //Desconcatenar el nombre del archivo al 'pathActual'
                    int size = strlen(pathActual);
                    char *aux = &pathActual[size-1];
                    while(aux[0] != '/'){
                        aux--;
                    }
                    aux[0] = '\0';

                } else {

                    //Concatenar el nombre de la carpeta al 'pathActual'
                    strcat(pathActual,"/");
                    strcat(pathActual,folder->b_content[i].b_name);

                    //Copiar carpeta
                    struct comando comando;
                    std::map<TipoParametro,struct parametro> parametros;
                    char buffer[100]; buffer[0] = '\0';
                    strcat(buffer,pathDestiny);
                    strcat(buffer,pathActual);
                    strcat(parametros[T_PATH].valor, buffer);
                    comando.tipo = T_MKDIR;
                    comando.parametros = &parametros;
                    Mkdir::ejecutarMkdir(comando,listaMontaje,usuario);

                    //Copiar el contenido de la carpeta
                    copiar_Inodo(folder->b_content[i].b_inode, superBloque,archivo,pathActual,pathDestiny,listaMontaje,usuario);

                    //Desconcatenar el nombre de la carpeta al 'pathActual'
                    int size = strlen(pathActual);
                    char *aux = &pathActual[size-1];
                    while(aux[0] != '/'){
                        aux--;
                    }
                    aux[0] = '\0';
                }



            }
        }
    }


}

void Cp::copiar_Puntero_Simple(int indice_bloque, char tipo_inodo, struct superBlock *superBloque, FILE *archivo,  char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        copiar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],tipo_inodo,superBloque,archivo,pathActual,pathDestiny,listaMontaje,usuario);
    }
}

void Cp::copiar_Puntero_Doble(int indice_bloque, char tipo_inodo, struct superBlock *superBloque, FILE *archivo,  char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        copiar_Puntero_Simple(bloquePunteros->b_pointers[i],tipo_inodo,superBloque,archivo,pathActual,pathDestiny,listaMontaje,usuario);
    }
}

void Cp::copiar_Puntero_Triple(int indice_bloque, char tipo_inodo, struct superBlock *superBloque, FILE *archivo,  char *pathActual, const char *pathDestiny, ListaMontaje *listaMontaje, struct user *usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        copiar_Puntero_Doble(bloquePunteros->b_pointers[i],tipo_inodo,superBloque,archivo,pathActual,pathDestiny,listaMontaje,usuario);
    }
}


int Cp::startsWith(char *pathCompleto, char *pathDestiny){
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

int Cp::isFile(char *pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Cp::isEndOfPath(char *pathCompleto){
    while( *pathCompleto != '/'){
        if(*pathCompleto == '\0'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}
