#include "listaLogicoDisponibles.h"

ListaLogicoDisponibles::ListaLogicoDisponibles(int disk_size, int disk_start)
{
    this->cabecera = nullptr;
    this->disk_size = disk_size;
    this->disk_start = disk_start;
}

int ListaLogicoDisponibles::insertarParticion(struct ebr nuevaParticion, struct ebr particionInicial, fit posicionamiento, char* path){

    //1. Ingresar las particiones ocupadas a la lista
    FILE *archivo;
    struct ebr *aux = &particionInicial;
    struct ebr *siguiente = new struct ebr;
    while(true){

        //1.1 La particion inicial es la unica que podria estar inactiva, en ese caso, no ingresa a la lista
        if(aux->part_status == 0){
            //Verificar si es la ultima particion en lista
            if(aux->part_next == -1){
                break;
            }

            //Continuar con la siguiente particion
            archivo = fopen(path,"rb+");
            fseek(archivo,aux->part_next, SEEK_SET);
            fread(&aux,sizeof(ebr),1,archivo);
            fclose(archivo);
            continue;
        }

        //1.2 Ingresar la particion a la lista
        this->insertar(*aux);

        //1.3 Pasar a la siguiente particion
        if(aux->part_next == -1){
            break;
        } else {
            archivo = fopen(path,"rb");
            fseek(archivo,aux->part_next, SEEK_SET);
            fread(siguiente,sizeof(ebr),1,archivo);
            fclose(archivo);
            aux = siguiente;
        }
    }

    //2. Generar los nodos disponibles entre particiones
    this->generarNodosDisponibles();

    //3. Determinar que modo de posicionamiento utilizar
    struct nodoLogicoDisponible *actual = nullptr;
    struct nodoLogicoDisponible *temp = nullptr;
    switch(posicionamiento){
    case FIRSTFIT:
        //4. Buscar un nodo disponible
        actual = cabecera;
        while(actual != nullptr){
            if(actual->particion.part_status == 0 && actual->particion.part_size >= nuevaParticion.part_size){
                //Se ha encontrado el nodo donde sera almacenada la particion

                //SETEAR 'part_start' y 'part_next'
                nuevaParticion.part_start = actual->particion.part_start;
                nuevaParticion.part_next = actual->particion.part_next;
                actual->particion = nuevaParticion;

                //Corregir las relaciones entre los nodos de la lista
                struct nodoLogicoDisponible *actual = cabecera;
                struct nodoLogicoDisponible *siguiente = cabecera->siguiente;
                while(siguiente != nullptr){
                    if(strcmp(siguiente->particion.part_name, nuevaParticion.part_name) == 0){
                        actual->particion.part_next = siguiente->particion.part_start;
                    }
                    actual = siguiente;
                    siguiente = siguiente->siguiente;
                }

                //5. Guardar los cambios en las particiones
                temp = cabecera;
                while(temp != nullptr){
                    if(temp->particion.part_status == 1){


                        archivo = fopen(path,"rb+");
                        fseek(archivo,temp->particion.part_start,SEEK_SET);
                        fwrite(&temp->particion,sizeof(ebr),1,archivo);
                        fclose(archivo);


                    }
                    temp = temp->siguiente;
                }
                return 1;
            }
            actual = actual->siguiente;
        }

        //No se ha encontrado una particion libre que posea el espacio solicitado
        std::cout<<"No se ha encontrado espacio contiguo disponible para la particion"<<std::endl;
        return 0;
    case WORSTFIT:
        //4. Buscar un nodo disponible
        actual = cabecera;
        while(actual != nullptr){
            if(actual->particion.part_status == 0 && actual->particion.part_size >= nuevaParticion.part_size){
                //Se ha encontrado el nodo donde se puede almacenar

                if(temp == nullptr){
                    //Es el primer espacio disponible que se encuentra
                    temp = actual;
                } else if(actual->particion.part_size > temp->particion.part_size){
                    //Se ha encontrado otra particion que posee un mayor espacio
                    temp = actual;
                }

            }
            actual = actual->siguiente;
        }

        if(temp == nullptr){
            //No se ha encontrado una particion libre que posea el espacio solicitado
            std::cout<<"No se ha encontrado espacio contiguo disponible para la particion"<<std::endl;
            return 0;
        } else {

            //SETEAR 'part_start' y 'part_next'
            nuevaParticion.part_start = temp->particion.part_start;
            nuevaParticion.part_next = temp->particion.part_next;
            temp->particion = nuevaParticion;

            //Corregir las relaciones entre los nodos de la lista
            struct nodoLogicoDisponible *actual = cabecera;
            struct nodoLogicoDisponible *siguiente = cabecera->siguiente;
            while(siguiente != nullptr){
                if(strcmp(siguiente->particion.part_name, nuevaParticion.part_name) == 0){
                    actual->particion.part_next = siguiente->particion.part_start;
                }
                actual = siguiente;
                siguiente = siguiente->siguiente;
            }

            //5. Setear las nuevas particiones al disco
            temp = cabecera;
            while(temp != nullptr){
                if(temp->particion.part_status == 1){


                    archivo = fopen(path,"rb+");
                    fseek(archivo,temp->particion.part_start,SEEK_SET);
                    fwrite(&temp->particion,sizeof(ebr),1,archivo);
                    fclose(archivo);


                }
                temp = temp->siguiente;
            }
            return 1;
        }
    case BESTFIT:
        //4. Buscar un nodo disponible
        actual = cabecera;
        while(actual != nullptr){
            if(actual->particion.part_status == 0 && actual->particion.part_size >= nuevaParticion.part_size){
                //Se ha encontrado el nodo donde se puede almacenar

                if(temp == nullptr){
                    //Es el primer espacio disponible que se encuentra
                    temp = actual;
                } else if(actual->particion.part_size < temp->particion.part_size){
                    //Se ha encontrado otra particion que posee un mayor espacio
                    temp = actual;
                }

            }
            actual = actual->siguiente;
        }

        if(temp == nullptr){
            //No se ha encontrado una particion libre que posea el espacio solicitado
            std::cout<<"No se ha encontrado espacio contiguo disponible para la particion"<<std::endl;
            return 0;
        } else {
            //SETEAR 'part_start' y 'part_next'
            nuevaParticion.part_start = temp->particion.part_start;
            nuevaParticion.part_next = temp->particion.part_next;
            temp->particion = nuevaParticion;

            //Corregir las relaciones entre los nodos de la lista
            struct nodoLogicoDisponible *actual = cabecera;
            struct nodoLogicoDisponible *siguiente = cabecera->siguiente;
            while(siguiente != nullptr){
                if(strcmp(siguiente->particion.part_name, nuevaParticion.part_name) == 0){
                    actual->particion.part_next = siguiente->particion.part_start;
                }
                actual = siguiente;
                siguiente = siguiente->siguiente;
            }

            //5. Setear las nuevas particiones al archivo
            temp = cabecera;
            while(temp != nullptr){
                if(temp->particion.part_status == 1){


                    archivo = fopen(path,"rb+");
                    fseek(archivo,temp->particion.part_start,SEEK_SET);
                    fwrite(&temp->particion,sizeof(ebr),1,archivo);
                    fclose(archivo);


                }
                temp = temp->siguiente;
            }
            return 1;
        }
    }

    return 0;
}

