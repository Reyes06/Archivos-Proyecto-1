#include "ls.h"

Ls::Ls()
{

}

int Ls::ejecutarLs(struct comando comando, ListaMontaje *listaMontaje){

    std::map<TipoParametro, struct parametro> parametros = (*comando.parametros);

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

    //Truncar el archivo
    FILE *a = fopen("temp.txt","w");
    fclose(a);

    //Buscar el archivo/carpeta en el directorio de archivos y cambiarle permisos
    int resultado = visitar_Inodo(0,superBloque,archivo,parametros[T_RUTA].valor);
    fclose(archivo);
    if(!resultado){
       std::cout<<"El archivo '"<<parametros[T_RUTA].valor<<"' no se ha podido leer"<<std::endl;
       return 0;
    }

    std::ofstream fs("dot.txt");
    char buffer[65];
    std::ifstream fsCupero("temp.txt");

    char *nombreArchivo = &parametros[T_RUTA].valor[strlen(parametros[T_RUTA].valor)];
    while(nombreArchivo[0] != '/'){
        nombreArchivo--;
    }
    nombreArchivo++;

    fs << "digraph G {\n";
    fs << "rankdir=LR;\n";
    fs << "nodoArchivo [shape=\"plaintext\" label=<\n";
    fs << "<table>\n";
    fs << "<tr>\n";
    fs <<   "<td>Permisos</td>\n";
    fs <<   "<td>Owner</td>\n";
    fs <<   "<td>Grupo</td>\n";
    fs <<   "<td>Size</td>\n";
    fs <<   "<td>Fecha</td>\n";
    fs <<   "<td>Hora</td>\n";
    fs <<   "<td>Tipo</td>\n";
    fs <<   "<td>Name</td>\n";
    fs << "</tr>\n";
    while(fsCupero.getline(buffer,64,'\n')){
        fs << buffer;
    }
    fs << "</table>\n";
    fs << ">];\n";

    fs << "}";
    fs.close();

    if(verificarSubCadena(parametros[T_PATH].valor, ".pdf")){
        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpdf dot.txt -o ");
        strcat(instruccion,parametros[T_PATH].valor);
        system(instruccion);
    } else {
        char path[100]; path[0] = '\0';
        strcat(path, parametros[T_PATH].valor);
        strcat(path, ".pdf");
        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpdf dot.txt -o ");
        strcat(instruccion,path);
        system(instruccion);
    }

    return 1;
}

int Ls::visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo,  char *path){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    path++;

    if(path[0] == '\0'){
        leer_Inodo(indice_inodo,superBloque,archivo,"/");
        return 1;
    }

    int encontrado = 0;
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                encontrado = visitar_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, path);
            } else if(i < 12){
                encontrado = visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, path);
            } else if(i == 12){
                encontrado = visitar_Puntero_Simple(inodo->i_block[i], inodo->i_type, superBloque, archivo, path);
            } else if(i == 13){
                encontrado = visitar_Puntero_Doble(inodo->i_block[i], inodo->i_type, superBloque, archivo, path);
            } else if(i == 14){
                encontrado = visitar_Puntero_Triple(inodo->i_block[i], inodo->i_type, superBloque, archivo, path);
            }

            if(encontrado == 1)
                return 1; //Se cambiaron los permisos correctamente

            if(encontrado == 0)
                continue; //No se ha hallado el archivo, continuear buscando
        }
    }
    std::cout<<"No se encontro el archivo/carpeta '"<<path<<"'"<<std::endl;
    return 0;
}

int Ls::visitar_Bloque_Carpeta_Archivo(Ignorar condicion, int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path){
    struct folderBlock *folder = new struct folderBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
    fread(folder,sizeof(folderBlock),1,archivo);

    //Setear el valor de inicio dependiendo de ignorar o no los primeros registros del folder
    int inicio;
    switch(condicion){
    case NO_IGNORAR_PRIMEROS_REGISTROS:
        inicio = 0;
        break;
    case IGNORAR_PRIMEROS_REGISTROS:
        inicio = 2;
        break;
    }

    //buscar el folderBlock
    for(int i = inicio; i < 4; i++){
        if(folder->b_content[i].b_inode == -1){
            continue;
        } else {
            if(startsWith(path,folder->b_content[i].b_name)){

                if(isFile(path)){
                    //Archivo

                    leer_Inodo(folder->b_content[i].b_inode,superBloque,archivo,folder->b_content[i].b_name);
                    return 1;
                } else {
                    //Carpeta

                    while(path[0] != '/' && path[0] != '\0')
                        path++;

                    if(path[0] == '\0'){
                        //Hacer el reporte a partir de esta carpeta
                        leer_Inodo(folder->b_content[i].b_inode,superBloque,archivo,folder->b_content[i].b_name);
                        return 1;
                    } else {
                        //Continuar buscando
                        return visitar_Inodo(folder->b_content[i].b_inode,superBloque,archivo,path);
                    }


                }
            }
        }
    }
    return 0;
}

