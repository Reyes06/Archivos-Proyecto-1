#include "fdisk.h"

int ejecutarFdisk(struct comando comando, ListaMontaje *listaMontaje, int isRAID){
    std::map<TipoParametro, struct parametro> parametros = *(comando.parametros);

    //1. Verificar si el archivo existe
    struct stat st = {0};
    if (stat(parametros[T_PATH].valor, &st) == -1){
        std::cout<<"El archivo '"<<parametros[T_PATH].valor<<"' no existe"<<std::endl;
        return 0;
    }

    //3. Verificar que no hayan los parametros SIZE, ADD y DELETEPARAM al mismo tiempo
    if(parametros[T_SIZE].valor[0] && parametros[T_ADD].valor[0]){
        std::cout<<"No se permite utilizar los parametros SIZE y ADD juntos en el comando FDISK"<<std::endl;
        return 0;
    } else if(parametros[T_SIZE].valor[0] && parametros[T_DELETE].valor[0]){
        std::cout<<"No se permite utilizar los parametros SIZE y DELETE juntos en el comando FDISK"<<std::endl;
        return 0;
    } else if(parametros[T_ADD].valor[0] && parametros[T_DELETE].valor[0]){
        std::cout<<"No se permite utilizar los parametros ADD y DELETE juntos en el comando FDISK"<<std::endl;
        return 0;
    } else if(parametros[T_SIZE].valor[0] && parametros[T_ADD].valor[0] && parametros[T_DELETE].valor[0]){
        std::cout<<"No se permite utilizar los parametros SIZE, ADD y DELETE juntos en el comando FDISK"<<std::endl;
        return 0;
    }

    //2. Determinar que se desea realizar
    struct parametro aux;
    aux = parametros[T_SIZE];
    if(aux.valor[0]){
        return crearParticion(parametros);
    }
    aux = parametros[T_ADD];
    if(aux.valor[0]){
        return modificarParticion(parametros);
    }
    aux = parametros[T_DELETE];
    if(aux.valor[0]){
        return borrarParticion(parametros, listaMontaje, isRAID);
    }
    std::cout<<"No se ha identicicado el procedimiento que se desea realizar (SIZE,DELETE,ADD)"<<std::endl;
    return 0;
}

