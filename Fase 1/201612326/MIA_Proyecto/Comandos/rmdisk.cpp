#include "rmdisk.h"

int ejecutarRmdisk(struct comando comando){
    std::map<TipoParametro, struct parametro> parametros = *(comando.parametros);

    //1. Verificar si el archivo existe
    struct stat st = {0};
    if (stat(parametros[T_PATH].valor, &st) == -1){
        std::cout<<"El archivo '"<<parametros[T_PATH].valor<<"' no existe"<<std::endl;
        return 0;
    }

    //2. Verificar que si se desea eliminar
    RESPUESTARMDISK:;
    std::cout<<"Está seguro de eliminar el archivo?(Y/N): ";
    char respuesta[20];
    std::cin.getline(respuesta,20,'\n');

    switch(respuesta[0]){
    case 'Y':
    case 'y':
        //Continuar
        break;
    case 'N':
    case 'n':
        std::cout<<"El archivo no se ha eliminado"<<std::endl;
        return 0;
    default:
        std::cout<<"Respuesta inválida"<<std::endl;
        goto RESPUESTARMDISK;
    }

    //3. Eliminar el archivo
    if(remove(parametros[T_PATH].valor)){
        std::cout<<"Ha ocurrido un error al borrar el archivo"<<std::endl;
        return 0;
    }

    return 1;
}
