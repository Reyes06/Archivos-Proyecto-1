#include "exec.h"

int ejecutarExec(struct comando com, ListaMontaje *listaMontaje){

    std::map<TipoParametro, struct parametro> parametros = *(com.parametros);

    //1. Verificar si el archivo existe
    struct stat st = {0};
    if (stat(parametros[T_PATH].valor, &st) == -1){
        std::cout<<"El archivo '"<<parametros[T_PATH].valor<<"' no existe"<<std::endl;
        return 0;
    }

    char entrada[150];
    std::ifstream fs(parametros[T_PATH].valor);
    while(fs.getline(entrada,150,'\n')){
        if(entrada[0] == '\t' || entrada[0] == '\n'|| entrada[0] == '\r' || entrada[0] == '\f'){
            continue;
        }
        std::cout<<"==============================================================================="<<std::endl;
        std::cout<<entrada<<std::endl;

        if(parsearArchivo(entrada) != 0){
            for(int i = 0; i < 20; i++){
                parsearArchivo("#CHAPUZ PARA LIMPIAR LA ENTRADA DEL PARSER\n");
            }
            std::cout<<"No se pudo parsear el comando"<<std::endl;
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
            continue;
        }

        //1.5 Validar si es comentario
        if(comando.tipo == T_COMENTARIO){
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
            continue;
        }

        //2. Validar parametros del comando
        if(!validar(comando)){
            std::cout<<"Parámetros no válidos"<<std::endl;
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
            continue;
        }

        //3. Ejecutar comando
        if(ejecutar(comando, listaMontaje)){
            std::cout<<"Finalizado correctamente"<<std::endl;
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
        } else {
            std::cout<<"Finalizado con errores"<<std::endl;
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
        }
    }
    fs.close();
    return 1;
}

int parsearArchivo(char *entrada){
    FILE *archivo = fopen("temp.txt","w+");
    fwrite(entrada,strlen(entrada),1,archivo);
    fclose(archivo);

    yyin = fopen("temp.txt","rt");
    int parserResult= yyparse();
    fclose(yyin);
    return parserResult;
}

