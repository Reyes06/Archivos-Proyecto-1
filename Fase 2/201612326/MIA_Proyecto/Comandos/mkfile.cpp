#include "mkfile.h"

Mkfile::Mkfile(){

}

int Mkfile::ejecutarMkfile(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

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
    if(!esFile){
        std::cout<<"El PATH debe hacer referencia a un archivo, no una carpeta"<<std::endl;
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
    int resultado;
    if(parametros[T_P].valor[0] == '1'){
         resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_CONT].valor,1,usuario,parametros[T_SIZE].valor);
    } else {
        resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_PATH].valor,parametros[T_CONT].valor,0,usuario,parametros[T_SIZE].valor);
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

    std::cout<<"No se que ha pasado :("<<std::endl;
    return 0;
}



int Mkfile::visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char p_param, struct user *usuario, char *fileSize){

    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    //Verificar que el inodo NO sea del tipo archivo
    if(inodo->i_type == 1){
        return 0;
    }

    //Quitar el '/' del path
    newFilePath++;
    int respuesta;

    //Verificar si el archivo se debe crear en el directorio actual o en un subdirectorio
    if(isFile(newFilePath)){

        //0. Verificar si se tienen permisos de escritura en la carpeta
        if(usuario->uid != -1){
            const char *permisos = std::to_string(inodo->i_perm).c_str();
            if(inodo->i_uid == usuario->gid){
                //Permisos de propietario
                char permiso = permisos[0];
                if(permiso == '0' || permiso == '1' || permiso == '4' || permiso == '5'){
                    std::cout<<"El usuario '"<<usuario->usr<<"' no posee permisos de escritura sobre la carpeta padre"<<std::endl;
                    return 0;
                }
            } else if(inodo->i_gid == usuario->gid){
                //Permisos de grupo
                char permiso = permisos[1];
                if(permiso == '0' || permiso == '1' || permiso == '4' || permiso == '5'){
                    std::cout<<"El usuario '"<<usuario->usr<<"' no posee permisos de escritura sobre la carpeta padre"<<std::endl;
                    return 0;
                }
            } else {
                //Permisos de otros
                char permiso = permisos[2];
                if(permiso == '0' || permiso == '1' || permiso == '4' || permiso == '5'){
                    std::cout<<"El usuario '"<<usuario->usr<<"' no posee permisos de escritura sobre la carpeta padre"<<std::endl;
                    return 0;
                }
            }
        }


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
                if(respuesta == 1)
                    return 1;  //El archivo ya existe, se reemplazo
                if(respuesta == 0)
                    continue;   //No se ha encontrado el archivo, continuar buscando
            }
        }

        //2. Crear el archivo en el directorio actual
        for(int i = 0; i < 15; i++) {

            if(i == 0){
                respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
            } else if(i < 12){
                respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
            } else if(i == 12){
                respuesta = visitar_Puntero_Simple_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
            } else if(i == 13){
                respuesta = visitar_Puntero_Doble_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
            } else if(i == 14){
                respuesta = visitar_Puntero_Triple_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
            }

            //Validar respuestas
            if(respuesta == 1){
                //Actualizar la hora de lectura
                time_t hora = time(0);
                inodo->i_mtime = *localtime(&hora);
                fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
                fwrite(inodo,sizeof(inodeTable),1,archivo);
                return 1;  //El archivo se ha creado satisfactoriamente
            }

            if(respuesta == 0)
                continue;   //No se ha encontrado el archivo, continuar buscando

        }

        std::cout<<"No existen apuntadores para almacenar el archivo en el inodo "<<indice_inodo<<std::endl;
        return 0;

    } else {
        //REVISAR DE AQUI EN ADELANTE

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
            //No existe la carpeta, hay que crearla
            for(int i = 0; i < 15; i++) {
                if(inodo->i_block[i] == -1){

                    //Crear nuevo foldeBlock en esta posicion y repetir la iteracion
                    char *NADA = "";
                    if(i < 12){
                        crear_Bloque_Directo(inodo->i_block[i],superBloque,archivo,NADA,inodo->i_type);
                    } else if (i == 12){
                        crear_Puntero_Simple(inodo->i_block[i],superBloque,archivo,NADA,inodo->i_type);
                    } else if (i == 13){
                        crear_Puntero_Doble(inodo->i_block[i],superBloque,archivo,NADA,inodo->i_type);
                    } else if (i == 14){
                        crear_Puntero_Triple(inodo->i_block[i],superBloque,archivo,NADA,inodo->i_type);
                    }

                    //Guardar el inodo porque ha sido modificado el valor 'inodo->i_block[i]'
                    fseek(archivo,superBloque->s_inode_start + indice_inodo * superBloque->s_inode_size, SEEK_SET);
                    fwrite(inodo,sizeof(inodeTable),1,archivo);

                    //Regresar una iteracion anterior
                    i--;
                    continue;


                } else {

                    //Visitar sub arpetas/Archivos
                    if(i == 0){
                        respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                    } else if(i < 12){
                        respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                    } else if(i == 12){
                        respuesta = visitar_Puntero_Simple_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                    } else if(i == 13){
                        respuesta = visitar_Puntero_Doble_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                    } else if(i == 14){
                        respuesta = visitar_Puntero_Triple_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
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
        }

        if(!existe){
            std::cout<<"La carpeta '"<<newFilePath<<"' no existe"<<std::endl;
            return 0;
        }


        //2. Crear el archivo en un subdirectorio
        for(int i = 0; i < 15; i++) {
            if(inodo->i_block[i] == -1){
                continue;
            } else {
                //Visitar Carpetas/Archivos
                if(i == 0){
                    respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                } else if(i < 12){
                    respuesta = visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                } else if(i == 12){
                    respuesta = visitar_Puntero_Simple_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                } else if(i == 13){
                    respuesta = visitar_Puntero_Doble_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                } else if(i == 14){
                    respuesta = visitar_Puntero_Triple_CREAR(inodo->i_block[i], superBloque, archivo, newFilePath, contPath, p_param,usuario,indice_inodo,fileSize,0);
                }

                //Validar respuestas
                if(respuesta == 1)
                    return 1;  //Se ha creado el archivo en un subdirectorio
                if(respuesta == 0)
                    continue;   //No se ha encontrado el archivo, continuar buscando
            }
        }

        //Si ha llegado aqui es porque no encontro el archivo indicado en la ruta
        if(respuesta == -1 && p_param){
            //Crear el archivo

        }

        std::cout<<"La carpeta '"<<newFilePath<<"' no existe"<<std::endl;
        return 0;
    }
}

int Mkfile::visitar_Bloque_Carpeta_Archivo_EXISTE(Condicion condicion, int indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize){

    struct folderBlock *folder = new struct folderBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(folderBlock),SEEK_SET );
    fread(folder,sizeof(folderBlock),1,archivo);

    switch(condicion){
    case NO_IGNORAR_PRIMEROS_REGISTROS:
        for(int i = 0; i < 4; i++){
            if(folder->b_content[i].b_inode == -1){
                continue;
            }
            if(startsWith(newFilePath, folder->b_content[i].b_name)){
                if(isFile(folder->b_content[i].b_name)){

                    //Reemplazar el archivo
                    std::cout<<"El archivo '"<<folder->b_content[i].b_name<<"' ya existe en el directorio. Sera reemplazado."<<std::endl;

                    //Obtener el inodo del archivo
                    struct inodeTable *newInode = new struct inodeTable;
                    fseek(archivo,superBloque->s_inode_start + folder->b_content[i].b_inode * superBloque->s_inode_size,SEEK_SET);
                    fread(newInode,sizeof(inodeTable),1,archivo);

                    //Eliminar los bloques pertenecientes al inodo
                    for(int i = 0; i < 15;i++){
                        if(newInode->i_block[i] != -1){
                            char cero = 0;
                            fseek(archivo,superBloque->s_bm_block_start+newInode->i_block[i],SEEK_SET);
                            fwrite(&cero,sizeof(char),1,archivo);
                            newInode->i_block[i] = -1;
                        }
                    }

                    char uno = 1;

                    //Guardar la informacion del archivo 'cont'
                    if(contPath[0] != '\0'){
                        //Verificar si el archivo 'cont' existe
                        struct stat st = {0};
                        if (stat(contPath, &st) == -1){
                            std::cout<<"El archivo '"<<contPath<<"' no existe"<<std::endl;
                            std::cout<<"Se va a crear el archivo sin contenido"<<std::endl;
                        } else {
                            char *buffer = new char[64]; buffer[0] = '\0';
                            FILE *contFile = fopen(contPath,"r");
                            int size = 0;
                            int eof = 0;
                            int contador = 0;

                            while(!eof){


                                for(int i = 0; i < 64; i++){
                                    if(!fread(&buffer[i],sizeof(char),1,contFile)){
                                        eof = 1;
                                        buffer[i] = '\0';
                                        break;
                                    }
                                    size++;
                                }

                                if(buffer[0] == '\0')
                                    break;

                                if(contador < 12){
                                    if(crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 12){
                                    if(crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 13){
                                    if(crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 14){
                                    if(crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                }
                            }

                            newInode->i_size = size;

                            fclose(contFile);
                        }
                    } else if (fileSize[0] != '\0'){
                        int size = atoi(fileSize);
                        char *numeros = "0123456789";
                        int numberIndex = 0;
                        int bufferIndex = 0;
                        int contador = 0;
                        char *buffer = new char [64];
                        buffer[0] = '\0';
                        while(bufferIndex < size){
                            if(bufferIndex != 0 && bufferIndex % 64 == 0){


                                if(contador < 12){
                                    crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                                } else if (contador == 12){
                                    crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                                } else if (contador == 13){
                                    crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                                } else if (contador == 14){
                                    crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                                }
                                contador++;

                            }
                            if(numberIndex == 10){
                                numberIndex = 0;
                            }
                            buffer[bufferIndex % 64] = numeros[numberIndex];
                            numberIndex++;
                            bufferIndex++;
                        }

                        buffer[bufferIndex % 64] = '\0';

                        if(buffer[0] != '\0'){
                            if(contador < 12){
                                crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 12){
                                crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 13){
                                crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 14){
                                crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            }
                        }

                        newInode->i_size = size;

                    }

                    //Guardar el inodo del nuevo archivo
                    fseek(archivo,superBloque->s_inode_start + folder->b_content[i].b_inode * superBloque->s_inode_size, SEEK_SET);
                    fwrite(newInode,sizeof(inodeTable),1,archivo);

                }
                return 1;
            }
        }
        return 0;
    case IGNORAR_PRIMEROS_REGISTROS:
        for(int i = 2; i < 4; i++){
            if(folder->b_content[i].b_inode == -1){
                continue;
            }
            if(startsWith(newFilePath, folder->b_content[i].b_name)){
                if(isFile(folder->b_content[i].b_name)){
                    //Reemplazar el archivo
                    std::cout<<"El archivo '"<<folder->b_content[i].b_name<<"' ya existe en el directorio. Sera reemplazado."<<std::endl;

                    //Obtener el inodo del archivo
                    struct inodeTable *newInode = new struct inodeTable;
                    fseek(archivo,superBloque->s_inode_start + folder->b_content[i].b_inode * superBloque->s_inode_size,SEEK_SET);
                    fread(newInode,sizeof(inodeTable),1,archivo);

                    //Eliminar los bloques pertenecientes al inodo
                    for(int i = 0; i < 15;i++){
                        if(newInode->i_block[i] != -1){
                            char cero = 0;
                            fseek(archivo,superBloque->s_bm_block_start+newInode->i_block[i],SEEK_SET);
                            fwrite(&cero,sizeof(char),1,archivo);
                            newInode->i_block[i] = -1;
                        }
                    }

                    char uno = 1;

                    //Guardar la informacion del archivo 'cont'
                    if(contPath[0] != '\0'){
                        //Verificar si el archivo 'cont' existe
                        struct stat st = {0};
                        if (stat(contPath, &st) == -1){
                            std::cout<<"El archivo '"<<contPath<<"' no existe"<<std::endl;
                            std::cout<<"Se va a crear el archivo sin contenido"<<std::endl;
                        } else {
                            char *buffer = new char[64]; buffer[0] = '\0';
                            FILE *contFile = fopen(contPath,"r");
                            int eof = 0;
                            int contador = 0;
                            int size = 0;

                            while(!eof){


                                for(int i = 0; i < 64; i++){
                                    if(!fread(&buffer[i],sizeof(char),1,contFile)){
                                        eof = 1;
                                        buffer[i] = '\0';
                                        break;
                                    }
                                    size++;
                                }

                                if(buffer[0] == '\0')
                                    break;

                                if(contador < 12){
                                    if(crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 12){
                                    if(crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 13){
                                    if(crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 14){
                                    if(crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                }
                                contador++;
                            }

                            newInode->i_size = size;

                            fclose(contFile);
                        }
                    } else if (fileSize[0] != '\0'){
                        int size = atoi(fileSize);
                        char *numeros = "0123456789";
                        int numberIndex = 0;
                        int bufferIndex = 0;
                        int contador = 0;
                        char *buffer = new char [64];
                        buffer[0] = '\0';
                        while(bufferIndex < size){
                            if(bufferIndex != 0 && bufferIndex % 64 == 0){


                                if(contador < 12){
                                    crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                                } else if (contador == 12){
                                    crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                                } else if (contador == 13){
                                    crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                                } else if (contador == 14){
                                    crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                                }
                                contador++;

                            }
                            if(numberIndex == 10){
                                numberIndex = 0;
                            }
                            buffer[bufferIndex % 64] = numeros[numberIndex];
                            numberIndex++;
                            bufferIndex++;
                        }

                        buffer[bufferIndex % 64] = '\0';

                        if(buffer[0] != '\0'){
                            if(contador < 12){
                                crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 12){
                                crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 13){
                                crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 14){
                                crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            }
                        }

                        newInode->i_size = size;

                    }

                    //Guardar el inodo del nuevo archivo
                    fseek(archivo,superBloque->s_inode_start + folder->b_content[i].b_inode * superBloque->s_inode_size, SEEK_SET);
                    fwrite(newInode,sizeof(inodeTable),1,archivo);

                }
                return 1;
            }
        }
        return 0;
    }
}

int Mkfile::visitar_Puntero_Simple_EXISTE(int indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);


    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Bloque_Carpeta_Archivo_EXISTE(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,fileSize)){
            return 1;
        }
    }
    return 0;
}

int Mkfile::visitar_Puntero_Doble_EXISTE(int indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);


    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Simple_EXISTE(bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,fileSize)){
            return 1;
        }
    }
    return 0;
}

int Mkfile::visitar_Puntero_Triple_EXISTE(int indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char *contPath, char *fileSize){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);


    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Doble_EXISTE(bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,fileSize)){
            return 1;
        }
    }
    return 0;
}

