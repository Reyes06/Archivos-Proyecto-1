#include "chgrp.h"

Chgrp::Chgrp()
{

}

int Chgrp::ejecutarChgrp(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

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
    int resultado = change_User_Group(parametros[T_USR].valor, parametros[T_GRP].valor, superBloque,archivo,usuario);

    fclose(archivo);

    if(resultado == 1){
       std::cout<<"Permisos cambiados correctamente"<<std::endl;
       return 1;
    } else {
        std::cout<<"Los permisos no se han cambiado"<<std::endl;
        return 0;
    }
}

int Chgrp::change_User_Group(char *userName, char *groupName, struct superBlock *superBloque, FILE *archivo, struct user *usuario){

    //CORREGIR: Validar los permisos del usuario

    //Verificar que el grupo exista
    int gid = searchGroupId(groupName,superBloque,archivo);
    if(gid == 0){
        std::cout<<"El grupo '"<<groupName<<"' no existe"<<std::endl;
        return 0;
    }

    //Recuperar el inodo del archivo users.txt
    struct inodeTable *inodeUsers = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
    fread(inodeUsers,sizeof(inodeTable),1,archivo);

    //Buscar el el usuario
    char *buffer = new char[100];
    buffer[0] = '\0';
    int contador_registros = 0;
    int encontrado = 0;
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(buscar_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR_USUARIO)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 12){
                if(buscar_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR_USUARIO)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 13){
                if(buscar_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR_USUARIO)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 14){
                if(buscar_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR_USUARIO)){
                    encontrado = 1;
                    break;
                }
            }
        }
    }

    if(!encontrado){
        std::cout<<"El usuario '"<<userName<<"' no existe"<<std::endl;
        return 0;
    }

    //Modificar el grupo del usuario
    buffer = new char[100];
    buffer[0] = '\0';
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(modificar_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, groupName, contador_registros)){
                    return 1;
                }
            } else if (i == 12){
                if(modificar_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, groupName, contador_registros)){
                    return 1;
                }
            } else if (i == 13){
                if(modificar_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, groupName, contador_registros)){
                    return 1;
                }
            } else if (i == 14){
                if(modificar_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, groupName, contador_registros)){
                    return 1;
                }
            }
        }
    }

    return 0;
}

int Chgrp::modificar_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, int &contador_registros){
    //Recuperar el fileBlock
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
    fread(file,sizeof(fileBlock),1,archivo);

    int primer_caracter_libre = strlen(buffer);
    for(size_t i = 0; i < 64;i++){
        if(file->b_content[i] == '\0')
            break;

        if(contador_registros == 0){


            //Esquivar el identificador
            while(file->b_content[i] != ','){
                i++;
            }
            i=i+3;


            //Copiar el nuevo groupname
            while(groupName[0] != '\0'){
                file->b_content[i] = groupName[0];
                groupName++;
                i++;
            }

            fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
            fwrite(file,sizeof(fileBlock),1,archivo);

            return 1;



        }
        if(file->b_content[i] == '\n'){
            contador_registros--;
        }
    }

    buffer[primer_caracter_libre] = '\0';
    return 0;


}

int Chgrp::modificar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, int &contador_registros){

    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(modificar_Directo(pointers->b_pointers[i],superBloque,archivo,buffer,groupName,contador_registros)){
                return 1;
            }
        }
    }
    return 0;
}

int Chgrp::modificar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, int &contador_registros){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(modificar_Puntero_Simple(pointers->b_pointers[i],superBloque,archivo,buffer,groupName,contador_registros)){
                return 1;
            }
        }
    }
    return 0;
}

int Chgrp::modificar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, int &contador_registros){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(modificar_Puntero_Doble(pointers->b_pointers[i],superBloque,archivo,buffer,groupName,contador_registros)){
                return 1;
            }
        }
    }
    return 0;
}

int Chgrp::searchGroupId(char *userName, struct superBlock *superBloque, FILE *archivo){

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
                if(buscar_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR_GRUPO)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 12){
                if(buscar_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR_GRUPO)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 13){
                if(buscar_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR_GRUPO)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 14){
                if(buscar_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, BUSCAR_GRUPO)){
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
                if(uid = buscar_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, OBTENER_ID)){
                    return uid;
                }
            } else if (i == 12){
                if(uid = buscar_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, OBTENER_ID)){
                    return uid;
                }
            } else if (i == 13){
                if(uid = buscar_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, OBTENER_ID)){
                    return uid;
                }
            } else if (i == 14){
                if(uid = buscar_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, userName, contador_registros, OBTENER_ID)){
                    return uid;
                }
            }
        }
    }

    return 0;
}

int Chgrp::buscar_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op){
    //Recuperar el fileBlock
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
    fread(file,sizeof(fileBlock),1,archivo);

    switch(op){
    case Chgrp::BUSCAR_GRUPO:
        {
            int primer_caracter_libre = strlen(buffer);
            for(size_t i = 0; i < 64;i++){
                if(file->b_content[i] == '\0')
                    break;

                if(file->b_content[i] == '\n'){


                        buffer[primer_caracter_libre] = '\n';
                        buffer[primer_caracter_libre+1] = '\0';


                        if(isGroup(buffer)){
                            if(existsGroup(buffer, userName)){

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
    case Chgrp::BUSCAR_USUARIO:
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

                                //Usuario encontrado
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
    case Chgrp::OBTENER_ID:
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

int Chgrp::buscar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op){

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

int Chgrp::buscar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op){
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

int Chgrp::buscar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *userName, int &contador_registros, Opcion op){
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

int Chgrp::isGroup(char *buffer){
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

int Chgrp::existsGroup(char *buffer, char *newNameGroup){
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

int Chgrp::isUser(char *buffer){
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

int Chgrp::existsUser(char *buffer, char *newNameGroup){
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