void ListaLogicoDisponibles::insertar(struct ebr particion){
    struct nodoLogicoDisponible *nuevo = new struct nodoLogicoDisponible;
    nuevo->particion = particion;
    nuevo->siguiente = nullptr;

    if(cabecera == nullptr){
        cabecera = nuevo;
        nuevo->siguiente = nullptr;
    } else {
        struct nodoLogicoDisponible *aux = cabecera;
        while(aux->siguiente != nullptr){
            aux = aux->siguiente;
        }
        aux->siguiente = nuevo;
    }
}

void ListaLogicoDisponibles::generarNodosDisponibles(){

    //1. Determinar si no hay particiones ocupadas en el disco
    if(cabecera == nullptr){
        //1.1 Crear el nodo disponible
        struct nodoLogicoDisponible *disponible = new struct nodoLogicoDisponible;
        disponible->particion.part_status = 0;
        disponible->particion.part_start = disk_start;
        disponible->particion.part_size = disk_size;
        disponible->particion.part_next = -1;

        //1.2 Agregar el nodo a la lista
        cabecera = disponible;
        cabecera->siguiente = nullptr;
        return;
    }

    //1. Determinar si hay espacio entre el inicio del disco y la primera particion
    if(cabecera->particion.part_start - disk_start > 0){
        // 1.1 Crear el nodo disponible
        struct nodoLogicoDisponible *disponible = new struct nodoLogicoDisponible;
        disponible->particion.part_status = 0;
        disponible->particion.part_start = disk_start;
        disponible->particion.part_size = (cabecera->particion.part_start-1) - disk_start + 1;      //limiteSuperior - limiteInferior + 1
        disponible->particion.part_next = cabecera->particion.part_start;

        // 1.2 Agregar el nodo al principio de la lista
        disponible->siguiente = cabecera;
        cabecera = disponible;
    }

    //2. Determinar si hay espacio entre los nodos ocupados de la lista
    struct nodoLogicoDisponible *actual = cabecera;
    struct nodoLogicoDisponible *siguiente = cabecera->siguiente;
    while(siguiente != nullptr){
        if(siguiente->particion.part_start - (actual->particion.part_start + actual->particion.part_size) > 0){
            // 2.2 Crear el nodo disponible
            struct nodoLogicoDisponible *disponible = new struct nodoLogicoDisponible;
            disponible->particion.part_status = 0;
            disponible->particion.part_start = actual->particion.part_start + actual->particion.part_size;
            disponible->particion.part_size = (siguiente->particion.part_start - 1) - (actual->particion.part_start + actual->particion.part_size) + 1;      //limiteSuperior - limiteInferior + 1
            disponible->particion.part_next = siguiente->particion.part_start;

            //2.2 Agregar el nodo disponible entre los nodos ocupados
            actual->siguiente = disponible;
            disponible->siguiente = siguiente;
        }

        actual = siguiente;
        siguiente = siguiente->siguiente;
    }

    //3. Determinar si hay espacio entre el fin del disco y la ultima particion
    if((this->disk_start + this->disk_size) - (actual->particion.part_start + actual->particion.part_size) > 0){
        // 3.1 Crear el nodo disponible
        struct nodoLogicoDisponible *disponible = new struct nodoLogicoDisponible;
        disponible->siguiente = nullptr;
        disponible->particion.part_status = 0;
        disponible->particion.part_start = actual->particion.part_start + actual->particion.part_size;
        disponible->particion.part_size = (this->disk_start + this->disk_size - 1) - (actual->particion.part_start + actual->particion.part_size) + 1;//limiteSuperior - limiteInferior + 1
        disponible->particion.part_next = -1;

        // 3.2 Agregar el nodo al final de la lista
        actual->siguiente = disponible;
    }
}
