#include "mkdisk.h"

int ejecutarMkdisk(struct comando comando){
    std::map<TipoParametro, struct parametro> parametros = *(comando.parametros);

    //1. Crear la ruta y el archivo
    crearDirectorio(parametros[T_PATH].valor);

    //2. Crear el MBR
    struct mbr *nuevoMBR = getDefaultMBR(parametros);
    if(nuevoMBR == nullptr){
        return 0;
    }

    //3. Verificar si el archivo existe
    struct stat st = {0};
    if (stat(parametros[T_PATH].valor, &st) != -1){
        std::cout<<"El archivo '"<<parametros[T_PATH].valor<<"' ya existe"<<std::endl;
        RESPUESTAMBR:;
        std::cout<<"Desea reemplazarlo? (Y/N): ";
        char respuesta[20];
        std::cin.getline(respuesta,20,'\n');

        switch(respuesta[0]){
        case 'N':
        case 'n':
            std::cout<<"Archivo no creado"<<std::endl;
            return 0;
        case 'Y':
        case 'y':
            //Continuar con la ejecución
            break;
        default:
            std::cout<<"Respuesta invalida"<<std::endl;
            //getchar();
            goto RESPUESTAMBR;
        }
    }

    //4. Crear el archivo
    FILE *archivo = fopen(parametros[T_PATH].valor,"wb");
    fwrite(nuevoMBR,sizeof(mbr), 1 , archivo);
    fseek(archivo, nuevoMBR->mbr_tamano - 184 - 1,SEEK_CUR);

    char ultimoCaracter = '\0';
    fwrite(&ultimoCaracter,1,sizeof(char),archivo);
    fclose(archivo);

    return 1;
}

struct mbr *getDefaultMBR(std::map<TipoParametro, struct parametro> parametros){
    struct mbr *nuevo = new struct mbr;
    if(parametros[T_UNIT].valor[0]){
        switch(*parametros[T_UNIT].valor){
        case 'k':
        case 'K':
            nuevo->mbr_tamano = atoi(parametros[T_SIZE].valor)*1024;
            break;
        case 'm':
        case 'M':
            nuevo->mbr_tamano = atoi(parametros[T_SIZE].valor)*1024*1024;
            break;
        default:
            std::cout<<"No se reconocen las unidades indicadas(UNIT)"<<std::endl;
            return nullptr;
        }
    } else {
        nuevo->mbr_tamano = atoi(parametros[T_SIZE].valor)*1024*1024; //Por defecto es MegaBytes
    }

    time_t hora = time(0);
    nuevo->mbr_fecha_creacion = *localtime(&hora);

    nuevo->mbr_disk_signature = rand() % 1000;

    if(parametros[T_FIT].valor[0]){
        nuevo->disk_fit = parametros[T_FIT].valor[0];
    } else {
        nuevo->disk_fit = 'F'; //Por defecto el primer ajuste
    }

    for(int i=0; i<4; i++){
        nuevo->mbr_partition[i].part_status = 0; //Marcar las particiones como no activas
    }

    return nuevo;
}

int crearDirectorio(char *ruta){
    char carpeta[256]; carpeta[0] = '\0';

    while(*ruta != '\0'){

        if(*ruta !='/'){
            strncat(carpeta,ruta,1);
        } else {
            strcat(carpeta,"/");
            mkdir(carpeta,0777);
        }
        ruta++;
    }
    return 1;
}