int crearParticion(std::map<TipoParametro, struct parametro> parametros){

    struct mbr MBR;

    //1. Abre el archivo y recuperar el MBR
    FILE *discoVirtual = fopen(parametros[T_PATH].valor,"rb+");
    fread(&MBR,sizeof(mbr),1,discoVirtual);
    fclose(discoVirtual);

    //2. Recorre las particiones para verificar que el nombre de la particion no exista
    for(struct partition particion : MBR.mbr_partition){
        //Verificar en particiones primarias y extendidas
        if(particion.part_status == 1 && strcmp(particion.part_name, parametros[T_NAME].valor) == 0){
            std::cout<<"Ya existe una particion bajo el nombre '"<<parametros[T_NAME].valor<<"'"<<std::endl;
            return 0;
        }
        //Verificar en particiones logicas
        if(particion.part_status == 1 && (particion.part_type == 'e' || particion.part_type == 'E')){

            struct ebr particionLogica;
            discoVirtual = fopen(parametros[T_PATH].valor,"rb+");
            fseek(discoVirtual,particion.part_start, SEEK_SET);
            fread(&particionLogica,sizeof(ebr),1,discoVirtual);
            fclose(discoVirtual);

            while(true){

                if(particionLogica.part_status == 0){
                    //Verificar si es la ultima particion en lista
                    if(particionLogica.part_next == -1){
                        break;
                    }

                    //Continuar con la siguiente particion
                    discoVirtual = fopen(parametros[T_PATH].valor,"rb+");
                    fseek(discoVirtual,particionLogica.part_next, SEEK_SET);
                    fread(&particionLogica,sizeof(ebr),1,discoVirtual);
                    fclose(discoVirtual);
                    continue;
                }


                if(strcmp(particionLogica       .part_name, parametros[T_NAME].valor) == 0){
                    std::cout<<"Ya existe una particion bajo el nombre '"<<parametros[T_NAME].valor<<"'"<<std::endl;
                    return 0;
                }

                //Continuar a la siguiente particion
                if(particionLogica.part_next == -1){
                    break;
                } else {
                    discoVirtual = fopen(parametros[T_PATH].valor,"rb+");
                    fseek(discoVirtual,particionLogica.part_next, SEEK_SET);
                    fread(&particionLogica,sizeof(ebr),1,discoVirtual);
                    fclose(discoVirtual);
                }

            }
        }
    }

    //3. Crear la particion de acuerdo a su tipo
    if(parametros[T_TYPE].valor[0]){
        switch(parametros[T_TYPE].valor[0]){
        case 'P': //Particion primaria
        case 'p':
            if (!crearParticionPrimaria(parametros, &MBR)){
                std::cout<<"No se ha creado la particion primaria"<<std::endl;
                return 0;
            }
            break;
        case 'L': //Particion logica
        case 'l':
            if(!crearParticionLogica(parametros, &MBR)){
                std::cout<<"No se ha creado la particion logica"<<std::endl;
                return 0;
            }
            break;
        case 'E': //Particion extendida
        case 'e':
            if(!crearParticionExtendida(parametros, &MBR)){
                std::cout<<"No se ha creado la particion extendida"<<std::endl;
                return 0;
            }
            break;
        default:
            std::cout<<"No se ha especificado que tipo de particion se desea crear (TYPE)"<<std::endl;
            return 0;
        }
    } else {
        if(!crearParticionPrimaria(parametros, &MBR)){
            std::cout<<"No se ha creado la particion primaria"<<std::endl;
            return 0;
        }
    }

    //4. Abre el archivo y guarda el MBR
    discoVirtual = fopen(parametros[T_PATH].valor,"rb+");
    fwrite(&MBR,sizeof(mbr),1,discoVirtual);
    fclose(discoVirtual);
    return 1;
}

int crearParticionPrimaria(std::map<TipoParametro, struct parametro> parametros, struct mbr *MBR){
    //1. Verificar si actualmente hay menos de 4 particiones activas
    bool espacioLibre = false;
    for(struct partition particion: MBR->mbr_partition){
        if(particion.part_status == 0){
            espacioLibre = true;
        }
    }
    if(!espacioLibre){
        std::cout<<"El disco no cuenta con particiones libres"<<std::endl;
        return 0;
    }

    //2. Crear la nueva particion
    struct partition nuevaParticion;
    nuevaParticion.part_status = 1;
    nuevaParticion.part_type = 'P';
    if(parametros[T_FIT].valor[0]){
        nuevaParticion.part_fit = parametros[T_FIT].valor[0];
    } else {
        nuevaParticion.part_fit = 'W';
    }
    if(parametros[T_UNIT].valor[0]){
        switch(parametros[T_UNIT].valor[0]){
        case 'k':
        case 'K':
            nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024;
            break;
        case 'm':
        case 'M':
            nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024*1024;
            break;
        default:
            std::cout<<"No se reconocen las unidades indicadas(UNIT)"<<std::endl;
            return 0;
        }
    } else {
        nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024; //Por defecto es KiloBytes
    }
    for(int i = 0; i < 16; i++){
        nuevaParticion.part_name[i] = parametros[T_NAME].valor[i];
    }
    //El atributo 'part_start' de la nueva particion no sera seteado en este momento, no se donde sera colocado


    //3. Crear un modelo con la lista de disponibles e insertar la particion
    ListaDisponibles* listaDisponibles = new ListaDisponibles(MBR->mbr_tamano);
    switch(MBR->disk_fit){
    case 'F': //Primer ajuste
    case 'f':
        return listaDisponibles->insertarParticion(nuevaParticion, MBR, listaDisponibles->FIRSTFIT);
    case 'W': //Peor ajuste
    case 'w':
        return listaDisponibles->insertarParticion(nuevaParticion, MBR, listaDisponibles->WORSTFIT);
    case 'B': //Mejor ajuste
    case 'b':
        return listaDisponibles->insertarParticion(nuevaParticion, MBR, listaDisponibles->BESTFIT);
    default:
        std::cout<<"El tipo de posicionamiento solicitado no se ha identificado (FIT)"<<std::endl;
        return 0;
    }
}

