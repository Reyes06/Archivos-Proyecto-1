#include "ListaMontaje.h"

ListaMontaje::ListaMontaje()
{
    this->cabeceraDiscos = nullptr;
}

int ListaMontaje::montarParticion(char* disk_location, struct partition part){
    //1. Verificar si es la primera particion que se monta
    if(cabeceraDiscos == nullptr){
        struct nodoDisco *nuevoDisco = new struct nodoDisco;
        strcpy(nuevoDisco->disk_location, disk_location);
        nuevoDisco->letraDisco = 'a';
        nuevoDisco->siguiente = nullptr;

        struct nodoParticion *nuevaParticion = new struct nodoParticion;
        nuevaParticion->particion = part;
        nuevaParticion->numeroMontaje = 1;
        nuevaParticion->siguiente = nullptr;

        nuevoDisco->cabeceraParticiones = nuevaParticion;
        this->cabeceraDiscos = nuevoDisco;

        std::cout<<"La particion se ha montado bajo el nombre 'vda1'"<<std::endl;
        return 1;
    }

    //2. Verificar si la particion pertenece a un disco ya montado
    struct nodoDisco *disk = cabeceraDiscos;
    while(disk != nullptr){
        if(strcmp(disk->disk_location, disk_location) == 0){

            //1.1 Verificar que la particion que se desea montar, no haya sido montada previamente
            struct nodoParticion *partition = disk->cabeceraParticiones;
            while(partition != nullptr){
                if(strcmp(partition->particion.part_name, part.part_name) == 0){
                    std::cout<<"La particion ya se encuentra montada bajo el ID 'vd"<<disk->letraDisco<<partition->numeroMontaje<<"'"<<std::endl;
                    return 0;
                }
                partition = partition->siguiente;
            }

            //1.2 Crear el nodo de la particion
            struct nodoParticion *nuevo = new struct nodoParticion;
            nuevo->particion = part;
            nuevo->siguiente = nullptr;

            partition = disk->cabeceraParticiones;
            while(partition->siguiente != nullptr){
                partition = partition->siguiente;
            }

            nuevo->numeroMontaje = partition->numeroMontaje+1;
            partition->siguiente = nuevo;

            std::cout<<"La particion se ha montado bajo el ID 'vd"<<disk->letraDisco<<nuevo->numeroMontaje<<"'"<<std::endl;
            return 1;
        }
        disk = disk->siguiente;
    }

    //El disco no ha sido montado

    //3. Crear el disco y su particion asociada
    struct nodoDisco *nuevoDisco = new struct nodoDisco;
    strcpy(nuevoDisco->disk_location, disk_location);
    /*nuevoDisco->letraDisco = 'a';     Todavia no se sabe que letra tendra el disco*/
    nuevoDisco->siguiente = nullptr;

    struct nodoParticion *nuevaParticion = new struct nodoParticion;
    nuevaParticion->particion = part;
    nuevaParticion->numeroMontaje = 1;
    nuevaParticion->siguiente = nullptr;

    nuevoDisco->cabeceraParticiones = nuevaParticion;

    struct nodoDisco *aux = cabeceraDiscos;
    while(aux->siguiente != nullptr){
        aux = aux->siguiente;
    }

    //4. Agregar el nuevo disco al final de la lista
    nuevoDisco->letraDisco = aux->letraDisco+1;
    aux->siguiente = nuevoDisco;

    std::cout<<"La particion se ha montado bajo el ID 'vd"<<nuevoDisco->letraDisco<<"1'"<<std::endl;
    return 1;
}

int ListaMontaje::desmontarParticion(char* id){

    //1. Obtener los valores que identifican a la particion
    char disk_letter = id[2];
    int partition_number = atoi(&(id[3]));

    //2. Recorrer los discos para buscar la particion solicitada
    struct nodoDisco *discoAnterior = nullptr;
    struct nodoDisco *discoActual = cabeceraDiscos;
    while(discoActual != nullptr){

        if(discoActual->letraDisco == disk_letter){

            //3. Recorrer las particiones del disco para buscar la particion solicitada
            struct nodoParticion *particionAnterior = nullptr;
            struct nodoParticion *particionActual= discoActual->cabeceraParticiones;
            while(particionActual != nullptr){

                if(particionActual->numeroMontaje == partition_number){
                    //4. Eliminar la particion encontrada

                    if(particionAnterior == nullptr){
                        //4.1 La particion es la primera en la lista
                        discoActual->cabeceraParticiones = discoActual->cabeceraParticiones->siguiente;
                    } else {
                        //4.2 La particion esta en el centro o el fin de la lista
                        particionAnterior->siguiente = particionActual->siguiente;
                    }

                    //5. Verificar si el disco se quedo sin particiones montadas
                    if(discoActual->cabeceraParticiones == nullptr){
                        //6 Eliminar el disco de la lista de discos montados

                        if(discoAnterior == nullptr){
                            //6.1 El disco es el primero en la lista
                            this->cabeceraDiscos = this->cabeceraDiscos->siguiente;
                        } else {
                            //6.2 El disco esta en el centro o el fin de la lista
                            discoAnterior->siguiente = discoActual->siguiente;
                        }


                    }
                    return 1;
                }

                particionAnterior = particionActual;
                particionActual = particionActual->siguiente;
            }

        }

        discoAnterior = discoActual;
        discoActual = discoActual->siguiente;
    }

    std::cout<<"No se ha encontrado una particion con el ID '"<<id<<"'"<<std::endl;
    return 0;
}