int Ls::visitar_Puntero_Simple(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],i_type,superBloque,archivo,path)){
            return 1;
        }
    }
    return 0;
}

int Ls::visitar_Puntero_Doble(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Simple(bloquePunteros->b_pointers[i],i_type,superBloque,archivo,path)){
            return 1;
        }
    }
    return 0;
}

int Ls::visitar_Puntero_Triple(int indice_bloque, int i_type, struct superBlock *superBloque, FILE *archivo,  char *path){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        if(visitar_Puntero_Doble(bloquePunteros->b_pointers[i],i_type,superBloque,archivo,path)){
            return 1;
        }
    }
    return 0;
}

void Ls::leer_Inodo(int index_inode,struct superBlock *superBloque, FILE *archivo, char *nombreArchivoCarpeta){
    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + index_inode * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    //OBTENER REGISTROLS

    struct filaLs registro = obtenerRegistroLs(superBloque,archivo,inodo,nombreArchivoCarpeta);
    std::ofstream fs("temp.txt",std::ofstream::app);
    fs << "<tr>\n";
    fs <<   "<td>"<<registro.permisos<<"</td>\n";
    fs <<   "<td>"<<registro.owner<<"</td>\n";
    fs <<   "<td>"<<registro.grupo<<"</td>\n";
    fs <<   "<td>"<<registro.size<<"</td>\n";
    fs <<   "<td>"<<registro.fecha<<"</td>\n";
    fs <<   "<td>"<<registro.hora<<"</td>\n";
    fs <<   "<td>"<<registro.tipo<<"</td>\n";
    fs <<   "<td>"<<registro.name<<"</td>\n";
    fs << "</tr>\n";
    fs.close();

    if(inodo->i_type == 1)  //Inodo de un archivo
        return;


    //Recorrer el inodo
    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                leer_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo);
            } else if(i < 12){
                leer_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], superBloque, archivo);
            } else if(i == 12){
                leer_Puntero_Simple(inodo->i_block[i], superBloque, archivo);
            } else if(i == 13){
                leer_Puntero_Doble(inodo->i_block[i], superBloque, archivo);
            } else if(i == 14){
                leer_Puntero_Triple(inodo->i_block[i], superBloque, archivo);
            }
        }
    }
}

void Ls::leer_Bloque_Carpeta_Archivo(Ignorar ignorar, int indice_bloque, struct superBlock *superBloque, FILE *archivo){

    struct folderBlock *folder = new struct folderBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(folderBlock),SEEK_SET );
    fread(folder,sizeof(folderBlock),1,archivo);

    int inicio;
    switch(ignorar){
    case Ls::IGNORAR_PRIMEROS_REGISTROS:
        inicio = 2;
        break;
    case Ls::NO_IGNORAR_PRIMEROS_REGISTROS:
        inicio = 0;
        break;

    }

    for(int i = inicio; i < 4; i++){
        if(folder->b_content[i].b_inode == -1){
            continue;
        }
        leer_Inodo(folder->b_content[i].b_inode,superBloque,archivo,folder->b_content[i].b_name);
    }
}

void Ls::leer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        leer_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, bloquePunteros->b_pointers[i],superBloque,archivo);
    }
}

void Ls::leer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        leer_Puntero_Simple(bloquePunteros->b_pointers[i],superBloque,archivo);
    }
}

void Ls::leer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo){
    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        leer_Puntero_Doble(bloquePunteros->b_pointers[i],superBloque,archivo);
    }
}


int Ls::startsWith(char *pathCompleto, char *fileFolderuserGroupName){
    while( *pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == *fileFolderuserGroupName){
            pathCompleto++;
            fileFolderuserGroupName++;
        } else {
            return 0;
        }
    }
    return 1;
}

int Ls::isFile(char *pathCompleto){
    while(*pathCompleto != '/' && *pathCompleto != '\0'){
        if(*pathCompleto == '.'){
            return 1;
        }
        pathCompleto++;
    }
    return 0;
}

int Ls::verificarSubCadena( char *cadena, char *subcadena )
{
   char *tmp = cadena;
   char *pdest;

   pdest = strstr( tmp, subcadena );
   if( pdest ) return 1;

   return 0;
}

