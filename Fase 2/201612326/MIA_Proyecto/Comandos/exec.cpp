#include "exec.h"

int ejecutarExec(struct comando com, ListaMontaje *listaMontaje, struct user *usuario){

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
        if(entrada[0] == '\0' || entrada[0] == '\t' || entrada[0] == '\n'|| entrada[0] == '\r' || entrada[0] == '\f'){
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
        if(ejecutar(comando, listaMontaje,usuario)){
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

    yyin = fopen("temp.txt","r");
    int parserResult= yyparse();
    fclose(yyin);
    return parserResult;
}

int validar(struct comando comando){

    if(comando.tipo == T_LOGOUT || comando.tipo == T_PAUSE){
        return 1;
    }
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
            std::cout<<"El comando necesita elparametros parametro GRP"<<std::endl;
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
    }

}

int ejecutar(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario){

    switch(comando.tipo){
    case T_COMENTARIO:
        return 1;
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
        return ejecutarExec(comando, listaMontaje,usuario);
    case T_REP:
        return ejecutarRep(comando, listaMontaje);
    case T_MKFS:
        return ejecutarMkfs(comando, listaMontaje);
    case T_LOGIN:
        return Login::ejecutarLogin(comando,listaMontaje,usuario);
    case T_LOGOUT:
        return Logout::ejecutarLogout(usuario);
    case T_MKGRP:
        return Mkgrp::ejecutarMkgrp(comando,listaMontaje,usuario);
    case T_RMGRP:
        return Rmgrp::ejecutarRmgrp(comando,listaMontaje,usuario);
    case T_MKUSR:
        return Mkusr::ejecutarMkusr(comando,listaMontaje,usuario);
    case T_RMUSR:
        return Rmusr::ejecutarRmusr(comando,listaMontaje,usuario);
    case T_CHMOD:
        return Chmod::ejecutarChmod(comando,listaMontaje,usuario);
    case T_MKFILE:
        return Mkfile::ejecutarMkfile(comando,listaMontaje,usuario);
    case T_CAT:
        return Cat::ejecutarCat(comando,listaMontaje,usuario);
    case T_REM:
        break;
    case T_EDIT:
        return Edit::ejecutarEdit(comando,listaMontaje,usuario);
    case T_REN:
        return Ren::ejecutarRen(comando,listaMontaje,usuario);
    case T_MKDIR:
        return Mkdir::ejecutarMkdir(comando,listaMontaje,usuario);
    case T_CP:
        return Cp::ejecutarCp(comando,listaMontaje,usuario);
    case T_MV:
        return Mv::ejecutarMv(comando,listaMontaje,usuario);
    case T_FIND:
        return Find::ejecutarFind(comando,listaMontaje,usuario);
    case T_CHOWN:
        return Chown::ejecutarChown(comando,listaMontaje,usuario);
    case T_CHGRP:
        return Chgrp::ejecutarChgrp(comando,listaMontaje,usuario);
    case T_PAUSE:
        getchar();
        return 1;
    }
}

void modificarRaidPath(char *pathDisco){
    char *aux = pathDisco;
    while(*aux != '.'){
        aux++;
    }
    strcpy(aux,"_raid.disk");
}
