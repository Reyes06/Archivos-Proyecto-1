#include "login.h"

Login::Login()
{

}

int Login::ejecutarLogin(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

    if(usuario->active){
        std::cout<<"El usuario '"<<usuario->usr<<"' se encuentra logueado"<<std::endl;
        return 0;
    }

    std::map<TipoParametro, struct parametro> parametros = (*comando.parametros);
    char *user = parametros[T_USR].valor;
    char *pwd = parametros[T_PWD].valor;

    //Verificar que la particion existe
    struct partition particion = listaMontaje->getPrimaryMountPartition(parametros[T_ID].valor);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado la particion '"<<parametros[T_ID].valor<<"'";
        return 0;
    }

    //Abrir el archivo
    char *disk_location = listaMontaje->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb+");


    //Recuperar el superbloque
    struct superBlock *superBloque = new struct superBlock;
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);

    //Recuperar el inodo del archivo users.txt
    struct inodeTable *inodeUsers = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
    fread(inodeUsers,sizeof(inodeTable),1,archivo);

    //Recuperar el contenido del archivo users.txt
    char buffer[100]; buffer[0] = '\0';
    int usuarioValidado = 0;
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(usuarioValidado = recorrer_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, user, pwd, usuario, VALIDAR_USUARIO))
                    break;;
            } else if (i == 12){
                if(usuarioValidado = recorrer_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, user, pwd, usuario, VALIDAR_USUARIO))
                    break;
            } else if (i == 13){
                if(usuarioValidado = recorrer_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, user, pwd, usuario, VALIDAR_USUARIO))
                    break;
            } else if (i == 14){
                if(usuarioValidado = recorrer_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, user, pwd, usuario, VALIDAR_USUARIO))
                    break;
            }
        }
    }

    if(!usuarioValidado){
        time_t hora = time(0);
        inodeUsers->i_atime = *localtime(&hora);

        fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
        fwrite(inodeUsers,sizeof(inodeTable),1,archivo);

        fclose(archivo);
        std::cout<<"User o password invalidos"<<std::endl;
        return 0;
    }

    //Obtener el nombre de la particion y el gid
    strcpy(usuario->partition_id,parametros[T_ID].valor);

    //Obtener el gid
    buffer[0] = '\0';
    usuarioValidado = 0;
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(recorrer_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, nullptr, nullptr, usuario, OBTENER_GID))
                    break;;
            } else if (i == 12){
                if(recorrer_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, nullptr, nullptr, usuario, OBTENER_GID))
                    break;
            } else if (i == 13){
                if(recorrer_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, nullptr, nullptr, usuario, OBTENER_GID))
                    break;
            } else if (i == 14){
                if(recorrer_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, nullptr, nullptr, usuario, OBTENER_GID))
                    break;
            }
        }
    }

    time_t hora = time(0);
    inodeUsers->i_atime = *localtime(&hora);

    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
    fwrite(inodeUsers,sizeof(inodeTable),1,archivo);

    //Cerrar el archivo
    fclose(archivo);
    return 1;
}

int Login::recorrer_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *buffer, char* user, char *pwd, struct user *usuario, Opcion dec){
    //Recuperar el fileBlock
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
    fread(file,sizeof(fileBlock),1,archivo);

    //Concatenar el contenido
    int primer_caracter_libre = strlen(buffer);
    for(size_t i = 0; i < strlen(file->b_content);i++){
        if(file->b_content[i] == '\n'){

            switch(dec){
            case Login::VALIDAR_USUARIO:
                buffer[primer_caracter_libre] = '\n';
                buffer[primer_caracter_libre+1] = '\0';
                if(validarUsuario(buffer,user,pwd,usuario)){
                    return 1;
                } else {
                    buffer[0] = '\0';
                    primer_caracter_libre = 0;
                }
                break;
            case Login::OBTENER_GID:
                buffer[primer_caracter_libre] = '\n';
                buffer[primer_caracter_libre+1] = '\0';
                if(obtenerGid(buffer,usuario)){
                    return 1;
                } else {
                    buffer[0] = '\0';
                    primer_caracter_libre = 0;
                }
                break;
            }


        } else {
            //Concatenar al buffer
            buffer[primer_caracter_libre] = file->b_content[i];
            primer_caracter_libre++;
        }
    }

    buffer[primer_caracter_libre] = '\0';
    return 0;
}