int crearParticionExtendida(std::map<TipoParametro, struct parametro> parametros, struct mbr *MBR){
    //1. Verificar si ya existe una particion extendida
    for(struct partition particion: MBR->mbr_partition){
        if(particion.part_status == 1 && (particion.part_type == 'E' || particion.part_type == 'e')){
            std::cout<<"Ya existe una particin extendida en el disco"<<std::endl;
            return 0;
        }
    }

    //2. Verificar si actualmente hay menos de 4 particiones activas
    bool espacioLibre = false;
    for(struct partition particion: MBR->mbr_partition){
        if(particion.part_status == 0){
            espacioLibre = true;
        }
    }
    if(!espacioLibre){
        std::cout<<"El disco no cuenta con particiones libres"<<std::endl;
        return 0;
    }

    //3. Crear la nueva particion
    struct partition nuevaParticion;
    nuevaParticion.part_status = 1;
    nuevaParticion.part_type = 'E';
    if(parametros[T_FIT].valor[0]){
        nuevaParticion.part_fit = parametros[T_FIT].valor[0];
    } else {
        nuevaParticion.part_fit = 'W';
    }
    if(parametros[T_UNIT].valor[0]){
        switch(parametros[T_UNIT].valor[0]){
        case 'k':
        case 'K':
            nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024;
            break;
        case 'm':
        case 'M':
            nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024*1024;
            break;
        default:
            std::cout<<"No se reconocen las unidades indicadas(UNIT)"<<std::endl;
            return 0;
        }
    } else {
        nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024; //Por defecto es KiloBytes
    }
    for(int i = 0; i < 16; i++){
        nuevaParticion.part_name[i] = parametros[T_NAME].valor[i];
    }
    //El atributo 'part_start' de la nueva particion no sera seteado en este momento, no se donde sera colocado


    //4. Crear un modelo con la lista de disponibles e insertar la particion
    ListaDisponibles *listaDisponibles = new ListaDisponibles(MBR->mbr_tamano);
    switch(MBR->disk_fit){
    case 'F': //Primer ajuste
    case 'f':
        if(!listaDisponibles->insertarParticion(nuevaParticion, MBR, listaDisponibles->FIRSTFIT)){
            return 0;
        }
        break;
    case 'W': //Peor ajuste
    case 'w':
        if(!listaDisponibles->insertarParticion(nuevaParticion, MBR,listaDisponibles-> WORSTFIT)){
            return 0;
        }
        break;
    case 'B': //Mejor ajuste
    case 'b':
        if(!listaDisponibles->insertarParticion(nuevaParticion, MBR, listaDisponibles->BESTFIT)){
            return 0;
        }
        break;
    default:
        std::cout<<"El tipo de posicionamiento solicitado no se ha identificado (FIT)"<<std::endl;
        return 0;
    }

    //5. Buscar la particion extendida creada
    struct partition extendPartition;
    for(struct partition particion : MBR->mbr_partition){
        if(particion.part_status == 1 && (particion.part_type == 'E' || particion.part_type == 'e')){
            extendPartition = particion;
            break;
        }
    }

    //6. Crear el EBR inicial
    struct ebr nuevoEBR;
    nuevoEBR.part_status = 0;
    nuevoEBR.part_next = -1;
    nuevoEBR.part_start = extendPartition.part_start;
    nuevoEBR.part_name[0] = '\0';
    nuevoEBR.part_size = 0;

    //7. Escribir el EBR inicial en el archivo
    FILE *archivo = fopen(parametros[T_PATH].valor, "rb+");
    fseek(archivo,extendPartition.part_start,SEEK_SET);
    fwrite(&nuevoEBR,sizeof(ebr),1,archivo);
    fclose(archivo);

    return 1;
}