struct partition* ListaMontaje::getPartition(char *id){
    //1. Obtener los valores que identifican a la particion
    char disk_letter = id[2];
    int partition_number = atoi(&(id[3]));

    //2. Recorrer los discos para buscar la particion solicitada
    struct nodoDisco *discoAnterior = nullptr;
    struct nodoDisco *discoActual = cabeceraDiscos;
    while(discoActual != nullptr){

        if(discoActual->letraDisco == disk_letter){

            //3. Recorrer las particiones del disco para buscar la particion solicitada
            struct nodoParticion *particionAnterior = nullptr;
            struct nodoParticion *particionActual= discoActual->cabeceraParticiones;
            while(particionActual != nullptr){

                if(particionActual->numeroMontaje == partition_number){
                    return &particionActual->particion;
                }

                particionAnterior = particionActual;
                particionActual = particionActual->siguiente;
            }

        }

        discoAnterior = discoActual;
        discoActual = discoActual->siguiente;
    }
    return nullptr;
}

char* ListaMontaje::getPath(char* id){

    //1. Obtener los valores que identifican a la particion
    char disk_letter = id[2];
    int partition_number = atoi(&(id[3]));

    //2. Recorrer los discos para buscar la particion solicitada
    struct nodoDisco *discoAnterior = nullptr;
    struct nodoDisco *discoActual = cabeceraDiscos;
    while(discoActual != nullptr){

        if(discoActual->letraDisco == disk_letter){

            //3. Recorrer las particiones del disco para buscar la particion solicitada
            struct nodoParticion *particionAnterior = nullptr;
            struct nodoParticion *particionActual= discoActual->cabeceraParticiones;
            while(particionActual != nullptr){

                if(particionActual->numeroMontaje == partition_number){

                    return discoActual->disk_location;
                }

                particionAnterior = particionActual;
                particionActual = particionActual->siguiente;
            }

        }

        discoAnterior = discoActual;
        discoActual = discoActual->siguiente;
    }

    return nullptr;
}

std::string ListaMontaje::getDot(){
    //1. Crear la cadena
    std::string dot;

    if(cabeceraDiscos == nullptr){
        return dot;
    }

    //Crear el primer disco
    dot += "disk";
    dot += cabeceraDiscos->letraDisco;
    dot += " [label=\"disk-";
    dot += cabeceraDiscos->letraDisco;
    dot += "\"]\n";

    //Crear las particiones del primer disco
    dot += this->getDotPartition(cabeceraDiscos->cabeceraParticiones, cabeceraDiscos->letraDisco);

    struct nodoDisco *disk = this->cabeceraDiscos;
    while(disk->siguiente != nullptr){

         //Crear el siguiente disco
         dot += "disk";
         dot += disk->siguiente->letraDisco;
         dot += " [label=\"disk-";
         dot += disk->siguiente->letraDisco;
         dot += "\"]\n";

         //Crear las particiones del siguiente disco
         dot += this->getDotPartition(disk->siguiente->cabeceraParticiones, disk->siguiente->letraDisco);

         disk = disk->siguiente;
    }

    return dot;
}

int ListaMontaje::estaMontado(char *disk_location, char* part_name){
    struct nodoDisco *disk = cabeceraDiscos;
    while(disk != nullptr){
        if(strcmp(disk_location, disk->disk_location) == 0){

            struct nodoParticion *part = disk->cabeceraParticiones;
            while(part != nullptr){
                if(strcmp(part->particion.part_name, part_name) == 0){
                    return 1;
                }
                part = part->siguiente;
            }
            return 0;
        }
        disk = disk->siguiente;
    }
    return 0;
}

std::string ListaMontaje::getDotPartition(struct nodoParticion *particion, char disk_letter){

    //1. Crear la cadena
    std::string dot;



    //Crear la primera particion
    dot += "part";
    dot += disk_letter;
    dot += std::to_string(particion->numeroMontaje);
    dot += " [label=\"vd";
    dot += disk_letter;
    dot += std::to_string(particion->numeroMontaje);
    dot += "\n";
    if(particion->particion.part_type == 'P' || particion->particion.part_type == 'p'){
        dot += "Primary";
    } else {
        dot += "Logic";
    }
    dot += "\"]\n";

    //Crear el enlace entre el disco y la primera particion
    dot += "disk";
    dot += disk_letter;
    dot += " -> ";
    dot += "part";
    dot += disk_letter;
    dot += std::to_string(particion->numeroMontaje);
    dot += "\n";

    while(particion->siguiente != nullptr){

         //Crear el siguiente disco
         dot += "part";
         dot += disk_letter;
         dot += std::to_string(particion->siguiente->numeroMontaje);
         dot += " [label=\"vd";
         dot += disk_letter;
         dot += std::to_string(particion->siguiente->numeroMontaje);
         dot += "\n";
         if(particion->particion.part_type == 'P' || particion->particion.part_type == 'p'){
             dot += "Primary";
         } else {
             dot += "Logic";
         }
         dot += "\"]\n";

        //Concatenar el actual disco con el siguiente
         dot += "part";
         dot += disk_letter;
         dot += std::to_string(particion->numeroMontaje);
         dot += " -> ";
         dot += "part";
         dot += disk_letter;
         dot += std::to_string(particion->siguiente->numeroMontaje);
         dot += "\n";

         particion = particion->siguiente;
    }

    return dot;
}