int Login::recorrer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *buffer, char* user, char *pwd, struct user *usuario, Opcion dec){

    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Directo(pointers->b_pointers[i],superBloque,archivo,buffer,user,pwd,usuario,dec)){
                return 1;
            }
        }
    }
    return 0;
}

int Login::recorrer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *buffer, char* user, char *pwd, struct user *usuario, Opcion dec){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Puntero_Simple(pointers->b_pointers[i],superBloque,archivo,buffer,user,pwd,usuario,dec)){
                return 1;
            }
        }
    }
    return 0;
}

int Login::recorrer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *buffer, char* user, char *pwd, struct user *usuario, Opcion dec){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(recorrer_Puntero_Doble(pointers->b_pointers[i],superBloque,archivo,buffer,user,pwd,usuario,dec)){
                return 1;
            }
        }
    }
    return 0;
}

int Login::validarUsuario(char *buffer, char *usr, char *pwd, struct user *registro_usuario){
    if(buffer[0] == '0')
        return 0;

    char usuario[16];
    char password[16];
    char grp[16];
    char uid[5];
    int i;

    char *aux = &buffer[0];

    //Copiar el id del usuario/grupo
    i = 0;
    while(*aux != ','){
        uid[i] = *aux;
        aux++;
        i++;
    }
    uid[i] = '\0';
    aux++;

    //Obtener el tipo del usuario/grupo
    char tipo = *aux;
    aux++;
    aux++;


    if(tipo == 'G' || tipo == 'g'){
        return 0;
    } else {

        //Copiar el nombre del grupo al que pertenece
        i = 0;
        while(*aux != ','){
            grp[i] = *aux;
            aux++;
            i++;
        }
        grp[i] = '\0';
        aux++;



        //Copiar el usuario al vector
        i = 0;
        while(*aux != ','){
            usuario[i] = *aux;
            aux++;
            i++;
        }
        usuario[i] = '\0';
        aux++;

        //Copiar el password al vector
        i = 0;
        while(*aux != '\n'){
            password[i] = *aux;
            aux++;
            i++;
        }
        password[i] = '\0';

        //Validar el usuario
        if(strcmp(usuario,usr) == 0 && strcmp(password,pwd) == 0){
            std::cout<<"Usuario logueado"<<std::endl;
            strcpy(registro_usuario->usr,usr);
            strcpy(registro_usuario->pwd,pwd);
            strcpy(registro_usuario->grp,grp);
            registro_usuario->uid = atoi(uid);
            registro_usuario->active = 1;
            return 1;
        } else {
            return 0;
        }
    }
}

int Login::obtenerGid(char *buffer, struct user *registro_usuario){
    if(buffer[0] == '0')
        return 0;

    char grp[16];
    char gid[5];
    int i;

    char *aux = &buffer[0];

    //Copiar el id del usuario/grupo
    i = 0;
    while(*aux != ','){
        gid[i] = *aux;
        aux++;
        i++;
    }
    gid[i] = '\0';
    aux++;

    //Obtener el tipo del usuario/grupo
    char tipo = *aux;
    aux++;
    aux++;


    if(tipo == 'U' || tipo == 'u'){
        return 0;
    } else {
        //Copiar el nombre del grupo
        i = 0;
        while(*aux != '\n'){
            grp[i] = *aux;
            aux++;
            i++;
        }
        grp[i] = '\0';
        aux++;

        //Validar el grpo
        if(strcmp(grp,registro_usuario->grp) == 0){
            registro_usuario->gid = atoi(gid);
            return 1;
        }
        return 0;
    }
}