struct Ls::filaLs Ls::obtenerRegistroLs(struct superBlock *superBloque, FILE *archivo, struct inodeTable *inodo, char *fileFolderName){

    struct filaLs registro;

    //PERMISOS
    const char *ugo = std::to_string(inodo->i_perm).c_str();
    char u = ugo[0];
    char g = ugo[1];
    char o = ugo[2];

    switch(u){
    case '0':
        registro.permisos[0] = '-';
        registro.permisos[1] = '-';
        registro.permisos[2] = '-';
        break;
    case '1':
        registro.permisos[0] = '-';
        registro.permisos[1] = '-';
        registro.permisos[2] = 'x';
        break;
    case '2':
        registro.permisos[0] = '-';
        registro.permisos[1] = 'w';
        registro.permisos[2] = '-';
        break;
    case '3':
        registro.permisos[0] = '-';
        registro.permisos[1] = 'w';
        registro.permisos[2] = 'x';
        break;
    case '4':
        registro.permisos[0] = 'r';
        registro.permisos[1] = '-';
        registro.permisos[2] = '-';
        break;
    case '5':
        registro.permisos[0] = 'r';
        registro.permisos[1] = '-';
        registro.permisos[2] = 'x';
        break;
    case '6':
        registro.permisos[0] = 'r';
        registro.permisos[1] = 'w';
        registro.permisos[2] = '-';
        break;
    case '7':
        registro.permisos[0] = 'r';
        registro.permisos[1] = 'w';
        registro.permisos[2] = 'x';
        break;
    }

    switch(g){
    case '0':
        registro.permisos[3] = '-';
        registro.permisos[4] = '-';
        registro.permisos[5] = '-';
        break;
    case '1':
        registro.permisos[3] = '-';
        registro.permisos[4] = '-';
        registro.permisos[5] = 'x';
        break;
    case '2':
        registro.permisos[3] = '-';
        registro.permisos[4] = 'w';
        registro.permisos[5] = '-';
        break;
    case '3':
        registro.permisos[3] = '-';
        registro.permisos[4] = 'w';
        registro.permisos[5] = 'x';
        break;
    case '4':
        registro.permisos[3] = 'r';
        registro.permisos[4] = '-';
        registro.permisos[5] = '-';
        break;
    case '5':
        registro.permisos[3] = 'r';
        registro.permisos[4] = '-';
        registro.permisos[5] = 'x';
        break;
    case '6':
        registro.permisos[3] = 'r';
        registro.permisos[4] = 'w';
        registro.permisos[5] = '-';
        break;
    case '7':
        registro.permisos[3] = 'r';
        registro.permisos[4] = 'w';
        registro.permisos[5] = 'x';
        break;
    }

    switch(o){
    case '0':
        registro.permisos[6] = '-';
        registro.permisos[7] = '-';
        registro.permisos[8] = '-';
        break;
    case '1':
        registro.permisos[6] = '-';
        registro.permisos[7] = '-';
        registro.permisos[8] = 'x';
        break;
    case '2':
        registro.permisos[6] = '-';
        registro.permisos[7] = 'w';
        registro.permisos[8] = '-';
        break;
    case '3':
        registro.permisos[6] = '-';
        registro.permisos[7] = 'w';
        registro.permisos[8] = 'x';
        break;
    case '4':
        registro.permisos[6] = 'r';
        registro.permisos[7] = '-';
        registro.permisos[8] = '-';
        break;
    case '5':
        registro.permisos[6] = 'r';
        registro.permisos[7] = '-';
        registro.permisos[8] = 'x';
        break;
    case '6':
        registro.permisos[6] = 'r';
        registro.permisos[7] = 'w';
        registro.permisos[8] = '-';
        break;
    case '7':
        registro.permisos[6] = 'r';
        registro.permisos[7] = 'w';
        registro.permisos[8] = 'x';
        break;
    }

    registro.permisos[9] = '\0';

    //OWNER
   buscar_Usuario_Grupo(superBloque,archivo,registro.owner,inodo->i_uid,BUSCAR_USUARIO);

   //GRUPO
   buscar_Usuario_Grupo(superBloque,archivo,registro.grupo,inodo->i_uid,BUSCAR_GRUPO);

   //SIZE
   registro.size = inodo->i_size;

   //FECHA MODIFICACION
   strftime(registro.fecha, sizeof(registro.fecha), "%Y-%m-%d", &inodo->i_atime);

   //HORA MODIFICACION
   strftime(registro.hora, sizeof(registro.hora), "%X", &inodo->i_atime);

