#include "mount.h"

int ejecutarMount(struct comando comando, ListaMontaje *listaMontaje){
    std::map<TipoParametro, struct parametro> parametros (*comando.parametros);

    //1. Verificar que el archivo existe
    struct stat st = {0};
    if (stat(parametros[T_PATH].valor, &st) == -1){
        std::cout<<"El archivo '"<<parametros[T_PATH].valor<<"' no existe"<<std::endl;
        return 0;
    }

    //2. Leer el archivo y obtener el MBR
    struct mbr MBR;
    FILE *discoVirtual = fopen(parametros[T_PATH].valor, "rb");
    fread(&MBR,sizeof(MBR),1,discoVirtual);
    fclose(discoVirtual);

    //3. Buscar la particion solicitada entre las particiones primarias
    for(struct partition particion : MBR.mbr_partition){
        if(particion.part_status == 1 && ( particion.part_type == 'P' || particion.part_type == 'p') && strcmp(particion.part_name, parametros[T_NAME].valor) == 0){

            //3.1. Montar particion
             return listaMontaje->montarParticion(parametros[T_PATH].valor, particion);

        } else if(particion.part_status == 1 && ( particion.part_type == 'E' || particion.part_type == 'e') && strcmp(particion.part_name, parametros[T_NAME].valor) == 0){

            std::cout<<"Las particiones Extendidas no se pueden montar"<<std::endl;
            return 0;
        }
    }

    //4. Buscar la particion solicitada entre las particiones logicas

    for(struct partition particion : MBR.mbr_partition){
        if(particion.part_status == 1 && particion.part_type == 'E'){

            struct ebr aux;
            FILE *archivo = fopen(parametros[T_PATH].valor, "rb+");
            fseek(archivo,particion.part_start,SEEK_SET);
            fread(&aux,sizeof(ebr),1,archivo);
            fclose(archivo);

            if(aux.part_status == 1 && strcmp(aux.part_name, parametros[T_NAME].valor) == 0){

                struct partition nuevaParticion;
                strcpy(nuevaParticion.part_name, aux.part_name);
                nuevaParticion.part_size = aux.part_size;
                nuevaParticion.part_type = 'L';
                nuevaParticion.part_start = aux.part_start;
                nuevaParticion.part_status = aux.part_status;

                //4.1. Montar particion
                 return listaMontaje->montarParticion(parametros[T_PATH].valor, nuevaParticion);
            }

            while(aux.part_next != -1){

                archivo = fopen(parametros[T_PATH].valor, "rb+");
                fseek(archivo,aux.part_next,SEEK_SET);
                fread(&aux,sizeof(ebr),1,archivo);
                fclose(archivo);

                if(aux.part_status == 1 && strcmp(aux.part_name, parametros[T_NAME].valor) == 0){

                    struct partition nuevaParticion;
                    strcpy(nuevaParticion.part_name, aux.part_name);
                    nuevaParticion.part_size = aux.part_size;
                    nuevaParticion.part_type = 'L';
                    nuevaParticion.part_start = aux.part_start;
                    nuevaParticion.part_status = aux.part_status;

                    //4.1. Montar particion
                     return listaMontaje->montarParticion(parametros[T_PATH].valor, nuevaParticion);
                }

            }
        }
    }

    std::cout<<"No existe una particion bajo el nombre '"<<parametros[T_NAME].valor<<"'"<<std::endl;
    return 0;
}
