#include "mkusr.h"

Mkusr::Mkusr()
{

}

int Mkusr::ejecutarMkusr(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

    if(!usuario->active){
        std::cout<<"No hay ningun usuario logueado"<<std::endl;
        return 0;
    } else if(usuario->uid != 1){
        std::cout<<"El comando MKUSR solo puede ser utilizado por el usuario ROOT"<<std::endl;
        return 0;
    }

    std::map<TipoParametro, struct parametro> parametros = (*comando.parametros);

    if(strlen(parametros[T_USR].valor) >= 10){
        std::cout<<"El parametro USR debe tener un maximo de 10 caracteres"<<std::endl;
        return 0;
    }
    if(strlen(parametros[T_PWD].valor) >= 10){
        std::cout<<"El parametro PWD debe tener un maximo de 10 caracteres"<<std::endl;
        return 0;
    }
    if(strlen(parametros[T_GRP].valor) >= 10){
        std::cout<<"El parametro GRP debe tener un maximo de 10 caracteres"<<std::endl;
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

    //Recuperar el inodo del archivo users.txt
    struct inodeTable *inodeUsers = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
    fread(inodeUsers,sizeof(inodeTable),1,archivo);

    //Verificar que el usuario no exista y contar la cantidad de usuarios que se han creado
    char *buffer = new char[100];
    buffer[0] = '\0';
    int userCount = 0;
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(recorrer_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_USR].valor, userCount, BUSCAR_USUARIO)){

                    //Actualizar la hora de lectura
                    time_t hora = time(0);
                    inodeUsers->i_atime = *localtime(&hora);
                    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
                    fwrite(inodeUsers,sizeof(inodeTable),1,archivo);
                    fclose(archivo);
                    return 0;
                }
            } else if (i == 12){
                if(recorrer_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_USR].valor, userCount, BUSCAR_USUARIO)){

                    //Actualizar la hora de lectura
                    time_t hora = time(0);
                    inodeUsers->i_atime = *localtime(&hora);
                    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
                    fwrite(inodeUsers,sizeof(inodeTable),1,archivo);
                    fclose(archivo);
                    return 0;
                }
            } else if (i == 13){
                if(recorrer_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_USR].valor, userCount, BUSCAR_USUARIO)){

                    //Actualizar la hora de lectura
                    time_t hora = time(0);
                    inodeUsers->i_atime = *localtime(&hora);
                    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
                    fwrite(inodeUsers,sizeof(inodeTable),1,archivo);
                    fclose(archivo);
                    return 0;
                }
            } else if (i == 14){
                if(recorrer_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_USR].valor, userCount, BUSCAR_USUARIO)){

                    //Actualizar la hora de lectura
                    time_t hora = time(0);
                    inodeUsers->i_atime = *localtime(&hora);
                    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
                    fwrite(inodeUsers,sizeof(inodeTable),1,archivo);
                    fclose(archivo);
                    return 0;
                }
            }
        }
    }

    //Verificar si existe el grupo
    int existe = 0;
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(existe = recorrer_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_GRP].valor, userCount, BUSCAR_GRUPO)){
                    break;
                }
            } else if (i == 12){
                if(existe = recorrer_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_GRP].valor, userCount, BUSCAR_GRUPO)){
                    break;
                }
            } else if (i == 13){
                if(existe = recorrer_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_GRP].valor, userCount, BUSCAR_GRUPO)){
                    break;
                }
            } else if (i == 14){
                if(existe = recorrer_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_GRP].valor, userCount, BUSCAR_GRUPO)){
                    break;
                }
            }
        }
    }

    if(!existe){

        //Actualizar la hora de lectura
        time_t hora = time(0);
        inodeUsers->i_atime = *localtime(&hora);
        fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
        fwrite(inodeUsers,sizeof(inodeTable),1,archivo);

        std::cout<<"El grupo '"<<parametros[T_GRP].valor<<"' no existe"<<std::endl;
        fclose(archivo);
        return 0;
    }


    //Crear el usuario
    buffer[0] = '\0';
    sprintf(buffer, "%d", userCount+1);
    strcat(buffer,",U,");
    strcat(buffer,parametros[T_GRP].valor);
    strcat(buffer,",");
    strcat(buffer,parametros[T_USR].valor);
    strcat(buffer,",");
    strcat(buffer,parametros[T_PWD].valor);
    strcat(buffer,"\n\0");

    inodeUsers->i_size += strlen(buffer);

    for(int i = 0; i < 15; i++){
        if(i < 12){
            if(crear_Bloque_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer)){
                break;
            }
        } else if (i == 12){
            if(crear_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer)){
                break;
            }
        } else if (i == 13){
            if(crear_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer)){
                break;
            }
        } else if (i == 14){
            if(crear_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer)){
                break;
            }
        }
    }

    //Actualizar la hora de modificacion
    time_t hora = time(0);
    inodeUsers->i_mtime = *localtime(&hora);

    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
    fwrite(inodeUsers,sizeof(inodeTable),1,archivo);

    fclose(archivo);
    return 1;
}

