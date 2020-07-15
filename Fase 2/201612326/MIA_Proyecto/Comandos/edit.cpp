#include "edit.h"

Edit::Edit()
{

}

int Edit::ejecutarEdit(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

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


    //Buscar el archivo en el directorio de archivos y editar su contenido
    int resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_CONT].valor,usuario);

    fclose(archivo);

    if(resultado){
       std::cout<<"Archivo editado correctamente"<<std::endl;
       return 1;
    } else {
        std::cout<<"El archivo no se ha editado"<<std::endl;
        return 0;
    }
}

int Edit::visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *path, char *cont, struct user* usuario){
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
                permisos_cambiados = visitar_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, path, cont, usuario);
            } else if(i < 12){
                permisos_cambiados = visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, path, cont, usuario);
            } else if(i == 12){
                permisos_cambiados = visitar_Puntero_Simple(inodo->i_block[i], superBloque, archivo, path, cont, usuario);
            } else if(i == 13){
                permisos_cambiados = visitar_Puntero_Doble(inodo->i_block[i], superBloque, archivo, path, cont, usuario);
            } else if(i == 14){
                permisos_cambiados = visitar_Puntero_Triple(inodo->i_block[i], superBloque, archivo, path, cont, usuario);
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

int Edit::visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque,  struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario){
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
                    //Archivo
                    return editar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,cont,usuario);
                } else {
                    //Carpeta
                    while(path[0] != '/')
                        path++;

                    return visitar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,path,cont,usuario);

                }
            }
        }
    }
    return 0;
}

int Edit::visitar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, bloquePunteros->b_pointers[i],superBloque,archivo,path, cont, usuario)){
            return 1;
        }
    }
    return 0;
}

int Edit::visitar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo,path, cont, usuario)){
            return 1;
        }
    }
    return 0;
}

int Edit::visitar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo,  char *path, char *cont, struct user* usuario){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo,path, cont, usuario)){
            return 1;
        }
    }
    return 0;
}

int Edit::editar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *cont, struct user* usuario){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    //CORREGIR: Se debe validar que el usuario tenga permisos de escritura

    int contador = 0;
    while(cont[0] != '\0') {

        if(contador == 0){
            if(editar_Bloque_Archivo(inodo->i_block[contador], superBloque, archivo, cont)){
                contador++;
            }
        } else if(contador < 12){
            if(editar_Bloque_Archivo(inodo->i_block[contador], superBloque, archivo, cont)){
                contador++;
            }
        } else if(contador == 12){
            if(editar_Puntero_Simple(inodo->i_block[contador], superBloque, archivo, cont)){
                contador++;
            }
        } else if(contador == 13){
            if(editar_Puntero_Doble(inodo->i_block[contador], superBloque, archivo, cont)){
                contador++;
            }
        } else if(contador == 14){
            if(editar_Puntero_Triple(inodo->i_block[contador], superBloque, archivo, cont)){
                contador++;
            }
        }
    }

    //Guardar los cambios en el inodo
    fseek(archivo, superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fwrite(inodo,sizeof(inodeTable),1,archivo);

    return 1;
}

int Edit::editar_Bloque_Archivo(int &indice_bloque,  struct superBlock *superBloque, FILE *archivo, char *&cont){
    struct fileBlock *file;

    if(indice_bloque == -1){
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);
        file = Getters::getNewFileBlock();

        char uno = 1;
        fseek(archivo,superBloque->s_bm_block_start + indice_bloque,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fwrite(file,sizeof(pointersBlock),1,archivo);
    } else {
        file = new struct fileBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
        fread(file,sizeof(folderBlock),1,archivo);
    }


    //Recorrer el folderBlock
    for(int i = 0; i < 64; i++){
        //Buscar espacio libre
        if(file->b_content[i] == '\0'){

            //concatenar 'cont' al archivo
            while(i < 64 && cont[0] != '\0'){
                file->b_content[i] = cont[0];
                cont++;
                i++;
            }

            if(i < 64)
                file->b_content[i] = '\0';

            fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
            fwrite(file,sizeof(folderBlock),1,archivo);

            return 1;
        }
    }

    //El bloque esta lleno, seguir buscando en los demas bloques
    return 1;
}

int Edit::editar_Puntero_Simple(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&cont){

    struct pointersBlock *bloquePunteros;

    if(indice_bloque == -1){
        bloquePunteros = Getters::getNewPointersBlock();
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);

        char uno = 1;
        fseek(archivo,superBloque->s_bm_block_start + indice_bloque,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }

    for(int i = 0; i < 16; i++){

        if(bloquePunteros->b_pointers[i] == -1)
            //No existe el bloque, crear uno y editarlo
            editar_Bloque_Archivo(bloquePunteros->b_pointers[i],superBloque,archivo, cont);
        else if(!esta_Lleno_Bloque_Archivo(bloquePunteros->b_pointers[i],superBloque,archivo))
            //Editar el bloque de archivos solo si NO esta vacio
            editar_Bloque_Archivo(bloquePunteros->b_pointers[i],superBloque,archivo, cont);
        else
            //El bloque ya existe y esta lleno, buscar otro bloque
            continue;


        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        if(i == 15 && esta_Lleno_Bloque_Archivo(bloquePunteros->b_pointers[i],superBloque,archivo))
            return 1;
        else
            return 0;
    }

    //Todos los apuntadores existen y estan llenos, continuar buscando
    return 1;
}

int Edit::editar_Puntero_Doble(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&cont){

    struct pointersBlock *bloquePunteros;

    if(indice_bloque == -1){
        bloquePunteros = Getters::getNewPointersBlock();
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);

        char uno = 1;
        fseek(archivo,superBloque->s_bm_block_start + indice_bloque,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1)
            //No existe el bloque, crear uno y editarlo
            editar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo, cont);
        else if(!esta_Lleno_Simple(bloquePunteros->b_pointers[i],superBloque,archivo))
            //Editar el bloque de archivos solo si NO esta vacio
            editar_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo, cont);
        else
            //El bloque ya existe y esta lleno, buscar otro bloque
            continue;


        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        if(i == 15 && esta_Lleno_Simple(bloquePunteros->b_pointers[i],superBloque,archivo))
            return 1;
        else
            return 0;
    }

    //Todos los apuntadores existen y estan llenos, continuar buscando
    return 1;
}