   //TIPO
   if(inodo->i_type){
       strcpy(registro.tipo,"Archivo");
   } else {
       strcpy(registro.tipo, "Carpeta");
   }

   //NOMBRE
   strcpy(registro.name,fileFolderName);

   return registro;

}

int Ls::buscar_Usuario_Grupo(struct superBlock *superBloque, FILE *archivo, char *groupUserName, int &userGroupId, Opcion op){
    //Recuperar el inodo del archivo users.txt
    struct inodeTable *inodeUsers = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + 1*superBloque->s_inode_size,SEEK_SET);
    fread(inodeUsers,sizeof(inodeTable),1,archivo);

    //Buscar el id del grupo a eliminar
    char *buffer = new char[100];
    buffer[0] = '\0';
    for(int i = 0; i < 15; i++){
        if(inodeUsers->i_block[i] == -1){
            continue;
        } else {
            if(i < 12){
                if(buscar_Directo(inodeUsers->i_block[i], superBloque, archivo, buffer, groupUserName, userGroupId, op)){
                    return 1;
                }
            } else if (i == 12){
                if(buscar_Puntero_Simple(inodeUsers->i_block[i], superBloque, archivo, buffer, groupUserName, userGroupId, op)){
                    return 1;
                }
            } else if (i == 13){
                if(buscar_Puntero_Doble(inodeUsers->i_block[i], superBloque, archivo, buffer, groupUserName, userGroupId, op)){
                    return 1;
                }
            } else if (i == 14){
                if(buscar_Puntero_Triple(inodeUsers->i_block[i], superBloque, archivo, buffer, groupUserName, userGroupId, op)){
                    return 1;
                }
            }
        }
    }

    return 0;
}

int Ls::buscar_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupUserName, int &userGroupId, Opcion op){
    //Recuperar el fileBlock
    struct fileBlock *file = new struct fileBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET );
    fread(file,sizeof(fileBlock),1,archivo);

    switch(op){
    case Ls::BUSCAR_USUARIO:
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

                    if(getUser(buffer, groupUserName, userGroupId)){

                        //Grupo encontrado
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
    case Ls::BUSCAR_GRUPO:
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

                    if(getGroup(buffer, groupUserName, userGroupId)){

                        //Grupo encontrado
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

int Ls::buscar_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupUserName, int &userGroupId, Opcion op){

    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //buscar los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(buscar_Directo(pointers->b_pointers[i],superBloque,archivo,buffer,groupUserName,userGroupId,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Ls::buscar_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupUserName, int &userGroupId, Opcion op){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //buscar los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(buscar_Puntero_Simple(pointers->b_pointers[i],superBloque,archivo,buffer,groupUserName,userGroupId,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Ls::buscar_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *&buffer, char *groupUserName, int &userGroupId, Opcion op){
    //Recuperar el pointersBlock
    struct pointersBlock * pointers = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice_bloque * superBloque->s_block_size,SEEK_SET);
    fread(pointers,sizeof(pointersBlock),1,archivo);

    //buscar los indices del puntero
    for(int i = 0; i < 16; i ++){
        if(pointers->b_pointers[i] == -1){
            continue;
        } else {
            if(buscar_Puntero_Doble(pointers->b_pointers[i],superBloque,archivo,buffer,groupUserName,userGroupId,op)){
                return 1;
            }
        }
    }
    return 0;
}

int Ls::isGroup(char *buffer){
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

int Ls::getGroup(char *buffer, char *userGroupName, int userGroupId){
    if(buffer[0] == '0')
        return 0;

    char id[5];
    int contador = 0;
    char *aux = &buffer[0];
    while(*aux != ','){
        id[contador] = *aux;
        aux++;
        contador++;
    }
    aux++;
    id[contador] = '\0';


    if(atoi(id) != userGroupId){
        //El id no pertenece a este grupo
        return 0;
    }

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

        strcpy(userGroupName,grp);
        return 1;

        return 0;
    } else {
        return 0;
    }
}

int Ls::isUser(char *buffer){
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

int Ls::getUser(char *buffer, char *userGroupName, int userGroupId){
    if(buffer[0] == '0')
        return 0;

    char id[5];
    int contador = 0;
    char *aux = &buffer[0];
    while(*aux != ','){
        id[contador] = *aux;
        aux++;
        contador++;
    }
    aux++;

    id[contador] = '\0';


    if(atoi(id) != userGroupId){
        //El id no pertenece a este usuario
        return 0;
    }

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

        strcpy(userGroupName,grp);
        return 1;

    } else {
        return 0;
    }
}