int Mkusr::recorrer_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *newGroupName, int &userCount, Opcion op){
    //Recuperar el fileBlock
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
    fread(file,sizeof(fileBlock),1,archivo);

    switch(op){
    case Mkusr::BUSCAR_USUARIO:
    {
        //Concatenar el contenido
        int primer_caracter_libre = strlen(buffer);
        for(size_t i = 0; i < 64;i++){
            if(file->b_content[i] == '\0')
                break;

            if(file->b_content[i] == '\n'){


                    buffer[primer_caracter_libre] = '\n';
                    buffer[primer_caracter_libre+1] = '\0';

                    if(isUser(buffer)){
                        userCount++;
                        if(existsUser(buffer,newGroupName)){
                            std::cout<<"El usuario '"<<newGroupName<<"' ya existe en la particion"<<std::endl;
                            return 1;
                        }
                    }

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
    case Mkusr::BUSCAR_GRUPO:
    {
        //Concatenar el contenido
        int primer_caracter_libre = strlen(buffer);
        for(size_t i = 0; i < 64;i++){
            if(file->b_content[i] == '\0')
                break;

            if(file->b_content[i] == '\n'){


                    buffer[primer_caracter_libre] = '\n';
                    buffer[primer_caracter_libre+1] = '\0';

                    if(isGroup(buffer)){
                        if(existsGroup(buffer,newGroupName)){
                            return 1;
                        }
                    }

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

    }


}

int Mkusr::recorrer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *newGroupName, int &userCount, Opcion op){

    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Directo(pointers->b_pointers[i],superBloque,archivo,buffer,newGroupName,userCount,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Mkusr::recorrer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *newGroupName, int &userCount, Opcion op){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Puntero_Simple(pointers->b_pointers[i],superBloque,archivo,buffer,newGroupName,userCount,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Mkusr::recorrer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *newGroupName, int &userCount, Opcion op){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Puntero_Doble(pointers->b_pointers[i],superBloque,archivo,buffer,newGroupName,userCount,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Mkusr::crear_Bloque_Directo(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer){

    struct fileBlock *file;

    if(indice_bloque == -1){
        char uno = 1;
        indice_bloque = Getters::getIndexNewBlock(superBloque,archivo);
        file = Getters::getNewFileBlock();
        fseek(archivo, superBloque->s_bm_block_start + indice_bloque,SEEK_SET );
        fwrite(&uno,sizeof(fileBlock),1,archivo);
        fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
        fwrite(file,sizeof(fileBlock),1,archivo);
    } else {
        file = new struct fileBlock;
        fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
        fread(file,sizeof(fileBlock),1,archivo);
    }


    int primer_caracter_libre = strlen(file->b_content);
    if(primer_caracter_libre < 64){

        for(int i = primer_caracter_libre; i < 64; i++){
            if(buffer[0] == '\0'){
                file->b_content[i] = '\0';
                break;
            }


            file->b_content[i] = buffer[0];
            buffer++;
        }
        fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
        fwrite(file,sizeof(fileBlock),1,archivo);
        if(buffer[0] == '\0'){
            std::cout<<"Se ha creado el grupo correctamente"<<std::endl;
            return 1;
        } else{
            return 0;
        }
    } else {
        return 0;
    }
}

int Mkusr::crear_Puntero_Simple(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer){

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

        if(bloquePunteros->b_pointers[i] == -1){
            //No existe el bloque, crear uno y editarlo
            if(crear_Bloque_Directo(bloquePunteros->b_pointers[i],superBloque,archivo, buffer) ==0){
                continue;
            }
        } else if(!esta_Lleno_Bloque_Archivo(bloquePunteros->b_pointers[i],superBloque,archivo)){
            //Editar el bloque de archivos solo si NO esta vacio
            if(crear_Bloque_Directo(bloquePunteros->b_pointers[i],superBloque,archivo, buffer) == 0){
                continue;
            }
        } else
            //El bloque ya existe y esta lleno, buscar otro bloque
            continue;


        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        return 1;
    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

    //Todos los apuntadores existen y estan llenos, continuar buscando
    return 0;
}

int Mkusr::crear_Puntero_Doble(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer){

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
        if(bloquePunteros->b_pointers[i] == -1){
            //No existe el bloque, crear uno y editarlo
            if(crear_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo, buffer) == 0){
                continue;
            }
        } else if(!esta_Lleno_Apuntador_Simple(bloquePunteros->b_pointers[i],superBloque,archivo)){
            //Editar el bloque de archivos solo si NO esta vacio
            if(crear_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo, buffer) == 0){
                continue;
            }
        } else
            //El bloque ya existe y esta lleno, buscar otro bloque
            continue;


        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        return 1;
    }

    //Todos los apuntadores existen y estan llenos, continuar buscando
    return 0;
}

int Mkusr::crear_Puntero_Triple(int &indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer){

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
        if(bloquePunteros->b_pointers[i] == -1){
            //No existe el bloque, crear uno y editarlo
            if(crear_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo, buffer) == 0){
                continue;
            }
        }else if(!esta_Lleno_Apuntador_Doble(bloquePunteros->b_pointers[i],superBloque,archivo)){
            //Editar el bloque de archivos solo si NO esta vacio
            if(crear_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo, buffer) == 0){
                continue;
            }
        }else
            //El bloque ya existe y esta lleno, buscar otro bloque
            continue;


        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        return 1;
    }

    //Todos los apuntadores existen y estan llenos, continuar buscando
    return 0;
}

int Mkusr::isUser(char *buffer){
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

int Mkusr::existsUser(char *buffer, char *newNameGroup){
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

        while(*aux != ','){
            aux++;
        }

        aux++;
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

int Mkusr::esta_Lleno_Bloque_Archivo(int indice_bloque_archivo, struct superBlock *superBloque, FILE *archivo){
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_archivo * superBloque->s_block_size,SEEK_SET);
    fread(file,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 64; i++){
        if(file->b_content[i] == '\0')
            return 0;
    }
    return 1;
}

int Mkusr::esta_Lleno_Apuntador_Simple(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo){
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

int Mkusr::esta_Lleno_Apuntador_Doble(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *pointers = new struct pointersBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_punteros_simple * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(pointers->b_pointers[i] == -1)
            return 0;
        if(!esta_Lleno_Apuntador_Simple(pointers->b_pointers[i],superBloque,archivo))
            return 0;
    }
    return 1;
}

int Mkusr::isGroup(char *buffer){
    if(buffer[0] == '0')
        return 0;

    char *aux = &buffer[0];
    while(*aux != ','){
        aux++;
    }
    aux++;

    //Obtener el tipo del usuario/grupo
    char tipo = *aux;

    if(tipo == 'G' || tipo == 'g'){
        return 1;
    } else {
        return 0;
    }
}

int Mkusr::existsGroup(char *buffer, char *newNameGroup){
    if(buffer[0] == '0')
        return 0;

    char *aux = &buffer[0];
    while(*aux != ','){
        aux++;
    }
    aux++;

    //Obtener el tipo del usuario/grupo
    char tipo = *aux;

    if(tipo == 'G' || tipo == 'g'){
        aux++;
        aux++;

        char grp[16]; grp[0] = '\0';
        int contador = 0;
        while(*aux != '\n'){
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