int validar(struct comando comando){

    std::map<TipoParametro, struct parametro> parametros = *(comando.parametros);

    switch(comando.tipo){
    case T_MKDISK:
        if(!parametros[T_SIZE].valor[0]){
            std::cout<<"El comando necesita el parametro SIZE"<<std::endl;
            return 0;
        }
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        } else {
            for(size_t i = 0; i < strlen(parametros[T_PATH].valor); i++){
                if(parametros[T_PATH].valor[i] == '.'){
                    return 1;
                }
            }
            std::cout<<"El PATH debe hacer referencia a un archivo"<<std::endl;
            return 0;
        }
        return 1;
    case T_RMDISK:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        } else {
            for(size_t i = 0; i < strlen(parametros[T_PATH].valor); i++){
                if(parametros[T_PATH].valor[i] == '.'){
                    return 1;
                }
            }
            std::cout<<"El PATH debe hacer referencia a un archivo"<<std::endl;
            return 0;
        }
        return 1;
    case T_FDISK:
        if (!parametros[T_NAME].valor[0]){
            std::cout<<"El comando necesita el parametro NAME"<<std::endl;
            return 0;
        }
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        } else {
            for(size_t i = 0; i < strlen(parametros[T_PATH].valor); i++){
                if(parametros[T_PATH].valor[i] == '.'){
                    return 1;
                }
            }
            std::cout<<"El PATH debe hacer referencia a un archivo"<<std::endl;
            return 0;
        }
        return 1;
    case T_MOUNT:
        if (!parametros[T_NAME].valor[0]){
            std::cout<<"El comando necesita el parametro NAME"<<std::endl;
            return 0;
        }
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        } else {
            for(size_t i = 0; i < strlen(parametros[T_PATH].valor); i++){
                if(parametros[T_PATH].valor[i] == '.'){
                    return 1;
                }
            }
            std::cout<<"El PATH debe hacer referencia a un archivo"<<std::endl;
            return 0;
        }
        return 1;
    case T_UNMOUNT:
        if (!parametros[T_ID].valor[0]){
            std::cout<<"El comando necesita el parametro ID"<<std::endl;
            return 0;
        }
        return 1;
    case T_EXEC:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        } else {
            for(size_t i = 0; i < strlen(parametros[T_PATH].valor); i++){
                if(parametros[T_PATH].valor[i] == '.'){
                    return 1;
                }
            }
            std::cout<<"El PATH debe hacer referencia a un archivo"<<std::endl;
            return 0;
        }
        return 1;
    case T_REP:
        if (!parametros[T_NAME].valor[0]){
            std::cout<<"El comando necesita el parametro NAME"<<std::endl;
            return 0;
        }
        if (!parametros[T_ID].valor[0]){
            std::cout<<"El comando necesita el parametro ID"<<std::endl;
            return 0;
        }
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        } else {
            for(size_t i = 0; i < strlen(parametros[T_PATH].valor); i++){
                if(parametros[T_PATH].valor[i] == '.'){
                    return 1;
                }
            }
            std::cout<<"El PATH debe hacer referencia a un archivo"<<std::endl;
            return 0;
        }
        return 1;
    case T_COMENTARIO:
        return 1;
    case T_MKFS:
        if (!parametros[T_ID].valor[0]){
            std::cout<<"El comando necesita el parametro ID"<<std::endl;
            return 0;
        }
        return 1;
    case T_LOGIN:
        if (!parametros[T_ID].valor[0]){
            std::cout<<"El comando necesita el parametro ID"<<std::endl;
            return 0;
        }
        if (!parametros[T_USR].valor[0]){
            std::cout<<"El comando necesita el parametro USR"<<std::endl;
            return 0;
        }
        if (!parametros[T_PWD].valor[0]){
            std::cout<<"El comando necesita el parametro PWD"<<std::endl;
            return 0;
        }
        return 1;
    case T_LOGOUT:
        return 1;
    case T_MKGRP:
        if (!parametros[T_NAME].valor[0]){
            std::cout<<"El comando necesita el parametro NAME"<<std::endl;
            return 0;
        }
        return 1;
    case T_RMGRP:
        if (!parametros[T_NAME].valor[0]){
            std::cout<<"El comando necesita el parametro NAME"<<std::endl;
            return 0;
        }
        return 1;
    case T_MKUSR:
        if (!parametros[T_USR].valor[0]){
            std::cout<<"El comando necesita el parametro USR"<<std::endl;
            return 0;
        }
        if (!parametros[T_PWD].valor[0]){
            std::cout<<"El comando necesita el parametro PWD"<<std::endl;
            return 0;
        }
        if (!parametros[T_GRP].valor[0]){
            std::cout<<"El comando necesita el parametro GRP"<<std::endl;
            return 0;
        }
        return 1;
    case T_RMUSR:
        if (!parametros[T_USR].valor[0]){
            std::cout<<"El comando necesita el parametro USR"<<std::endl;
            return 0;
        }
        return 1;
    case T_CHMOD:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        if (!parametros[T_UGO].valor[0]){
            std::cout<<"El comando necesita el parametro UGO"<<std::endl;
            return 0;
        }
        return 1;
    case T_MKFILE:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        return 1;
    case T_CAT:
        if (!parametros[T_FILE].valor[0]){
            std::cout<<"El comando necesita el parametro FILE"<<std::endl;
            return 0;
        }
        return 1;
    case T_REM:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        return 1;
    case T_EDIT:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        if (!parametros[T_CONT].valor[0]){
            std::cout<<"El comando necesita el parametro CONT"<<std::endl;
            return 0;
        }
        return 1;
    case T_REN:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        if (!parametros[T_NAME].valor[0]){
            std::cout<<"El comando necesita el parametro NAME"<<std::endl;
            return 0;
        }
        return 1;
    case T_MKDIR:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        return 1;
    case T_CP:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        if (!parametros[T_DEST].valor[0]){
            std::cout<<"El comando necesita el parametro DEST"<<std::endl;
            return 0;
        }
        return 1;
    case T_MV:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        if (!parametros[T_DEST].valor[0]){
            std::cout<<"El comando necesita el parametro DEST"<<std::endl;
            return 0;
        }
        return 1;
    case T_FIND:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        if (!parametros[T_NAME].valor[0]){
            std::cout<<"El comando necesita el parametro NAME"<<std::endl;
            return 0;
        }
        return 1;
    case T_CHOWN:
        if (!parametros[T_PATH].valor[0]){
            std::cout<<"El comando necesita el parametro PATH"<<std::endl;
            return 0;
        }
        if (!parametros[T_USR].valor[0]){
            std::cout<<"El comando necesita el parametro USR"<<std::endl;
            return 0;
        }
        return 1;
    case T_CHGRP:
        if (!parametros[T_USR].valor[0]){
            std::cout<<"El comando necesita el parametro USR"<<std::endl;
            return 0;
        }
        if (!parametros[T_GRP].valor[0]){
            std::cout<<"El comando necesita el parametro GRP"<<std::endl;
            return 0;
        }
        return 1;
    case T_PAUSE:
        return 1;
    case T_LOSS:
        if (!parametros[T_ID].valor[0]){
            std::cout<<"El comando necesita el parametro ID"<<std::endl;
            return 0;
        }
        return 1;
    case T_RECOVERY:
        if (!parametros[T_ID].valor[0]){
            std::cout<<"El comando necesita el parametro ID"<<std::endl;
            return 0;
        }
        return 1;
    }

}