int Mkfile::visitar_Bloque_Carpeta_Archivo_CREAR(Condicion condicion, int &block_index, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char* contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize, char tipo_inodo){

    struct folderBlock *folder;

    if(block_index == -1){
        int uno = 1;
        block_index = Getters::getIndexNewBlock(superBloque,archivo);
        folder = Getters::getNewFolderBlock();
        fseek(archivo, superBloque->s_bm_block_start + block_index,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + block_index * sizeof(pointersBlock),SEEK_SET);
        fwrite(folder,sizeof(pointersBlock),1,archivo);
    } else {
        folder = new struct folderBlock;
        fseek(archivo, superBloque->s_block_start + block_index * sizeof(pointersBlock),SEEK_SET);
        fread(folder,sizeof(pointersBlock),1,archivo);
    }

    //REVISAR DE AQUI EN ADELANTE

    if(isFile(newFilePath)){    //Archivo
        switch(condicion){
        case NO_IGNORAR_PRIMEROS_REGISTROS:
            for(int i = 0; i < 4; i++){
                if(folder->b_content[i].b_inode == -1){
                    int new_index_inode = Getters::getIndexNewInode(superBloque,archivo);
                    struct inodeTable *newInode = Getters::getNewFileInode(usuario->uid, usuario->gid);

                    //Copiar el nombre de la carpeta y su inodo de archivos
                    strcpy(folder->b_content[i].b_name,newFilePath);
                    folder->b_content[i].b_inode = new_index_inode;

                    char uno = 1;

                    //Actualizar el folderBlock
                    fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size, SEEK_SET);
                    fwrite(folder,sizeof(folderBlock),1,archivo);
                    fseek(archivo,superBloque->s_bm_inode_start + new_index_inode, SEEK_SET);
                    fwrite(&uno,sizeof(char),1,archivo);

                    if(contPath[0] != '\0'){
                        //Verificar si el archivo 'cont' existe
                        struct stat st = {0};
                        if (stat(contPath, &st) == -1){
                            std::cout<<"El archivo '"<<contPath<<"' no existe"<<std::endl;
                            std::cout<<"No se ha podido agregar informacion al archivo"<<std::endl;
                        } else {
                            //Guardar la informacion de la carpeta 'cont'
                            char *buffer = new char [64];
                            buffer[0] = '\0';
                            FILE *contFile = fopen(contPath,"r");
                            int eof = 0;
                            int contador = 0;
                            int size = 0;

                            while(!eof){

                                for(int i = 0; i < 64; i++){
                                    if(!fread(&buffer[i],sizeof(char),1,contFile)){
                                        eof = 1;
                                        buffer[i] = '\0';
                                        break;
                                    }
                                    i++;
                                }

                                if(buffer[0] == '\0')
                                    break;

                                if(contador < 12){
                                    if(crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 12){
                                    if(crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 13){
                                    if(crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 14){
                                    if(crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                }

                            }

                            newInode->i_size = size;

                            fclose(contFile);
                        }
                    } else if (fileSize[0] != '\0'){
                        int size = atoi(fileSize);
                        char *numeros = "0123456789";
                        int numberIndex = 0;
                        int bufferIndex = 0;
                        int contador = 0;
                        char *buffer = new char [64];
                        buffer[0] = '\0';
                        while(bufferIndex < size){
                            if(bufferIndex != 0 && bufferIndex % 64 == 0){
                                if(contador < 12){
                                    if(crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                    }
                                } else if (contador == 12){
                                    if(crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                    }
                                } else if (contador == 13){
                                    if(crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){

                                    }
                                } else if (contador == 14){
                                    if(crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                    }
                                }
                            }
                            if(numberIndex == 10){
                                numberIndex = 0;
                            }
                            buffer[bufferIndex % 64] = numeros[numberIndex];
                            numberIndex++;
                            bufferIndex++;
                        }

                        if(bufferIndex % 64 != 0)
                            buffer[bufferIndex % 64] = '\0';

                        if(buffer[0] != '\0'){
                            if(contador < 12){
                                crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 12){
                                crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 13){
                                crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 14){
                                crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            }
                        }

                        newInode->i_size = size;

                    }

                    //Guardar el inodo del nuevo archivo

                    fseek(archivo,superBloque->s_inode_start + new_index_inode * superBloque->s_inode_size, SEEK_SET);
                    fwrite(newInode,sizeof(inodeTable),1,archivo);


                    return 1;
                }
            }
            return 0;
        case IGNORAR_PRIMEROS_REGISTROS:
            for(int i = 2; i < 4; i++){
                if(folder->b_content[i].b_inode == -1){
                    int new_index_inode = Getters::getIndexNewInode(superBloque,archivo);
                    struct inodeTable *newInode = Getters::getNewFileInode(usuario->uid, usuario->gid);

                    //Copiar el nombre de la carpeta y su inodo de archivos
                    strcpy(folder->b_content[i].b_name,newFilePath);
                    folder->b_content[i].b_inode = new_index_inode;

                    char uno = 1;

                    //Actualizar el folderBlock
                    fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size, SEEK_SET);
                    fwrite(folder,sizeof(folderBlock),1,archivo);
                    fseek(archivo,superBloque->s_bm_inode_start + new_index_inode, SEEK_SET);
                    fwrite(&uno,sizeof(char),1,archivo);

                    //Guardar la informacion del archivo 'cont'
                    if(contPath[0] != '\0'){
                        //Verificar si el archivo 'cont' existe
                        struct stat st = {0};
                        if (stat(contPath, &st) == -1){
                            std::cout<<"El archivo '"<<contPath<<"' no existe"<<std::endl;
                            std::cout<<"Se va a crear el archivo sin contenido"<<std::endl;
                        } else {
                            char *buffer = new char[64]; buffer[0] = '\0';
                            FILE *contFile = fopen(contPath,"r");
                            int eof = 0;
                            int contador = 0;
                            int size = 0;

                            while(!eof){


                                for(int i = 0; i < 64; i++){
                                    if(!fread(&buffer[i],sizeof(char),1,contFile)){
                                        eof = 1;
                                        buffer[i] = '\0';
                                        break;
                                    }
                                    size++;
                                }

                                if(buffer[0] == '\0')
                                    break;

                                if(contador < 12){
                                    if(crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 12){
                                    if(crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 13){
                                    if(crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                } else if (contador == 14){
                                    if(crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                        continue;
                                    }
                                }
                            }

                            newInode->i_size = size;

                            fclose(contFile);
                        }
                    } else if (fileSize[0] != '\0'){
                        int size = atoi(fileSize);
                        char *numeros = "0123456789";
                        int numberIndex = 0;
                        int bufferIndex = 0;
                        int contador = 0;
                        char *buffer = new char [64];
                        buffer[0] = '\0';
                        while(bufferIndex < size){
                            if(bufferIndex != 0 && bufferIndex % 64 == 0){
                                if(contador < 12){
                                    if(crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                    }
                                } else if (contador == 12){
                                    if(crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                    }
                                } else if (contador == 13 ){
                                    if(crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                    }
                                } else if (contador == 14){
                                    if(crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type)){
                                        contador++;
                                    }
                                }
                            }
                            if(numberIndex == 10){
                                numberIndex = 0;
                            }
                            buffer[bufferIndex % 64] = numeros[numberIndex];
                            numberIndex++;
                            bufferIndex++;
                        }

                        if(bufferIndex % 64 != 0)
                            buffer[bufferIndex % 64] = '\0';

                        if(buffer[0] != '\0'){
                            if(contador < 12){
                                crear_Bloque_Directo(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 12){
                                crear_Puntero_Simple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 13){
                                crear_Puntero_Doble(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            } else if (contador == 14){
                                crear_Puntero_Triple(newInode->i_block[contador],superBloque,archivo,buffer,newInode->i_type);
                            }
                        }

                        newInode->i_size = size;

                    }

                    //Guardar el inodo del nuevo archivo

                    fseek(archivo,superBloque->s_inode_start + new_index_inode * superBloque->s_inode_size, SEEK_SET);
                    fwrite(newInode,sizeof(inodeTable),1,archivo);


                    return 1;
                }
            }
            return 0;
        }
    } else {                    //Carpeta

        //Buscar la carpeta
        switch(condicion){
        case NO_IGNORAR_PRIMEROS_REGISTROS:
            for(int i = 0; i < 4; i++){
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

                    return 1;

                } else if(startsWith(newFilePath, folder->b_content[i].b_name)){
                    while(newFilePath[0] != '/'){
                        newFilePath++;
                    }
                    return visitar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,newFilePath,contPath,p_param,usuario,fileSize);
                }
            }
            return 0;
        case IGNORAR_PRIMEROS_REGISTROS:
            for(int i = 2; i < 4; i++){
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

                    return 1;

                } else if(startsWith(newFilePath, folder->b_content[i].b_name)){
                    while(newFilePath[0] != '/'){
                        newFilePath++;
                    }
                    return visitar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,newFilePath,contPath,p_param,usuario,fileSize);
                }
            }
            return 0;
        }
    }

}

int Mkfile::visitar_Puntero_Simple_CREAR(int &indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath, char* contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize, char tipo_inodo){

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
            visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize,tipo_inodo);
        else {
            int res = visitar_Bloque_Carpeta_Archivo_CREAR(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize,tipo_inodo);
            if(res == 0){
                //El bloque ya existe y esta lleno, buscar otro bloque
                continue;
            }
        }



        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        return 1;
    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
    fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

    (bloquePunteros);
    return 0;
}

int Mkfile::visitar_Puntero_Doble_CREAR(int &indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath,char*contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize, char tipo_inodo){

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
            visitar_Puntero_Simple_CREAR(bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize,tipo_inodo);
        else {
            int res = visitar_Puntero_Simple_CREAR(bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize,tipo_inodo);
            if (res == 0){
                //El bloque ya existe y esta lleno, buscar otro bloque
                continue;
            }
        }

        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        return 1;
    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
    fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

    return 0;
}

int Mkfile::visitar_Puntero_Triple_CREAR(int &indice, struct superBlock *superBloque, FILE *archivo, char *newFilePath,char*contPath, char p_param, struct user *usuario, int inodo_index, char *fileSize, char tipo_inodo){

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
            visitar_Puntero_Doble_CREAR(bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize,tipo_inodo);
        else {
            int res= visitar_Puntero_Doble_CREAR(bloquePunteros->b_pointers[i],superBloque,archivo,newFilePath,contPath,p_param,usuario,inodo_index,fileSize,tipo_inodo);
            if(res == 0){
                //El bloque ya existe y esta lleno, buscar otro bloque
                continue;
            }
        }

        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
        fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

        return 1;
    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + indice * superBloque->s_block_size,SEEK_SET);
    fwrite(bloquePunteros,sizeof(pointersBlock),1,archivo);

    return 0;
}