int crearParticionLogica(std::map<TipoParametro, struct parametro> parametros, struct mbr *MBR){
    //1. Buscar y verificar si ya existe una particion extendida
    bool existe = 0;
    struct partition extendPartition;
    for(struct partition particion: MBR->mbr_partition){
        if(particion.part_status == 1 && (particion.part_type == 'E' || particion.part_type == 'e')){
            existe = 1;
            extendPartition = particion;
        }
    }
    if(!existe){
        std::cout<<"No existe una particion extendida en el disco"<<std::endl;
        return 0;
    }

    //2. Abrir el archivo y recuperar el EBR inicial
    struct ebr particionInicial;
    FILE *archivo = fopen(parametros[T_PATH].valor, "rb+");
    fseek(archivo,extendPartition.part_start,SEEK_SET);
    fread(&particionInicial,sizeof(ebr),1,archivo);
    fclose(archivo);

    //3. Crear la nueva particion
    struct ebr nuevaParticion;
    nuevaParticion.part_status = 1;
    if(parametros[T_FIT].valor[0]){
        nuevaParticion.part_fit = parametros[T_FIT].valor[0];
    } else {
        nuevaParticion.part_fit = 'W';
    }
    if(parametros[T_UNIT].valor[0]){
        switch(parametros[T_UNIT].valor[0]){
        case 'k':
        case 'K':
            nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024;
            break;
        case 'm':
        case 'M':
            nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024*1024;
            break;
        default:
            std::cout<<"No se reconocen las unidades indicadas(UNIT)"<<std::endl;
            return 0;
        }
    } else {
        nuevaParticion.part_size = atoi(parametros[T_SIZE].valor)*1024; //Por defecto es KiloBytes
    }
    for(int i = 0; i < 16; i++){
        nuevaParticion.part_name[i] = parametros[T_NAME].valor[i];
    }
    //El atributo 'part_start' de la nueva particion no sera seteado en este momento, no se donde sera colocado

    //3. Crear un modelo con la lista de disponibles e insertar la particion
    ListaLogicoDisponibles *listaDisponibles = new ListaLogicoDisponibles(extendPartition.part_size, extendPartition.part_start);
    switch(extendPartition.part_fit){
    case 'F': //Primer ajuste
    case 'f':
        return listaDisponibles->insertarParticion(nuevaParticion, particionInicial, listaDisponibles->FIRSTFIT, parametros[T_PATH].valor);
    case 'W': //Peor ajuste
    case 'w':
        return listaDisponibles->insertarParticion(nuevaParticion, particionInicial, listaDisponibles->WORSTFIT, parametros[T_PATH].valor);
    case 'B': //Mejor ajuste
    case 'b':
        return listaDisponibles->insertarParticion(nuevaParticion, particionInicial, listaDisponibles->BESTFIT, parametros[T_PATH].valor);
    default:
        std::cout<<"El tipo de posicionamiento solicitado no se ha identificado (FIT)"<<std::endl;
        return 0;
    }

    return 0;
}

