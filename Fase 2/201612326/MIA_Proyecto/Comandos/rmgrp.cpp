#include "rmgrp.h"

Rmgrp::Rmgrp()
{

}

int Rmgrp::ejecutarRmgrp(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

    if(!usuario->active){
        std::cout<<"No hay ningun usuario logueado"<<std::endl;
        return 0;
    } else if(usuario->uid != 1){
        std::cout<<"El comando RMGRP solo puede ser utilizado por el usuario ROOT"<<std::endl;
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
                if(recorrer_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_NAME].valor, usuario, contador_registros, BUSCAR)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 12){
                if(recorrer_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_NAME].valor, usuario, contador_registros, BUSCAR)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 13){
                if(recorrer_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_NAME].valor, usuario, contador_registros, BUSCAR)){
                    encontrado = 1;
                    break;
                }
            } else if (i == 14){
                if(recorrer_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_NAME].valor, usuario, contador_registros, BUSCAR)){
                    encontrado = 1;
                    break;
                }
            }
        }
    }

    if(!encontrado){
        //Actualizar la hora de lectura
        time_t hora = time(0);
        inodeUsers->i_atime = *localtime(&hora);
        fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
        fwrite(inodeUsers,sizeof(inodeTable),1,archivo);
        fclose(archivo);

        std::cout<<"El grupo indicado no existe"<<std::endl;
        return 0;
    }

    //Buscar el id del grupo a eliminar
    buffer = new char[100];
    buffer[0] = '\0';
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(recorrer_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_NAME].valor, usuario, contador_registros, BORRAR_GRUPO)){
                    break;
                }
            } else if (i == 12){
                if(recorrer_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_NAME].valor, usuario, contador_registros, BORRAR_GRUPO)){
                    break;
                }
            } else if (i == 13){
                if(recorrer_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_NAME].valor, usuario, contador_registros, BORRAR_GRUPO)){
                    break;
                }
            } else if (i == 14){
                if(recorrer_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, parametros[T_NAME].valor, usuario, contador_registros, BORRAR_GRUPO)){
                    break;
                }
            }
        }
    }

    //Actualizar la hora de modificacion
    time_t hora = time(0);
    inodeUsers->i_mtime = *localtime(&hora);
    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
    fwrite(inodeUsers,sizeof(inodeTable),1,archivo);
    fclose(archivo);

    std::cout<<"Grupo eliminado correctamente"<<std::endl;
    return 1;
}

int Rmgrp::recorrer_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *newGroupName, struct user *usuario, int &contador_registros, Opcion op){
    //Recuperar el fileBlock
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
    fread(file,sizeof(fileBlock),1,archivo);

    switch(op){
    case Rmgrp::BUSCAR:
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
                            if(existGroup(buffer, newGroupName)){

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
    case Rmgrp::BORRAR_GRUPO:
        {
            //Concatenar el contenido
            int primer_caracter_libre = strlen(buffer);
            for(size_t i = 0; i < 64;i++){
                if(file->b_content[i] == '\0')
                    break;

                if(contador_registros == 0){
                    while(file->b_content[i] != ','){
                        file->b_content[i] = '0';
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
        break;

    }



}

int Rmgrp::recorrer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, struct user *usuario, int &contador_registros, Opcion op){

    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Directo(pointers->b_pointers[i],superBloque,archivo,buffer,groupName,usuario,contador_registros,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Rmgrp::recorrer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, struct user *usuario, int &contador_registros, Opcion op){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Puntero_Simple(pointers->b_pointers[i],superBloque,archivo,buffer,groupName,usuario,contador_registros,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Rmgrp::recorrer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupName, struct user *usuario, int &contador_registros, Opcion op){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Puntero_Doble(pointers->b_pointers[i],superBloque,archivo,buffer,groupName,usuario,contador_registros,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Rmgrp::isGroup(char *buffer){
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

int Rmgrp::existGroup(char *buffer, char *newNameGroup){
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


