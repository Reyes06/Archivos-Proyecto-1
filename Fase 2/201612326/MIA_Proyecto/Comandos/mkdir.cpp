#include "mkdir.h"

Mkdir::Mkdir()
{

}

int Mkdir::ejecutarMkdir(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

    if(usuario->active == 0){
        std::cout<<"No hay ningun usuario logueado en el sistema"<<std::endl;
        return 0;
    }

    std::map<TipoParametro, struct parametro> parametros = *(comando.parametros);
    //1. Verificar que se trata de un archivo
    char* path = parametros[T_PATH].valor;
    bool esFile = false;
    for(size_t i = 0; i < strlen(path); i++){
        if(path[i] == '.'){
            esFile = true;
            break;
        }
    }
    if(esFile){
        std::cout<<"El PATH debe hacer referencia a una carpeta, no a un archivo"<<std::endl;
        return 0;
    }

    //2. Obtener la particion montada
    struct partition particion = listaMontaje->getPrimaryMountPartition(usuario->partition_id);

    //3. Validar que la particion se haya encontrado
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    //4. Obtener el superbloque
    struct superBlock *superBloque = new struct superBlock;
    FILE *archivo = fopen(listaMontaje->getDiskLocation(usuario->partition_id),"rb+");
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);

    //5. Crear Archivo
    if(strcmp(parametros[T_PATH].valor,"/eddy010") == 0){
        std::cout<<"";
    }
    int resultado;
    if(parametros[T_P].valor[0] == '1'){
         resultado = visitar_Inodo(0,0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_CONT].valor,1,usuario,parametros[T_SIZE].valor);
    } else {
        resultado = visitar_Inodo(0,0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_CONT].valor,0,usuario,parametros[T_SIZE].valor);
    }


    fclose(archivo);

    //6. Mensajes
    if(resultado == 1 ){
        std::cout<<"Se ha creado el archivo satisfactoriamente"<<std::endl;
        return 1;
    }

    if (resultado == 0){
        std::cout<<"Ha ocurrido un error al crear el archivo"<<std::endl;
        return 0;
    }

    free(superBloque);
    std::cout<<"No se que ha pasado :("<<std::endl;
    return 0;
}