int modificarParticion(std::map<TipoParametro, struct parametro> parametros){
    //0. Obtener el valor en bytes de las unidades a incrementar/decrementar
    int unidadesIncreDecrementar;
    if(parametros[T_ADD].valor[0] != '-'){
        if(parametros[T_UNIT].valor[0]){
            switch(*parametros[T_UNIT].valor){
            case 'k':
            case 'K':
                unidadesIncreDecrementar = atoi(parametros[T_ADD].valor)*1024;
                break;
            case 'm':
            case 'M':
                unidadesIncreDecrementar = atoi(parametros[T_ADD].valor)*1024*1024;
                break;
            default:
                std::cout<<"No se reconocen las unidades indicadas(UNIT)"<<std::endl;
                return 0;
            }
        } else {
            unidadesIncreDecrementar = atoi(parametros[T_SIZE].valor)*1024; //Por defecto es Kilobytes
        }
    } else {
        char *add = parametros[T_ADD].valor;
        add++;
        if(parametros[T_UNIT].valor[0]){
            switch(*parametros[T_UNIT].valor){
            case 'k':
            case 'K':
                unidadesIncreDecrementar = atoi(add)*1024*(-1);
                break;
            case 'm':
            case 'M':
                unidadesIncreDecrementar = atoi(add)*1024*1024*(-1);
                break;
            default:
                std::cout<<"No se reconocen las unidades indicadas(UNIT)"<<std::endl;
                return 0;
            }
        } else {
            unidadesIncreDecrementar = atoi(add)*1024*(-1); //Por defecto es Kilobytes
        }
    }

    //1. Recuperar el MBR
    struct mbr *MBR = new struct mbr;
    FILE *archivo = fopen(parametros[T_PATH].valor,"rb+");
    fread(MBR,sizeof(mbr),1,archivo);
    fclose(archivo);

    //2. Buscar la particion
    for(int i = 0; i < 4; i++){
        struct partition *part = &MBR->mbr_partition[i];
        //2.1 Buscar entre las particiones primarias y extendidas
        if(part->part_status == 1 && strcmp(part->part_name, parametros[T_NAME].valor) == 0){

            //Modificar la particion
            return modificarPriExt(part,unidadesIncreDecrementar,i,MBR, parametros[T_PATH].valor);

        }

        //2.2 Buscar entre las particiones logicas
        if(part->part_status == 1 && (part->part_type == 'e' || part->part_type == 'E')){

            //2.2.1 Obtener el EBR inicial
            struct ebr *part_logica = new struct ebr;
            archivo = fopen(parametros[T_PATH].valor, "rb+");
            fseek(archivo,part->part_start,SEEK_SET);
            fread(part_logica,sizeof(ebr),1,archivo);
            fclose(archivo);

            //2.2.2 Buscar en la lista de particiones logicas
            while(true){
                //En caso de ser el EBR inicial
                if(part_logica->part_status == 0){

                    if(part_logica->part_next == -1){
                        break;
                    }
                    archivo = fopen(parametros[T_PATH].valor, "rb+");
                    fseek(archivo,part_logica->part_next,SEEK_SET);
                    fread(part_logica,sizeof(ebr),1,archivo);
                    fclose(archivo);
                    continue;
                }

                if(part_logica->part_status == 1 && strcmp(part_logica->part_name, parametros[T_NAME].valor) == 0){

                    //Modificar particion
                    return modificarLog(part_logica, unidadesIncreDecrementar, parametros[T_PATH].valor, part->part_size);

                }

                if(part_logica->part_next == -1){
                    break;
                }
                archivo = fopen(parametros[T_PATH].valor, "rb+");
                fseek(archivo,part_logica->part_next,SEEK_SET);
                fread(part_logica,sizeof(ebr),1,archivo);
                fclose(archivo);
                continue;
            }
        }
    }

    std::cout<<"No se ha encontrado la particion '"<<parametros[T_NAME].valor<<"'"<<std::endl;
    return 0;
}

