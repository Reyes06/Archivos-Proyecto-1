#include "Comandos/rep.h"

int ejecutarRep(struct comando comando, ListaMontaje *listaMontaje){
    std::map<TipoParametro, struct parametro> parametros = (*comando.parametros);
    switch(parametros[T_NAME].valor[0]){
    case 'm':
        return graficarMbr(parametros, listaMontaje);
    case 'd':
        return graficarDisk(parametros,listaMontaje);
    case 'y':
        return graficarListaMontaje(listaMontaje);
    default:
        std::cout<<"El valor indicado es invalido (NAME)"<<std::endl;
        return 0;
    }

    return 1;
}

int graficarMbr(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje){

    char* path = listaMontaje->getPath(parametros[T_ID].valor);
    if(path == nullptr){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        return 0;
    }

    struct mbr MBR;
    FILE *archivo = fopen(path,"rb+");
    fread(&MBR, sizeof(MBR),1,archivo);
    fclose(archivo);

    std::ofstream fs("dot.txt");
    fs << "digraph G {\n";

    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &MBR.mbr_fecha_creacion);

    fs << "nodo [shape=plaintext label=<\n";
    fs << "<table>\n";
    fs <<  "<tr>\n";
    fs <<      "<td>Nombre</td>\n";
    fs <<      "<td>Valor</td>\n";
    fs <<  "</tr>\n";
    fs <<  "<tr>\n";
    fs <<      "<td>mbr_tamano</td>\n";
    fs <<      "<td>";
    fs <<      MBR.mbr_tamano;
    fs <<      "</td>\n";
    fs <<  "</tr>\n";
    fs <<  "<tr>\n";
    fs <<      "<td>mbr_fecha_creacion</td>\n";
    fs <<      "<td>";
    fs <<      buf;
    fs <<      "</td>\n";
    fs <<  "</tr>\n";
    fs <<  "<tr>\n";
    fs <<      "<td>mbr_disk_signature</td>\n";
    fs <<      "<td>";
    fs <<      MBR.mbr_disk_signature;
    fs <<      "</td>\n";
    fs <<  "</tr>\n";
    fs <<  "<tr>\n";
    fs <<      "<td>disk_fit</td>\n";
    fs <<      "<td>";
    fs <<      MBR.disk_fit;
    fs <<      "</td>\n";
    fs <<  "</tr>\n";

    int contador = 1;
    for(struct partition particion : MBR.mbr_partition){
        fs <<  "<tr>\n";
        fs <<      "<td>part_status_";
        fs <<      contador;
        fs <<      "</td>\n";
        fs <<      "<td>";
        fs <<      particion.part_status + 0;
        fs <<      "</td>\n";
        fs <<  "</tr\n>";
        if(particion.part_status){
            fs <<  "<tr>\n";
            fs <<      "<td>part_type_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td>";
            fs <<      particion.part_type;
            fs <<      "</td>\n";
            fs <<  "</tr>\n";
            fs <<  "<tr>\n";
            fs <<      "<td>part_fit_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td>";
            fs <<      particion.part_fit;
            fs <<      "</td>\n";
            fs <<  "</tr>\n";
            fs <<  "<tr>\n";
            fs <<      "<td>part_start_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td>";
            fs <<      particion.part_start;
            fs <<      "</td>\n";
            fs <<  "</tr>\n";
            fs <<  "<tr>\n";
            fs <<      "<td>part_size_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td>";
            fs <<      particion.part_size;
            fs <<      "</td>\n";
            fs <<  "</tr>\n";
            fs <<  "<tr>\n";
            fs <<      "<td>part_name_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td>";
            fs <<      particion.part_name;
            fs <<      "</td>\n";
            fs <<  "</tr>\n";
        } else {
            fs <<  "<tr>\n";
            fs <<      "<td>part_type_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td> - </td>\n";
            fs <<  "</tr>\n";
            fs <<  "<tr>\n";
            fs <<      "<td>part_fit_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td> - </td>\n";
            fs <<  "</tr>\n";
            fs <<  "<tr>\n";
            fs <<      "<td>part_start_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td> - </td>\n";
            fs <<  "</tr>\n";
            fs <<  "<tr>\n";
            fs <<      "<td>part_size_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td> - </td>\n";
            fs <<  "</tr>\n";
            fs <<  "<tr>\n";
            fs <<      "<td>part_name_";
            fs <<      contador;
            fs <<      "</td>\n";
            fs <<      "<td> - </td>\n";
            fs <<  "</tr>\n";
        }

        contador++;
    }

    fs << "</table>\n";
    fs << ">]\n";

    for(struct partition particion : MBR.mbr_partition){
        if(particion.part_status == 1 && (particion.part_type == 'e' || particion.part_type == 'E')){



            struct ebr EBR;
            archivo = fopen(path, "rb+");
            fseek(archivo,particion.part_start,SEEK_SET);
            fread(&EBR,sizeof(ebr),1,archivo);
            fclose(archivo);

            if(EBR.part_status == 1){
                fs << "nodo1";
                fs << " [shape=plaintext label=<\n";
                fs << "<table>\n";
                fs <<   "<tr>\n";
                fs <<       "<td>Nombre</td>\n";
                fs <<       "<td>Valor</td>\n";
                fs <<   "</tr>\n";
                fs <<   "<tr>\n";
                fs <<       "<td>part_status_1";
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_status + 0;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_fit_1";
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_fit;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_start_1";
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_start;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_size_1";
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_size;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_next_1";
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_next;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_name_1";
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_name;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs << "</table>";
                fs << ">]\n";
            }

            int contador = 2;
            while(EBR.part_next != -1){

                archivo = fopen(path, "rb+");
                fseek(archivo,EBR.part_next,SEEK_SET);
                fread(&EBR,sizeof(ebr),1,archivo);
                fclose(archivo);

                fs << "nodo";
                fs << contador;
                fs << " [shape=plaintext label=<\n";
                fs << "<table>\n";
                fs <<   "<tr>\n";
                fs <<       "<td>Nombre</td>\n";
                fs <<       "<td>Valor</td>\n";
                fs <<   "</tr>\n";
                fs <<   "<tr>\n";
                fs <<       "<td>part_status_";
                fs <<       contador;
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_status + 0;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_fit_";
                fs <<       contador;
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_fit;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_start_";
                fs <<       contador;
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_start;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_size_";
                fs <<       contador;
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_size;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_next_";
                fs <<       contador;
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_next;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs <<   "<tr>\n";
                fs <<       "<td>part_name_";
                fs <<       contador;
                fs <<       "</td>\n";
                fs <<       "<td>";
                fs <<       EBR.part_name;
                fs <<       "</td>\n";
                fs <<   "</tr>";
                fs << "</table>";
                fs << ">]\n";

                contador++;

            }

            break;
        }
    }

    fs << "}";
    fs.close();

    crearDirectorio(parametros[T_PATH].valor);

    if(verificarSubCadena(parametros[T_PATH].valor, "png")){

        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpng dot.txt -o ");
        strcat(instruccion,parametros[T_PATH].valor);
        system(instruccion);

        return 1;

    } else if (verificarSubCadena(parametros[T_PATH].valor, "jpg")){

        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tjpg dot.txt -o ");
        strcat(instruccion,parametros[T_PATH].valor);
        system(instruccion);

        return 1;

    } else {

        std::cout<<"No se ha reconocido el formato de la imagen solictada (png o jpg)"<<std::endl;
        return 0;
    }
}