int Mkfile::startsWith(char *pathCompleto, char *fileFolderName){
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

int Mkfile::crear_Bloque_Directo(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer, char tipo_inodo){

    if(tipo_inodo == 0){   //Carpeta

        char uno = 1;

        //Crear el nuevo folderBlock
        int new_index_folder_block = Getters::getIndexNewBlock(superBloque,archivo);
        struct folderBlock *newFolderBlock = Getters::getNewFolderBlock();
        fseek(archivo,superBloque->s_bm_block_start + new_index_folder_block, SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo,superBloque->s_block_start + new_index_folder_block * superBloque->s_block_size, SEEK_SET);
        fwrite(newFolderBlock,sizeof(folderBlock),1,archivo);

        //Guardar los cambios en el inodo del directorio actual
        indice_padre = new_index_folder_block;

        return 1;

    } else if (tipo_inodo == 1){   //Archivo
        //Obtener un indice para el nuevo bloque
        int block_index = Getters::getIndexNewBlock(superBloque,archivo);

        //Crear el fileBlock
        struct fileBlock *file = Getters::getNewFileBlock();

        strcpy(file->b_content,buffer);

        //Guardar el nuevo bloque
        char uno = 1;
        fseek(archivo,superBloque->s_bm_block_start + block_index,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo, superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET );
        fwrite(file,sizeof(fileBlock),1,archivo);

        //Guardar el indice del padre
        indice_padre = block_index;

        std::cout<<"Se ha creado el archivo correctamente"<<std::endl;
        return 1;
    }

}

int Mkfile::crear_Puntero_Simple(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer, char tipo_inodo){

    struct pointersBlock *pointers = new pointersBlock;
    int block_index;

    if(indice_padre == -1){
        //Obtener un indice para el nuevo bloque
        block_index = Getters::getIndexNewBlock(superBloque,archivo);

        //Crear el pointersBlock
        pointers = Getters::getNewPointersBlock();

        //Guardar el nuevo bloque
        char uno = 1;
        fseek(archivo,superBloque->s_bm_block_start + block_index,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fwrite(pointers,sizeof(pointersBlock),1,archivo);
    } else {
        block_index = indice_padre;
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fread(pointers,sizeof(pointersBlock),1,archivo);
    }


    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] != -1)
            continue;

        crear_Bloque_Directo(pointers->b_pointers[i],superBloque,archivo,buffer,tipo_inodo);

        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fwrite(pointers,sizeof(pointersBlock),1,archivo);

        //Guardar el indice del padre
        indice_padre = block_index;

        if(i == 15)
            return 1;
        else
            return 0;

    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
    fwrite(pointers,sizeof(pointersBlock),1,archivo);

    //Guardar el indice del padre
    indice_padre = block_index;

    return 1;
}

