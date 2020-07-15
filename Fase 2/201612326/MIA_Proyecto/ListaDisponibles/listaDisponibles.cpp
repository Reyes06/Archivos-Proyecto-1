    #include "listaDisponibles.h"

ListaDisponibles::ListaDisponibles(int disk_size)
{
    this->cabecera = nullptr;
    this->disk_size = disk_size;
    this->disk_start = sizeof(struct mbr) + 1;
}

void ListaDisponibles::insertar(struct partition particion){
    struct nodoDisponible *nuevo = new struct nodoDisponible;
    nuevo->particion = particion;
    nuevo->siguiente = nullptr;

    if(cabecera == nullptr){
        cabecera = nuevo;
        nuevo->siguiente = nullptr;
    } else {
        struct nodoDisponible *aux = cabecera;
        while(aux->siguiente != nullptr){
            aux = aux->siguiente;
        }
        aux->siguiente = nuevo;
    }
}

void ListaDisponibles::generarNodosDisponibles(){

    //1. Determinar si no hay particiones ocupadas en el disco
    if(cabecera == nullptr){
        //1.1 Crear el nodo disponible
        struct nodoDisponible *disponible = new struct nodoDisponible;
        disponible->particion.part_status = 0;
        disponible->particion.part_start = disk_start;
        disponible->particion.part_size = disk_size;

        //1.2 Agregar el nodo a la lista
        cabecera = disponible;
        cabecera->siguiente = nullptr;
        return;
    }

    //1. Determinar si hay espacio entre el inicio del disco y la primera particion
    if(cabecera->particion.part_start - disk_start > 0){
        // 1.1 Crear el nodo disponible
        struct nodoDisponible *disponible = new struct nodoDisponible;
        disponible->particion.part_status = 0;
        disponible->particion.part_start = disk_start;
        disponible->particion.part_size = (cabecera->particion.part_start-1) - disk_start + 1;      //limiteSuperior - limiteInferior + 1

        // 1.2 Agregar el nodo al principio de la lista
        disponible->siguiente = cabecera;
        cabecera = disponible;
    }

    //2. Determinar si hay espacio entre los nodos ocupados de la lista
    struct nodoDisponible *actual = cabecera;
    struct nodoDisponible *siguiente = cabecera->siguiente;
    while(siguiente != nullptr){
        if(siguiente->particion.part_start - (actual->particion.part_start + actual->particion.part_size) > 0){
            // 2.2 Crear el nodo disponible
            struct nodoDisponible *disponible = new struct nodoDisponible;
            disponible->particion.part_status = 0;
            disponible->particion.part_start = actual->particion.part_start + actual->particion.part_size;
            disponible->particion.part_size = (siguiente->particion.part_start - 1) - (actual->particion.part_start + actual->particion.part_size) + 1;      //limiteSuperior - limiteInferior + 1

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
        struct nodoDisponible *disponible = new struct nodoDisponible;
        disponible->siguiente = nullptr;
        disponible->particion.part_status = 0;
        disponible->particion.part_start = actual->particion.part_start + actual->particion.part_size;
        disponible->particion.part_size = (this->disk_start + this->disk_size - 1) - (actual->particion.part_start + actual->particion.part_size) + 1;//limiteSuperior - limiteInferior + 1

        // 3.2 Agregar el nodo al final de la lista
        actual->siguiente = disponible;
    }
}

int ListaDisponibles::insertarParticion(struct partition nuevaParticion, struct mbr *MBR, fit posicionamiento){

    //1. Ingresar las particiones ocupadas a la lista
    for(int i = 0; i < 4; i++){
        if(MBR->mbr_partition[i].part_status == 1){
            insertar(MBR->mbr_partition[i]);
        }
    }

    //2. Generar los nodos disponibles entre particiones
    this->generarNodosDisponibles();

    //3. Determinar que modo de posicionamiento utilizar
    struct nodoDisponible *actual = nullptr;
    struct nodoDisponible *temp = nullptr;
    int contador = 0;
    switch(posicionamiento){
    case FIRSTFIT:
        //4. Buscar un nodo disponible
        actual = cabecera;
        while(actual != nullptr){
            if(actual->particion.part_status == 0 && actual->particion.part_size >= nuevaParticion.part_size){
                //Se ha encontrado el nodo donde sera almacenada la particion

                nuevaParticion.part_start = actual->particion.part_start;                   //SETEAR 'part_start'
                actual->particion = nuevaParticion;

                //5. Setear las nuevas particiones al MBR
                temp = cabecera;
                while(temp != nullptr){
                    if(temp->particion.part_status == 1){
                        MBR->mbr_partition[contador] = temp->particion;
                        contador++;
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
            nuevaParticion.part_start = temp->particion.part_start;                   //SETEAR 'part_start'
            temp->particion = nuevaParticion;

            //5. Setear las nuevas particiones al MBR
            temp = cabecera;
            while(temp != nullptr){
                if(temp->particion.part_status == 1){
                    MBR->mbr_partition[contador] = temp->particion;
                    contador++;
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
            nuevaParticion.part_start = temp->particion.part_start;                   //SETEAR 'part_start'
            temp->particion = nuevaParticion;

            //5. Setear las nuevas particiones al MBR
            temp = cabecera;
            while(temp != nullptr){
                if(temp->particion.part_status == 1){
                    MBR->mbr_partition[contador] = temp->particion;
                    contador++;
                }
                temp = temp->siguiente;
            }
            return 1;
        }
    }

}

void ListaDisponibles::getDotDisk(char *path, std::ofstream &fs, struct mbr *MBR){

    //1. Ingresar las particiones ocupadas a la lista
    bool existeExtendida = false;
    for(int i = 0; i < 4; i++){
        if(MBR->mbr_partition[i].part_status == 1){
            insertar(MBR->mbr_partition[i]);
            if(MBR->mbr_partition[i].part_type == 'e' || MBR->mbr_partition[i].part_type == 'E'){
                existeExtendida = true;
            }
        }
    }

    //2. Generar los nodos libres entre particiones
    this->generarNodosDisponibles();

    //2.1 Verificar si existe particion extendida/logica
    int rowspan;
    if(existeExtendida){
        rowspan = 2;
    } else {
        rowspan = 1;
    }

    //2.2 Graficar el MBR
    fs << "<tr>\n";
    fs << "<td rowspan=\""<<rowspan + 1<<"\" color=\"red\"><b>MBR</b></td>\n";

    //3. Graficas las particiones primarias, extendida y espacios libres
    ListaLogicoDisponibles *listaLogica = nullptr;
    struct nodoDisponible *aux = cabecera;
    while(aux != nullptr){
        if(aux->particion.part_status == 1 && (aux->particion.part_type == 'p' || aux->particion.part_type == 'P')){
            fs << "<td rowspan=\""<<rowspan<<"\" color=\"orange\">[PRI] "<<aux->particion.part_name<<"</td>\n";
        } else if (aux->particion.part_status == 0){
            fs << "<td rowspan=\""<<rowspan<<"\" color=\"gray\">Libre</td>\n";
        } else if(aux->particion.part_status == 1 && (aux->particion.part_type == 'e' || aux->particion.part_type == 'E')){

            int contadorBloques = 0;
            struct nodoDisponible *partExtendida = aux;
            listaLogica = new ListaLogicoDisponibles(partExtendida->particion.part_size, partExtendida->particion.part_start);

            struct ebr *partLogica = new struct ebr;
            FILE *archivo = fopen(path,"rb+");
            fseek(archivo,partExtendida->particion.part_start,SEEK_SET);
            fread(partLogica,sizeof(ebr),1,archivo);
            fclose(archivo);

            //Cargar las particiones logicas a memoria
            while(true){
                if(partLogica->part_status == 0){
                    contadorBloques++;
                    if(partLogica->part_next == -1){
                        break;
                    } else {
                        archivo = fopen(path,"rb+");
                        fseek(archivo,partLogica->part_next,SEEK_SET);
                        fread(partLogica,sizeof(ebr),1,archivo);
                        fclose(archivo);
                        continue;
                    }
                }

                listaLogica->insertar(*partLogica);

                if(partLogica->part_next == -1){
                    break;
                } else {
                    archivo = fopen(path,"rb+");
                    fseek(archivo,partLogica->part_next,SEEK_SET);
                    fread(partLogica,sizeof(ebr),1,archivo);
                    fclose(archivo);
                    continue;
                }
            }

            //Generar los espacios disponibles entre particiones
            listaLogica->generarNodosDisponibles();

            //Recorrer la lista para determinar la cantidad de particiones bloques a colocar
            struct nodoLogicoDisponible *nodoLogico = listaLogica->cabecera;
            while(nodoLogico != nullptr){

                if(nodoLogico->particion.part_status == 1){
                    contadorBloques += 2;
                } else {
                    contadorBloques++;
                }
                nodoLogico = nodoLogico->siguiente;
            }

            //Escribir la etiqueta de particion extendida
            fs << "<td colspan=\""<<contadorBloques<<"\" color=\"blue\">[EXT] "<<partExtendida->particion.part_name<<"</td>\n";
        }
        aux = aux->siguiente;
    }

    fs << "</tr>\n";


    //4. Graficar las particiones logicas y libres (Si hubieran)
    if(listaLogica != nullptr){
        fs << "<tr>\n";
        struct nodoLogicoDisponible *nodoLogico = listaLogica->cabecera;
        if(nodoLogico->particion.part_status == 0){
            fs << "<td color=\"green\"><b>EBR</b></td>\n";
        }
        while(nodoLogico != nullptr){
            if(nodoLogico->particion.part_status == 1){
                fs << "<td color=\"green\"><b>EBR</b></td>\n";
                fs << "<td color=\"green\">[LOG] "<<nodoLogico->particion.part_name<<"</td>\n";
            } else {
                fs << "<td color=\"gray\">Libre</td>\n";
            }
            nodoLogico = nodoLogico->siguiente;
        }
        fs << "</tr>\n";
    }

    fs << "<tr>\n";

    //5. Graficar los porcentajes de cada particion ==================================================================
    aux = this->cabecera;
    float porcentaje;
    float numerador;
    float denominador;
    while(aux != nullptr){
        if(aux->particion.part_status == 1 && (aux->particion.part_type == 'p' || aux->particion.part_type == 'P')){
            numerador = aux->particion.part_size;
            denominador = this->disk_size;
            porcentaje = (numerador/denominador)*100;
            fs << "<td color=\"orange\">" << std::fixed << std::setprecision(2) << porcentaje << "%</td>\n";
        } else if (aux->particion.part_status == 0){
            numerador = aux->particion.part_size;
            denominador = this->disk_size;
            porcentaje = (numerador/denominador)*100;
            fs << "<td color=\"gray\">" << std::fixed << std::setprecision(2) << porcentaje << "%</td>\n";
        } else if(aux->particion.part_status == 1 && (aux->particion.part_type == 'e' || aux->particion.part_type == 'E')){

            struct ebr *partLogica = new struct ebr;
            FILE *archivo = fopen(path,"rb+");
            fseek(archivo,aux->particion.part_start,SEEK_SET);
            fread(partLogica,sizeof(ebr),1,archivo);
            fclose(archivo);

            //Recorrer la lista logica para determinar la cantidad de particiones bloques a colocar
            struct nodoLogicoDisponible *nodoLogico = listaLogica->cabecera;
            if(nodoLogico->particion.part_status == 0){
                fs << "<td color=\"green\"></td>\n";
            }
            while(nodoLogico != nullptr){

                if(nodoLogico->particion.part_status == 1){
                    numerador = nodoLogico->particion.part_size;
                    denominador = this->disk_size;
                    porcentaje = (numerador/denominador)*100;
                    fs << "<td colspan=\"2\" color=\"green\">" << std::fixed << std::setprecision(2) << porcentaje << "%</td>\n";
                } else {
                    numerador = nodoLogico->particion.part_size;
                    denominador = this->disk_size;
                    porcentaje = (numerador/denominador)*100;
                    fs << "<td color=\"gray\">" << std::fixed << std::setprecision(2) << porcentaje << "%</td>\n";
                }
                nodoLogico = nodoLogico->siguiente;
            }
        }
        aux = aux->siguiente;
    }

    fs << "</tr>\n";
}