int graficarDisk(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje){

    char* path = listaMontaje->getPath(parametros[T_ID].valor);
    if(path == nullptr){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        return 0;
    }

    struct mbr *MBR = new struct mbr;
    FILE *archivo = fopen(path, "rb+");
    fread(MBR,sizeof(mbr),1,archivo);
    fclose(archivo);

    std::ofstream fs("dot.txt");
    fs << "digraph G {\n";
    fs << "nodo [shape=plaintext label=<\n";
    fs << "<table>";

    ListaDisponibles *ld = new ListaDisponibles(MBR->mbr_tamano);
    ld->getDotDisk(path,fs, MBR);

    fs << "</table>";
    fs << ">];";
    fs << "}";
    fs.close();

    crearDirectorio(parametros[T_PATH].valor);

    if(verificarSubCadena(parametros[T_PATH].valor, "png")){

        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpng dot.txt -o ");
        strcat(instruccion,parametros[T_PATH].valor);
        system(instruccion);

        return 1;

    } else if (verificarSubCadena(parametros[T_PATH].valor, "jpg")){

        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tjpg dot.txt -o ");
        strcat(instruccion,parametros[T_PATH].valor);
        system(instruccion);

        return 1;

    } else {

        std::cout<<"No se ha reconocido el formato de la imagen solictada (png o jpg)"<<std::endl;
        return 0;
    }

    return 0;
}

int graficarListaMontaje(ListaMontaje *listaMontaje){
    std::string dot = "digraph G {\n";
    dot += listaMontaje->getDot();
    dot += "}";

    std::ofstream fs("dot.txt");
    fs << dot;
    fs.close();

    return 1;
}

int verificarSubCadena( char *cadena, char *subcadena )
{
   char *tmp = cadena;
   char *pdest;

   pdest = strstr( tmp, subcadena );
   if( pdest ) return 1;

   return 0;
}
