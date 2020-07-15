#include "find.h"

Find::Find()
{

}

int Find::ejecutarFind(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

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


    //buscar_Inicio el archivo en el directorio de archivos y Findar su contenido
    int contador = 0;
    int resultado = buscar_Inicio_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_NAME].valor,contador);

    fclose(archivo);

    if(resultado == 1){
       return 1;
    } else {
        return 0;
    }
}

int Find::buscar_Inicio_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *startPath, char *fileFolderName, int &contador){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    startPath++;
    if(startPath[0] == '\0'){
        //Carpeta de inicio encontrada
        std::string bufferSalida;
        if(buscar_Fin_Inodo(indice_inodo,superBloque,archivo,fileFolderName,bufferSalida,contador)){
            //Archivo encontrado
            std::string temp = bufferSalida;
            bufferSalida = "";
            bufferSalida += "/";
            bufferSalida += '\n';
            bufferSalida += temp;
            std::cout<<bufferSalida<<std::endl;
            return 1;
        }
        return 0;
    }

    int permisos_cambiados = 0;
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                permisos_cambiados = buscar_Inicio_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, startPath, fileFolderName, contador);
            } else if(i < 12){
                permisos_cambiados = buscar_Inicio_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, startPath, fileFolderName, contador);
            } else if(i == 12){
                permisos_cambiados = buscar_Inicio_Puntero_Simple(inodo->i_block[i], superBloque, archivo, startPath, fileFolderName, contador);
            } else if(i == 13){
                permisos_cambiados = buscar_Inicio_Puntero_Doble(inodo->i_block[i], superBloque, archivo, startPath, fileFolderName, contador);
            } else if(i == 14){
                permisos_cambiados = buscar_Inicio_Puntero_Triple(inodo->i_block[i], superBloque, archivo, startPath, fileFolderName, contador);
            }

            if(permisos_cambiados == 1)
                return 1; //Se cambiaron los permisos correctamente

            if(permisos_cambiados == -1)
                return -1; //El usuario no es el propietario del archivo/carpeta, error

            if(permisos_cambiados == 0)
                continue; //No se ha hallado el archivo, continuear buscando
        }
    }
    std::cout<<"No se encontro la carpeta '"<<startPath<<"'"<<std::endl;
    return 1;
}

int Find::buscar_Inicio_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque,  struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador){
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
            if(startsWith(startPath,folder->b_content[i].b_name)){

                //Validar si ste strata de un archivo
                if(isFile(startPath)){
                    std::cout<<"El PATH de inicio no puede ser un archivo"<<std::endl;
                    return -1;
                }

                //Carpeta de inicio encontrada, buscar archivo
                if(isEndOfPath(startPath)){

                    std::string bufferSalida;
                    if(buscar_Fin_Inodo(folder->b_content[i].b_inode,superBloque,archivo,fileFolderName,bufferSalida,contador)){
                        //Archivo encontrado
                        std::string temp = bufferSalida;
                        bufferSalida = "";
                        bufferSalida += folder->b_content[i].b_name;
                        bufferSalida += '\n';
                        bufferSalida += temp;
                        std::cout<<bufferSalida<<std::endl;
                        return 1;
                    } else {
                        std::cout<<"No se encontro el archivo/carpeta '"<<fileFolderName<<"'"<<std::endl;
                        return -1;
                    }

                }

                //Carpeta no encontrada, continuar buscando
                while(startPath[0] != '/'){
                    startPath++;
                }
                return buscar_Inicio_Inodo(folder->b_content[i].b_inode,superBloque,archivo,startPath,fileFolderName,contador);

            }
        }
    }
    return 0;
}

int Find::buscar_Inicio_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Inicio_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],superBloque,archivo,startPath, fileFolderName,contador)){
            return 1;
        }
    }
    return 0;
}

int Find::buscar_Inicio_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Inicio_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,startPath, fileFolderName,contador)){
            return 1;
        }
    }
    return 0;
}

int Find::buscar_Inicio_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *startPath, char *fileFolderName, int &contador){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Inicio_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,startPath, fileFolderName,contador)){
            return 1;
        }
    }
    return 0;
}

int Find::buscar_Fin_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *fileFolderName, std::string &bufferSalida, int contador){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    contador++;

    int permisos_cambiados = 0;
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                permisos_cambiados = buscar_Fin_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, fileFolderName, bufferSalida, contador);
            } else if(i < 12){
                permisos_cambiados = buscar_Fin_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, fileFolderName, bufferSalida, contador);
            } else if(i == 12){
                permisos_cambiados = buscar_Fin_Puntero_Simple(inodo->i_block[i], superBloque, archivo, fileFolderName, bufferSalida, contador);
            } else if(i == 13){
                permisos_cambiados = buscar_Fin_Puntero_Doble(inodo->i_block[i], superBloque, archivo, fileFolderName, bufferSalida, contador);
            } else if(i == 14){
                permisos_cambiados = buscar_Fin_Puntero_Triple(inodo->i_block[i], superBloque, archivo, fileFolderName, bufferSalida, contador);
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

int Find::buscar_Fin_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque,  struct superBlock *superBloque, FILE *archivo,  char *fileFolderName, std::string &bufferSalida, int contador){
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
            if(startsWith(fileFolderName,folder->b_content[i].b_name)){


                //Archivo encontrado
                for(int i = 0; i < contador-2; i++){
                    bufferSalida += " ";
                }
                bufferSalida += "|_";
                bufferSalida += fileFolderName;
                bufferSalida += '\n';
                return 1;


            }
            if(!isFile(folder->b_content[i].b_name)){


                //Recorrer la carpeta
                contador++;
                if(buscar_Fin_Inodo(folder->b_content[i].b_inode,superBloque,archivo,fileFolderName,bufferSalida,contador)){
                    //Archivo encontrado
                    std::string temp = bufferSalida;
                    bufferSalida = "";
                    for(int i = 0; i < contador-2; i++){
                        bufferSalida += " ";
                    }
                    bufferSalida += "|_";
                    bufferSalida += folder->b_content[i].b_name;
                    bufferSalida += '\n';
                    bufferSalida += temp;
                    return 1;
                }



            }
        }
    }
    return 0;
}

int Find::buscar_Fin_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *fileFolderName, std::string &bufferSalida,int contador){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Fin_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],superBloque,archivo,fileFolderName,bufferSalida,contador)){
            return 1;
        }
    }
    return 0;
}

int Find::buscar_Fin_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *fileFolderName, std::string &bufferSalida, int contador){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Fin_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,fileFolderName,bufferSalida,contador)){
            return 1;
        }
    }
    return 0;
}

int Find::buscar_Fin_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *fileFolderName, std::string &bufferSalida, int contador){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(buscar_Fin_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,fileFolderName,bufferSalida,contador)){
            return 1;
        }
    }
    return 0;
}

int Find::startsWith(char *pathCompleto, char *fileFolderName){
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

int Find::isFile(char *pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Find::isEndOfPath(char *pathCompleto){
    while( *pathCompleto != '/'){
        if(*pathCompleto == '\0'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}