int modificarPriExt(struct partition *particion, int unidadesIncreDecrementar, int i, struct mbr *MBR, char *path){

    if(unidadesIncreDecrementar > 0){
        //Incrementar

        //1. Verificar que exista espacio suficiente entre la particion actual y la siguiente
        if(i < 4 && MBR->mbr_partition[i+1].part_status == 1){
            if(particion->part_start + particion->part_size + unidadesIncreDecrementar > MBR->mbr_partition[i+1].part_start){
                std::cout<<"No existe espacio suficiente para aumentar el tamano de la particion"<<std::endl;
                return 0;
            }
        } else {
            if(particion->part_start + particion->part_size + unidadesIncreDecrementar > MBR->mbr_tamano){
                std::cout<<"No existe espacio suficiente para aumentar el tamano de la particion"<<std::endl;
                return 0;
            }
        }


        //2. Incrementar la particion
        particion->part_size += unidadesIncreDecrementar;
    } else {
        //Decrementar
        unidadesIncreDecrementar *= -1;

        //1. Verificar si la particion tiene suficiente espacio para decrementar
        if(particion->part_size - unidadesIncreDecrementar < 1){
            std::cout<<"No existe espacio suficiente para reducir la particion"<<std::endl;
            return 0;
        }

        //2. Decrementar la particion
        particion->part_size -= unidadesIncreDecrementar;
    }

    FILE *archivo = fopen(path, "rb+");
    fwrite(MBR,sizeof(mbr),1,archivo);
    fclose(archivo);

    return 1;
}

int modificarLog(struct ebr *particion, int unidadesIncreDecrementar, char *path, int size_extend_partition){

    if(unidadesIncreDecrementar > 0){
        //Incrementar

        //1. Verificar que exista espacio suficiente entre la particion actual y la siguiente
        if(particion->part_next != -1){
            if(particion->part_start + particion->part_size + unidadesIncreDecrementar > particion->part_next){
                std::cout<<"No existe espacio suficiente para aumentar el tamano de la particion"<<std::endl;
                return 0;
            }
        } else {
            if(particion->part_start + particion->part_size + unidadesIncreDecrementar > size_extend_partition){
                std::cout<<"No existe espacio suficiente para aumentar el tamano de la particion"<<std::endl;
                return 0;
            }
        }


        //2. Incrementar la particion
        particion->part_size += unidadesIncreDecrementar;
    } else {
        //Decrementar
        unidadesIncreDecrementar *= -1;

        //1. Verificar si la particion tiene suficiente espacio para decrementar
        if(particion->part_size - unidadesIncreDecrementar < 1){
            std::cout<<"No existe espacio suficiente para reducir la particion"<<std::endl;
            return 0;
        }

        //2. Decrementar la particion
        particion->part_size -= unidadesIncreDecrementar;
    }

    FILE *archivo = fopen(path, "rb+");
    fseek(archivo,particion->part_start,SEEK_SET);
    fwrite(particion,sizeof(ebr),1,archivo);
    fclose(archivo);

    return 1;
}