int Mkdir::visitar_Inodo(int indice_inodo, int indice_inodo_padre, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, char *fileSize){

    if(indice_inodo == 213){
        std::cout<<"";
    }
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    //Quitar el '/' del path
    newFilePath++;
    int respuesta;

    //Verificar la carpeta se debe crear en el directorio actual o en un subdirectorio
    if(isEndOfPath(newFilePath)){
        //Crear en el directorio actual

        //1. Validar que el archivo no exista
        for(int i = 0; i < 15; i++) {
            if(inodo->i_block[i] == -1){
                continue;
            } else {
                //Visitar Carpetas/Archivos
                if(i == 0){
                    respuesta = visitar_Bloque_Carpeta_Archivo_EXISTE(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                } else if(i < 12){
                    respuesta = visitar_Bloque_Carpeta_Archivo_EXISTE(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                } else if(i == 12){
                    respuesta = visitar_Puntero_Simple_EXISTE(inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                } else if(i == 13){
                    respuesta = visitar_Puntero_Doble_EXISTE(inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                } else if(i == 14){
                    respuesta = visitar_Puntero_Triple_EXISTE(inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                }

                //Validar respuestas
                if(respuesta == 1){
                    free(inodo);
                    return -1;  //El archivo ya existe
                }

                if(respuesta == 0)
                    continue;   //No se ha encontrado el archivo, continuar buscando
            }
        }

        //2. Crear la carpeta en el directorio actual
        for(int i = 0; i < 15; i++) {

            //Visitar sub arpetas/Archivos
            if(i == 0){
                respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
            } else if(i < 12){
                respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
            } else if(i == 12){
                respuesta = visitar_Puntero_Simple_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
            } else if(i == 13){
                respuesta = visitar_Puntero_Doble_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
            } else if(i == 14){
                respuesta = visitar_Puntero_Triple_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
            }

            //Validar respuestas
            if(respuesta == 1){
                fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
                fwrite(inodo,sizeof(inodeTable),1,archivo);
                free(inodo);
                return 1;  //El archivo se ha creado satisfactoriamente
            }

            if(respuesta == 0)
                continue;   //No se ha encontrado el archivo, continuar buscando

        }


        free(inodo);
        std::cout<<"No existen apuntadores para almacenar el archivo en el inodo "<<indice_inodo<<std::endl;
        return 0;

    } else {

        //1. Validar que la carpeta exista
        int existe = 0;
        for(int i = 0; i < 15; i++) {
            if(inodo->i_block[i] == -1){
                continue;
            } else {
                //Visitar Carpetas/Archivos
                if(i == 0){
                    respuesta = visitar_Bloque_Carpeta_Archivo_EXISTE(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                } else if(i < 12){
                    respuesta = visitar_Bloque_Carpeta_Archivo_EXISTE(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                } else if(i == 12){
                    respuesta = visitar_Puntero_Simple_EXISTE(inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                } else if(i == 13){
                    respuesta = visitar_Puntero_Doble_EXISTE(inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                } else if(i == 14){
                    respuesta = visitar_Puntero_Triple_EXISTE(inodo->i_block[i], superBloque, archivo, newFilePath,contPath,fileSize);
                }

                //Validar respuestas
                if(respuesta == 1){
                    existe = 1;
                    break;  //La carpeta si existe
                }

                if(respuesta == 0)
                    continue;   //No se ha encontrado la carpeta, continuar buscando
            }
        }

        if(!existe && p_param){
            for(int i = 0; i < 15; i++) {

                //Visitar sub arpetas/Archivos
                if(i == 0){
                    respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                } else if(i < 12){
                    respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                } else if(i == 12){
                    respuesta = visitar_Puntero_Simple_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                } else if(i == 13){
                    respuesta = visitar_Puntero_Doble_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                } else if(i == 14){
                    respuesta = visitar_Puntero_Triple_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                }

                //Validar respuestas
                if(respuesta == 1){
                    existe = 1;
                    break;  //El archivo se ha creado satisfactoriamente
                }

                if(respuesta == 0)
                    continue;   //No se ha encontrado el archivo, continuar buscando

            }
        }

        if(!existe){
            std::cout<<"La carpeta '"<<newFilePath<<"' no existe"<<std::endl;
            free(inodo);
            return 0;
        }


        //2. Crear el archivo en un subdirectorio
        for(int i = 0; i < 15; i++) {
            if(inodo->i_block[i] == -1){
                continue;
            } else {
                //Visitar Carpetas/Archivos
                if(i == 0){
                    respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                } else if(i < 12){
                    respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                } else if(i == 12){
                    respuesta = visitar_Puntero_Simple_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                } else if(i == 13){
                    respuesta = visitar_Puntero_Doble_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                } else if(i == 14){
                    respuesta = visitar_Puntero_Triple_CREAR(inodo->i_block[i], indice_inodo_padre, superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize);
                }

                //Validar respuestas
                if(respuesta == 1){
                    free(inodo);
                    return 1;  //Se ha creado el archivo en un subdirectorio
                }

                if(respuesta == 0)
                    continue;   //No se ha encontrado el archivo, continuar buscando
            }
        }

        //Si ha llegado aqui es porque no encontro el archivo indicado en la ruta
        free(inodo);
        std::cout<<"La carpeta '"<<newFilePath<<"' no existe"<<std::endl;
        return 0;
    }
}

int Mkdir::visitar_Bloque_Carpeta_Archivo_EXISTE(Condicion condicion, int indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize){

    struct folderBlock *folder = new struct folderBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(folderBlock),SEEK_SET );
    fread(folder,sizeof(folderBlock),1,archivo);

    int inicio;
    switch(condicion){
    case NO_IGNORAR_PRIMEROS_REGISTROS:
        inicio = 0;
        break;
    case IGNORAR_PRIMEROS_REGISTROS:
        inicio = 2;
        break;
    }

    for(int i = inicio; i < 4; i++){
        if(folder->b_content[i].b_inode == -1){
            continue;
        }
        if(startsWith(newFilePath, folder->b_content[i].b_name)){
            free(folder);
            return 1;
        }
    }
    free(folder);
    return 0;
}

int Mkdir::visitar_Puntero_Simple_EXISTE(int indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize){


    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);


    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Bloque_Carpeta_Archivo_EXISTE(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,fileSize)){
            free(bloquePunteros);
            return 1;
        }
    }
    free(bloquePunteros);
    return 0;
}

int Mkdir::visitar_Puntero_Doble_EXISTE(int indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);


    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Simple_EXISTE(bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,fileSize)){
            free(bloquePunteros);
            return 1;
        }
    }
    free(bloquePunteros);
    return 0;
}

int Mkdir::visitar_Puntero_Triple_EXISTE(int indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);


    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Doble_EXISTE(bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,fileSize)){
            free(bloquePunteros);
            return 1;
        }
    }
    free(bloquePunteros);
    return 0;
}