int ejecutar(struct comando comando, ListaMontaje *listaMontaje){

    switch(comando.tipo){
    case T_MKDISK:
        if(ejecutarMkdisk(comando)){
            modificarRaidPath((*(comando.parametros))[T_PATH].valor);
            return ejecutarMkdisk(comando);
        } else {
            return 0;
        }
    case T_RMDISK:
        return ejecutarRmdisk(comando);
    case T_FDISK:
        if(ejecutarFdisk(comando, listaMontaje, 0)){
            modificarRaidPath((*(comando.parametros))[T_PATH].valor);
            return ejecutarFdisk(comando, listaMontaje, 1);
        } else {
            return 0;
        }
    case T_MOUNT:
        return ejecutarMount(comando, listaMontaje);
    case T_UNMOUNT:
        return ejecutarUnmount(comando, listaMontaje);
    case T_EXEC:
        return ejecutarExec(comando, listaMontaje);
    case T_REP:
        return ejecutarRep(comando, listaMontaje);
    case T_COMENTARIO:
        return 1;
    case T_MKFS:
        break;
    case T_LOGIN:
        break;
    case T_LOGOUT:
        break;
    case T_MKGRP:
        break;
    case T_RMGRP:
        break;
    case T_MKUSR:
        break;
    case T_RMUSR:
        break;
    case T_CHMOD:
        break;
    case T_MKFILE:
        break;
    case T_CAT:
        break;
    case T_REM:
        break;
    case T_EDIT:
        break;
    case T_REN:
        break;
    case T_MKDIR:
        break;
    case T_CP:
        break;
    case T_MV:
        break;
    case T_FIND:
        break;
    case T_CHOWN:
        break;
    case T_CHGRP:
        break;
    case T_PAUSE:
        break;
    case T_LOSS:
        break;
    case T_RECOVERY:
        break;
    }
}

void modificarRaidPath(char *pathDisco){
    char *aux = pathDisco;
    while(*aux != '.'){
        aux++;
    }
    strcpy(aux,"_raid.disk");
}