int Edit::editar_Puntero_Triple(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&cont){
    struct pointersBlock *bloquePunteros;

    if(indice_bloque == -1){
        bloquePunteros = Getters::getNewPointersBlock();
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);

        char uno = 1;
        fseek(archivo,superBloque->s_bm_block_start + indice_bloque,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);
    } else {
        bloquePunteros = new struct pointersBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
        fread(bloquePunteros,sizeof(pointersBlock),1,archivo);
    }

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1)
            //No existe el bloque, crear uno y editarlo
            editar_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo, cont);
        else if(!esta_Lleno_Doble(bloquePunteros->b_pointers[i],superBloque,archivo))
            //Editar el bloque de archivos solo si NO esta vacio
            editar_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo, cont);
        else
            //El bloque ya existe y esta lleno, buscar otro bloque
            continue;


        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        if(i == 15 && esta_Lleno_Doble(bloquePunteros->b_pointers[i],superBloque,archivo))
            return 1;
        else
            return 0;
    }

    //Todos los apuntadores existen y estan llenos, continuar buscando
    return 0;
}

int Edit::startsWith(char *pathCompleto, char *fileFolderName){
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

int Edit::isFile(char *pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Edit::esta_Lleno_Bloque_Archivo(int indice_bloque_archivo, struct superBlock *superBloque, FILE *archivo){
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_archivo * superBloque->s_block_size,SEEK_SET);
    fread(file,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 64; i++){
        if(file->b_content[i] == '\0')
            return 0;
    }
    return 1;
}

int Edit::esta_Lleno_Simple(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *pointers = new struct pointersBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_punteros_simple * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(pointers->b_pointers[i] == -1)
            return 0;
        if(!esta_Lleno_Bloque_Archivo(pointers->b_pointers[i],superBloque,archivo))
            return 0;
    }
    return 1;
}

int Edit::esta_Lleno_Doble(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *pointers = new struct pointersBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_punteros_simple * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(pointers->b_pointers[i] == -1)
            return 0;
        if(!esta_Lleno_Simple(pointers->b_pointers[i],superBloque,archivo))
            return 0;
    }
    return 1;
}