int Mkdir::visitar_Bloque_Carpeta_Archivo_CREAR(Condicion condicion, int &block_index, int indice_inodo_padre, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char* contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize){

    struct folderBlock *folder;

    if(block_index == -1){
        //Crear nuevo folder
        int uno = 1;
        block_index = Getters::getIndexNewBlock(superBloque,archivo);
        folder = Getters::getNewFolderBlock();
        fseek(archivo, superBloque->s_bm_block_start + block_index,SEEK_SET );
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + block_index * sizeof(folderBlock),SEEK_SET );
        fwrite(folder,sizeof(folderBlock),1,archivo);
    } else {
        //Obtener el folder
        folder = new struct folderBlock;
        fseek(archivo, superBloque->s_block_start + block_index * sizeof(folderBlock),SEEK_SET );
        fread(folder,sizeof(folderBlock),1,archivo);
    }


    int inicio;
    switch(condicion){
    case NO_IGNORAR_PRIMEROS_REGISTROS:
        inicio = 0;
        break;
    case IGNORAR_PRIMEROS_REGISTROS:
        inicio = 2;
        break;
    }

    //Buscar la carpeta
    for(int i = inicio; i < 4; i++){
        if(folder->b_content[i].b_inode == -1){

            int new_index_block = Getters::getIndexNewBlock(superBloque,archivo);
            int new_index_inode = Getters::getIndexNewInode(superBloque,archivo);

            struct folderBlock *nuevaCarpeta = Getters::getNewFolderBlock(new_index_inode,inodo_index);
            struct inodeTable *newInode = Getters::getNewFolderInode(usuario->uid, usuario->gid,new_index_block);


            //Obtener el nombre de la carpeta
            char * nombreCarpeta = new char [15];
            int contador = 0;
            while(newFilePath[0] != '/' && newFilePath[0] != '\0'){
                nombreCarpeta[contador] = newFilePath[0];
                newFilePath++;
                contador++;
            }
            nombreCarpeta[contador] = '\0';

            //Setear los valores de la carpeta
            strcpy(folder->b_content[i].b_name,nombreCarpeta);
            folder->b_content[i].b_inode = new_index_inode;

            //Guardar los nuevos valores
            char uno = 1;
            fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size, SEEK_SET);
            fwrite(folder,sizeof(folderBlock),1,archivo);
            fseek(archivo,superBloque->s_bm_block_start + new_index_block, SEEK_SET);
            fwrite(&uno,sizeof(char),1,archivo);
            fseek(archivo,superBloque->s_block_start + new_index_block * superBloque->s_block_size, SEEK_SET);
            fwrite(nuevaCarpeta,sizeof(folderBlock),1,archivo);
            fseek(archivo,superBloque->s_bm_inode_start + new_index_inode, SEEK_SET);
            fwrite(&uno,sizeof(char),1,archivo);
            fseek(archivo,superBloque->s_inode_start + new_index_inode * superBloque->s_inode_size, SEEK_SET);
            fwrite(newInode,sizeof(inodeTable),1,archivo);

            free(folder);
            free(nuevaCarpeta);
            free(newInode);
            return 1;

        }



        if(startsWith(newFilePath, folder->b_content[i].b_name)){
            while(newFilePath[0] != '/'){
                newFilePath++;
            }
            int res = visitar_Inodo(folder->b_content[i].b_inode, inodo_index,superBloque,archivo,newFilePath,contPath,p_param,usuario,fileSize);
            free(folder);
            return res;
        }
    }

    free(folder);
    return 0;


}