int Mkfile::crear_Puntero_Doble(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer, char tipo_inodo){

    struct pointersBlock *pointers = new pointersBlock;
    int block_index;

    if(indice_padre == -1){
        //Obtener un indice para el nuevo bloque
        block_index = Getters::getIndexNewBlock(superBloque,archivo);

        //Crear el pointersBlock
        pointers = Getters::getNewPointersBlock();

        //Guardar el nuevo bloque
        char uno = 1;
        fseek(archivo,superBloque->s_bm_block_start + block_index,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fwrite(pointers,sizeof(pointersBlock),1,archivo);
    } else {
        block_index = indice_padre;
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fread(pointers,sizeof(pointersBlock),1,archivo);
    }

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){

        if(pointers->b_pointers[i] == -1){
            crear_Puntero_Simple(pointers->b_pointers[i],superBloque,archivo,buffer,tipo_inodo);
        } else {
            if(estaLleno_Simple(pointers->b_pointers[i],superBloque,archivo,tipo_inodo)){
                    continue;
            } else {
                crear_Puntero_Simple(pointers->b_pointers[i],superBloque,archivo,buffer,tipo_inodo);
            }
        }

        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fwrite(pointers,sizeof(pointersBlock),1,archivo);

        //Guardar el indice del padre
        indice_padre = block_index;

        if(i == 15 && estaLleno_Simple(pointers->b_pointers[i],superBloque,archivo,tipo_inodo))
            return 1;
        else
            return 0;

    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
    fwrite(pointers,sizeof(pointersBlock),1,archivo);

    //Guardar el indice del padre
    indice_padre = block_index;

    return 1;
}

int Mkfile::crear_Puntero_Triple(int &indice_padre, struct superBlock *superBloque, FILE *archivo, char *&buffer, char tipo_inodo){

    struct pointersBlock *pointers = new pointersBlock;
    int block_index;

    if(indice_padre == -1){
        //Obtener un indice para el nuevo bloque
        block_index = Getters::getIndexNewBlock(superBloque,archivo);

        //Crear el pointersBlock
        pointers = Getters::getNewPointersBlock();

        //Guardar el nuevo bloque
        char uno = 1;
        fseek(archivo,superBloque->s_bm_block_start + block_index,SEEK_SET);
        fwrite(&uno,sizeof(char),1,archivo);
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fwrite(pointers,sizeof(pointersBlock),1,archivo);
    } else {
        block_index = indice_padre;
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fread(pointers,sizeof(pointersBlock),1,archivo);
    }

    //Recorrer los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            crear_Puntero_Doble(pointers->b_pointers[i],superBloque,archivo,buffer,tipo_inodo);
        } else {
            if(estaLleno_Doble(pointers->b_pointers[i],superBloque,archivo,tipo_inodo)){
                continue;
            } else {
                crear_Puntero_Doble(pointers->b_pointers[i],superBloque,archivo,buffer,tipo_inodo);
            }
        }

        //Guardar cualquier cambio que se haya hecho en el bloque
        fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
        fwrite(pointers,sizeof(pointersBlock),1,archivo);

        //Guardar el indice del padre
        indice_padre = block_index;

        return 0;

    }

    //Guardar cualquier cambio que se haya hecho en el bloque
    fseek(archivo,superBloque->s_block_start + block_index * superBloque->s_block_size,SEEK_SET);
    fwrite(pointers,sizeof(pointersBlock),1,archivo);

    //Guardar el indice del padre
    indice_padre = block_index;

    return 1;
}