int borrarParticion(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje, int isRAID){

    for(unsigned long i = 0; i < strlen(parametros[T_DELETE].valor); i++)
        parametros[T_DELETE].valor[i] = tolower(parametros[T_DELETE].valor[i]);
    if(strcmp(parametros[T_DELETE].valor,"fast") == 0 || strcmp(parametros[T_DELETE].valor,"full") == 0){
        //Solo es para verificar que sea FAST o FULL
    } else {
        std::cout<<"No se ha identificado el tipo de borrado a implementar (FAST/FULL)"<<std::endl;
        return 0;
    }

    //0. Verificar que la particion no este montada
    if(listaMontaje->estaMontado(parametros[T_PATH].valor,parametros[T_NAME].valor)){
        std::cout<<"La particion '"<<parametros[T_NAME].valor<<"' se encuentra montada"<<std::endl;
        return 0;
    }

    //1. Recuperar el MBR
    struct mbr *MBR = new struct mbr;
    FILE *archivo = fopen(parametros[T_PATH].valor,"rb+");
    fread(MBR,sizeof(mbr),1,archivo);
    fclose(archivo);

    //2. Buscar la particion
    for(int i = 0; i < 4; i++){
        struct partition *part = &MBR->mbr_partition[i];
        //2.1 Buscar entre las particiones primarias y extendidas
        if(part->part_status == 1 && strcmp(part->part_name, parametros[T_NAME].valor) == 0){

            //Eliminar la particion
            if(!isRAID){
                RESPUESTAFDISK1:;
                std::cout<<"Desea eliminar la particion '"<<parametros[T_NAME].valor<<"'? (Y/N): ";
                char respuesta[20];
                std::cin.getline(respuesta,20,'\n');

                switch(respuesta[0]){
                case 'N':
                case 'n':
                    std::cout<<"No se ha eliminado la particion"<<std::endl;
                    return 0;
                case 'Y':
                case 'y':
                    //Continuar con la ejecución
                    break;
                default:
                    std::cout<<"Respuesta invalida"<<std::endl;
                    goto RESPUESTAFDISK1;
                }
            }


            part->part_status = 0;

            archivo = fopen(parametros[T_PATH].valor,"rb+");
            fwrite(MBR,sizeof(mbr),1,archivo);
            fclose(archivo);

            return 1;


        }

        //2.2 Buscar entre las particiones logicas
        if(part->part_status == 1 && (part->part_type == 'e' || part->part_type == 'E')){

            //2.2.1 Obtener el EBR inicial
            struct ebr *part_logica_anterior = new struct ebr;
            part_logica_anterior->part_start = 0;
            struct ebr *part_logica = new struct ebr;

            archivo = fopen(parametros[T_PATH].valor, "rb+");
            fseek(archivo,part->part_start,SEEK_SET);
            fread(part_logica,sizeof(ebr),1,archivo);
            fclose(archivo);

            //2.2.2 Buscar en la lista de particiones logicas
            while(true){

                if(part_logica->part_status == 0){ //En caso de ser el EBR inicial


                    if(part_logica->part_next == -1){
                        break;
                    }
                    *part_logica_anterior = *part_logica;
                    archivo = fopen(parametros[T_PATH].valor, "rb+");
                    fseek(archivo,part_logica->part_next,SEEK_SET);
                    fread(part_logica,sizeof(ebr),1,archivo);
                    fclose(archivo);
                    continue;
                }

                if(part_logica->part_status == 1 && strcmp(part_logica->part_name, parametros[T_NAME].valor) == 0){

                    //Eliminar la particion logica
                    if(!isRAID){
                        RESPUESTAFDISK5:;
                        std::cout<<"Desea eliminar la particion '"<<parametros[T_NAME].valor<<"'? (Y/N): ";
                        char respuesta[20];
                        std::cin.getline(respuesta,20,'\n');

                        switch(respuesta[0]){
                        case 'N':
                        case 'n':
                            std::cout<<"No se ha eliminado la particion"<<std::endl;
                            return 0;
                        case 'Y':
                        case 'y':
                            //Continuar con la ejecución
                            break;
                        default:
                            std::cout<<"Respuesta invalida"<<std::endl;
                            goto RESPUESTAFDISK5;
                        }
                    }


                    if(part_logica_anterior->part_start == 0){
                        //Se desea eliminar la primera particion de la lista de particiones logicas
                        part_logica->part_status = 0;

                        archivo = fopen(parametros[T_PATH].valor,"rb+");
                        fseek(archivo, part_logica->part_start,SEEK_SET);
                        fwrite(part_logica,sizeof(ebr),1,archivo);
                        fclose(archivo);
                    } else {
                        //Se desea eliminar una particion logica intermedia o final
                        part_logica_anterior->part_next = part_logica->part_next;

                        archivo = fopen(parametros[T_PATH].valor,"rb+");
                        fseek(archivo, part_logica_anterior->part_start,SEEK_SET);
                        fwrite(part_logica_anterior,sizeof(ebr),1,archivo);
                        fclose(archivo);
                    }
                    return 1;
                }

                if(part_logica->part_next == -1){
                    break;
                }
                *part_logica_anterior = *part_logica;
                archivo = fopen(parametros[T_PATH].valor, "rb+");
                fseek(archivo,part_logica->part_next,SEEK_SET);
                fread(part_logica,sizeof(ebr),1,archivo);
                fclose(archivo);
                continue;
            }
        }
    }

    std::cout<<"No se ha encontrado una particion bajo el nombre '"<<parametros[T_NAME].valor<<"'"<<std::endl;
    return 0;
}