int Mkdir::visitar_Puntero_Simple_CREAR(int &indice, int indice_inodo_padre, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char* contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize){

    struct pointersBlock *bloquePunteros;

    if(indice == -1){
        int uno = 1;
        indice = Getters::getIndexNewBlock(superBloque,archivo);
        bloquePunteros = Getters::getNewPointersBlock();
        fseek(archivo, superBloque->s_bm_block_start + indice,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }



    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1)
            //No existe el bloque, crear uno y editarlo
            visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i], indice_inodo_padre,superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize);
        else {
            int res = visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i], indice_inodo_padre,superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize);
            if(res == 0){
                //El bloque ya existe y esta lleno, buscar otro bloque
                continue;
            }
        }


        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        free(bloquePunteros);
        return 1;
    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
    fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

    free(bloquePunteros);
    return 0;
}

int Mkdir::visitar_Puntero_Doble_CREAR(int &indice, int indice_inodo_padre, struct superBlock *superBloque, FILE *archivo, char *newFilePath,char*contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize){

    struct pointersBlock *bloquePunteros;

    if(indice == -1){
        int uno = 1;
        indice = Getters::getIndexNewBlock(superBloque,archivo);
        bloquePunteros = Getters::getNewPointersBlock();
        fseek(archivo, superBloque->s_bm_block_start + indice,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }



    for(int i = 0; i < 16; i++){
        if(i == 6 && indice == 362){
            std::cout<<"";
        }
        if(bloquePunteros->b_pointers[i] == -1)
            //No existe el bloque, crear uno y editarlo
            visitar_Puntero_Simple_CREAR(bloquePunteros->b_pointers[i], indice_inodo_padre,superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize);
        else {
            int res = visitar_Puntero_Simple_CREAR(bloquePunteros->b_pointers[i], indice_inodo_padre,superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize);
            //El bloque ya existe y esta lleno, buscar otro bloque
            if(res == 0){
                continue;
            }
        }

        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        free(bloquePunteros);
        return 1;
    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
    fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

    free(bloquePunteros);
    return 0;
}

int Mkdir::visitar_Puntero_Triple_CREAR(int &indice, int indice_inodo_padre, struct superBlock *superBloque, FILE *archivo, char *newFilePath,char*contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize){

    struct pointersBlock *bloquePunteros;

    if(indice == -1){
        int uno = 1;
        indice = Getters::getIndexNewBlock(superBloque,archivo);
        bloquePunteros = Getters::getNewPointersBlock();
        fseek(archivo, superBloque->s_bm_block_start + indice,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }



    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1)
            //No existe el bloque, crear uno y editarlo
            visitar_Puntero_Doble_CREAR(bloquePunteros->b_pointers[i], indice_inodo_padre,superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize);
        else {
            int res = visitar_Puntero_Doble_CREAR(bloquePunteros->b_pointers[i], indice_inodo_padre,superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize);
            if(res == 0){
                //El bloque ya existe y esta lleno, buscar otro bloque
                continue;
            }
        }

        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        free(bloquePunteros);
        return 1;
    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
    fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

    free(bloquePunteros);
    return 0;
}

int Mkdir::startsWith(char *pathCompleto, char *fileFolderName){
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

void Mkdir::removeFolderName(char *&pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        pathCompleto++;
    }
}

int Mkdir::isFile(char *pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Mkdir::isEndOfPath(char *pathCompleto){
    while( *pathCompleto != '/'){
        if(*pathCompleto == '\0'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Mkdir::estaLleno_Bloque_Carpeta(int indice_bloque_carpeta, struct superBlock *superBloque, FILE *archivo){
    struct folderBlock *folder = new struct folderBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_carpeta * superBloque->s_block_size,SEEK_SET);
    fread(folder,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 4; i++){
        if(folder->b_content[i].b_inode == -1){
            free(folder);
            return 0;
        }

    }
    free(folder);
    return 1;
}


int Mkdir::estaLleno_Simple(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *pointers = new struct pointersBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_punteros_simple * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(pointers->b_pointers[i] == -1){
            free(pointers);
            return 0;
        }

        if(!estaLleno_Bloque_Carpeta(pointers->b_pointers[i],superBloque,archivo)){
            free(pointers);
            return 0;
        }
    }
    free(pointers);
    return 1;
}

int Mkdir::estaLleno_Doble(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *pointers = new struct pointersBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_punteros_simple * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(pointers->b_pointers[i] == -1){
            free(pointers);
            return 0;
        }
        if(!estaLleno_Simple(pointers->b_pointers[i],superBloque,archivo)){
            free(pointers);
            return 0;
        }
    }
    free(pointers);
    return 1;
}