void Mkfile::removeFolderName(char *&pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        pathCompleto++;
    }
}

int Mkfile::isFile(char *pathCompleto){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}


int Mkfile::estaLleno_Bloque_Archivo_Carpeta(int indice_bloque_carpeta, struct superBlock *superBloque, FILE *archivo, char tipo){

    if(tipo == 1){  //Archivo
        return 1;
    } else {        //Carpeta
        struct folderBlock *folder = new struct folderBlock;
        fseek(archivo,superBloque->s_block_start + indice_bloque_carpeta * superBloque->s_block_size,SEEK_SET);
        fread(folder,sizeof(pointersBlock),1,archivo);

        for(int i = 0; i < 4; i++){
            if(folder->b_content[i].b_inode == -1){
                (folder);
                return 0;
            }

        }
        (folder);
        return 1;
    }
}


int Mkfile::estaLleno_Simple(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo, char tipo){
    struct pointersBlock *pointers = new struct pointersBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_punteros_simple * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(pointers->b_pointers[i] == -1){
            (pointers);
            return 0;
        }

        if(!estaLleno_Bloque_Archivo_Carpeta(pointers->b_pointers[i],superBloque,archivo,tipo)){
            (pointers);
            return 0;
        }
    }
    (pointers);
    return 1;
}

int Mkfile::estaLleno_Doble(int indice_bloque_punteros_simple, struct superBlock *superBloque, FILE *archivo, char tipo){
    struct pointersBlock *pointers = new struct pointersBlock;
    fseek(archivo,superBloque->s_block_start + indice_bloque_punteros_simple * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(pointers->b_pointers[i] == -1){
            (pointers);
            return 0;
        }
        if(!estaLleno_Simple(pointers->b_pointers[i],superBloque,archivo,tipo)){
            (pointers);
            return 0;
        }
    }
    (pointers);
    return 1;
}


