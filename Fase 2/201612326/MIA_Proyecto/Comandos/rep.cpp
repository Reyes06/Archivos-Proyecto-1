#include "Comandos/rep.h"

int ejecutarRep(struct comando comando, ListaMontaje *listaMontaje){
    std::map<TipoParametro, struct parametro> parametros = (*comando.parametros);

    if(strcmp(parametros[T_NAME].valor,"mbr")==0)
        return graficarMbr(parametros, listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"disk")==0)
        return graficarDisk(parametros,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"l_montaje")==0)
        return graficarListaMontaje(parametros, listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"inode")==0)
        return graficarInode(parametros,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"journaling")==0)
        return graficarJournaling(parametros,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"block")==0)
        return graficarBlock(parametros,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"bm_inode")==0)
        return graficarBmInode(parametros,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"bm_block")==0)
        return graficarBmBlock(parametros,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"tree")==0)
        return graficarTree(parametros,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"sb")==0)
        return graficarSb(parametros,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"file")==0)
        return File::ejecutarFile(comando,listaMontaje);
    if(strcmp(parametros[T_NAME].valor,"ls")==0)
        return Ls::ejecutarLs(comando,listaMontaje);

    std::cout<<"El valor indicado es invalido (NAME)"<<std::endl;
    return 0;
}

int graficarMbr(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje){

    char* path = listaMontaje->getDiskLocation(parametros[T_ID].valor);
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

    char* path = listaMontaje->getDiskLocation(parametros[T_ID].valor);
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

int graficarListaMontaje(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje){
    std::string dot = "digraph G {\n";
    dot += listaMontaje->getDot();
    dot += "}";

    std::ofstream fs("dot.txt");
    fs << dot;
    fs.close();

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

int graficarInode(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje){
    //Obtener el superBloque
    struct partition particion = listaMontaje->getPrimaryMountPartition(parametros[T_ID].valor);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    struct superBlock *superBloque = new struct superBlock;
    struct inodeTable *inodo = new struct inodeTable;
    char inode_state;
    int inodo_anterior = -1;
    int inodo_actual = 0;

    char *disk_location = listaMontaje->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb");
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);

    std::ofstream fs("dot.txt");

    fs << "digraph G {\n";
    fs << "rankdir=LR;\n";

    //Obtener los inodos
    while(inodo_actual < superBloque->s_inodes_count ){
        fseek(archivo,superBloque->s_bm_inode_start + inodo_actual*sizeof(char),SEEK_SET);
        fread(&inode_state,sizeof(char),1,archivo);
        if(inode_state == 1){
            fseek(archivo, superBloque->s_inode_start + inodo_actual*sizeof(inodeTable), SEEK_SET);
            fread(inodo,sizeof(inodeTable),1,archivo);

            char atime[80];
            strftime(atime, sizeof(atime), "%Y-%m-%d.%X", &inodo->i_atime);
            char ctime[80];
            strftime(ctime, sizeof(ctime), "%Y-%m-%d.%X", &inodo->i_ctime);
            char mtime[80];
            strftime(mtime, sizeof(mtime), "%Y-%m-%d.%X", &inodo->i_mtime);

            //Graficar el inodo
            fs << "inodo"<<inodo_actual<<" [color=\"orange\" style=\"filled\" shape=\"box\" label=<\n";
            fs << "<table color=\"black\" border=\"0\">\n";
            fs <<   "<tr>\n";
            fs <<       "<td colspan=\"2\"><b>Inodo "<<inodo_actual<<"</b></td>\n";
            fs <<   "</tr>\n";
            fs <<   "<tr>\n";
            fs <<       "<td>i_uid</td>\n";
            fs <<       "<td>"<<inodo->i_uid<<"</td>\n";
            fs <<   "</tr>\n";
            fs <<   "<tr>\n";
            fs <<       "<td>i_gid</td>\n";
            fs <<       "<td>"<<inodo->i_gid<<"</td>\n";
            fs <<   "</tr>\n";
            fs <<   "<tr>\n";
            fs <<       "<td>i_size</td>\n";
            fs <<       "<td>"<<inodo->i_size<<"</td>\n";
            fs <<   "</tr>\n";
            fs <<   "<tr>\n";
            fs <<       "<td>i_atime</td>\n";
            fs <<       "<td>"<<atime<<"</td>\n";
            fs <<   "</tr>\n";
            fs <<   "<tr>\n";
            fs <<       "<td>i_ctime</td>\n";
            fs <<       "<td>"<<ctime<<"</td>\n";
            fs <<   "</tr>\n";
            fs <<   "<tr>\n";
            fs <<       "<td>i_ctime</td>\n";
            fs <<       "<td>"<<mtime<<"</td>\n";
            fs <<   "</tr>\n";
            for(int i = 0; i < 15; i++){
                fs <<   "<tr>\n";
                fs <<       "<td>i_block_"<<i<<"</td>\n";
                fs <<       "<td>"<<inodo->i_block[i]<<"</td>\n";
                fs <<   "</tr>\n";
            }
            fs <<   "<tr>\n";
            fs <<       "<td>i_type</td>\n";
            fs <<       "<td>"<<inodo->i_type+0<<"</td>\n";
            fs <<   "</tr>\n";
            fs <<   "<tr>\n";
            fs <<       "<td>i_perm</td>\n";
            fs <<       "<td>"<<inodo->i_perm<<"</td>\n";
            fs <<   "</tr>\n";
            fs << "</table>\n";
            fs << ">];\n";

            if(inodo_anterior == -1){
                inodo_anterior = inodo_actual;
            } else {
                fs << "inodo"<<inodo_anterior<<"->inodo"<<inodo_actual<<";\n";
                inodo_anterior = inodo_actual;
            }
        }



        inodo_actual++;
    }

    fs <<"}";

    fs.close();
    fclose(archivo);

    if(verificarSubCadena(parametros[T_PATH].valor, ".pdf")){
        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpdf dot.txt -o ");
        strcat(instruccion,parametros[T_PATH].valor);
        system(instruccion);
    } else {
        char path[100]; path[0] = '\0';
        strcat(path, parametros[T_PATH].valor);
        strcat(path, ".pdf");
        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpdf dot.txt -o ");
        strcat(instruccion,path);
        system(instruccion);
    }

    return 1;
}

int graficarBmInode(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje){
    //Obtener el superBloque
    struct partition particion = listaMontaje->getPrimaryMountPartition(parametros[T_ID].valor);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    struct superBlock *superBloque = new struct superBlock;
    char inode_state;
    int contadorFilas = 0;
    int contadorColumnas = 0;
    std::ofstream fs(parametros[T_PATH].valor);

    char *disk_location = listaMontaje->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb");
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);
    fseek(archivo,superBloque->s_bm_inode_start,SEEK_SET);


    while(20*contadorFilas + contadorColumnas < superBloque->s_inodes_count ){

        fread(&inode_state,sizeof(char),1,archivo);

        if(contadorColumnas == 20){
            fs << "\n";
            fs << inode_state + 0 << " ";
            contadorFilas++;
            contadorColumnas = 1;
            continue;
        }

        fs << inode_state + 0 << " ";

        contadorColumnas++;
    }

    fs.close();
    fclose(archivo);

    return 1;
}

int graficarBmBlock(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje){
    //Obtener el superBloque
    struct partition particion = listaMontaje->getPrimaryMountPartition(parametros[T_ID].valor);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    struct superBlock *superBloque = new struct superBlock;
    char block_state;
    int contadorFilas = 0;
    int contadorColumnas = 0;
    std::ofstream fs(parametros[T_PATH].valor);

    char *disk_location = listaMontaje->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb");
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);
    fseek(archivo,superBloque->s_bm_block_start,SEEK_SET);


    while(20*contadorFilas + contadorColumnas < superBloque->s_blocks_count ){

        fread(&block_state,sizeof(char),1,archivo);

        if(contadorColumnas == 20){
            fs << "\n";
            fs << block_state + 0 << " ";
            contadorFilas++;
            contadorColumnas = 1;
            continue;
        }

        fs << block_state + 0 << " ";

        contadorColumnas++;
    }

    fs.close();
    fclose(archivo);

    return 1;
}

int graficarSb(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje){
    //Obtener el superBloque
    struct partition particion = listaMontaje->getPrimaryMountPartition(parametros[T_ID].valor);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    struct superBlock *superBloque = new struct superBlock;
    std::ofstream fs("dot.txt");

    char *disk_location = listaMontaje->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb");
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);
    fseek(archivo,superBloque->s_bm_block_start,SEEK_SET);
    fclose(archivo);

    char mtime[80];
    strftime(mtime, sizeof(mtime), "%Y-%m-%d.%X", &superBloque->s_mtime);
    char umtime[80];
    strftime(umtime, sizeof(umtime), "%Y-%m-%d.%X", &superBloque->s_umtime);


    fs << "digraph G {\n";
    fs << "superblock [shape=\"box\" label=<";
    fs << "<table>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_inodes_count</td>\n";
    fs <<       "<td>"<<superBloque->s_inodes_count<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_blocks_count</td>\n";
    fs <<       "<td>"<<superBloque->s_blocks_count<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_free_blocks_count</td>\n";
    fs <<       "<td>"<<superBloque->s_free_blocks_count<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_free_inodes_count</td>\n";
    fs <<       "<td>"<<superBloque->s_free_inodes_count<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_mtime</td>\n";
    fs <<       "<td>"<<mtime<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_umtime</td>\n";
    fs <<       "<td>"<<umtime<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_mnt_count</td>\n";
    fs <<       "<td>"<<superBloque->s_mnt_count<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_magic</td>\n";
    fs <<       "<td>0xEF53</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_inode_size</td>\n";
    fs <<       "<td>"<<superBloque->s_inode_size<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_block_size</td>\n";
    fs <<       "<td>"<<superBloque->s_block_size<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_first_ino</td>\n";
    fs <<       "<td>"<<superBloque->s_first_ino<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_first_blo</td>\n";
    fs <<       "<td>"<<superBloque->s_first_blo<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_bm_inode_start</td>\n";
    fs <<       "<td>"<<superBloque->s_bm_inode_start<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_bm_block_start</td>\n";
    fs <<       "<td>"<<superBloque->s_bm_block_start<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_inode_start</td>\n";
    fs <<       "<td>"<<superBloque->s_inode_start<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>s_block_start</td>\n";
    fs <<       "<td>"<<superBloque->s_block_start<<"</td>\n";
    fs <<   "</tr>\n";
    fs << "</table>\n";
    fs << ">];\n";
    fs << "}";
    fs.close();

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


    return 1;
}

int graficarJournaling(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje){
    //Obtener el superBloque
    struct partition particion = listaMontaje->getPrimaryMountPartition(parametros[T_ID].valor);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    struct journal *journaling = new struct journal;
    std::ofstream fs("dot.txt");

    char *disk_location = listaMontaje->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb");
    fseek(archivo,particion.part_start+sizeof(superBlock),SEEK_SET);
    fread(journaling,sizeof(journal),1,archivo);

    fs << "Digraph G {\n";

    fs << "journal [shape=plaintext label=<\n";
    fs << "<table>\n";

    fs << "<tr>\n";
    fs <<   "<td><b>Time</b></td>\n";
    fs <<   "<td><b>User</b></td>\n";
    fs <<   "<td><b>Part_name</b></td>\n";
    fs <<   "<td><b>Operation</b></td>\n";
    fs <<   "<td><b>Fs</b></td>\n";
    fs << "</tr>\n";

    while(journaling->active){

        char time[80];
        strftime(time, sizeof(time), "%Y-%m-%d.%X", &journaling->hora);
        fs << "<tr>\n";
        fs <<   "<td>"<<time<<"</td>\n";
        fs <<   "<td>"<<journaling->user<<"</td>\n";
        fs <<   "<td>"<<particion.part_name<<"</td>\n";
        fs <<   "<td>"<<journaling->operation<<"</td>\n";
        fs <<   "<td>Ext3</td>\n";
        fs << "</tr>\n";
        fread(journaling,sizeof(journal),1,archivo);
    }

    fs << "</table>\n";
    fs << ">];\n";
    fs << "}";


    fclose(archivo);
    fs.close();

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


    return 1;
}

int graficarTree(std::map<TipoParametro, struct parametro> parametros,ListaMontaje *listaMontaje){

    //Obtener el superBloque
    struct partition particion = listaMontaje->getPrimaryMountPartition(parametros[T_ID].valor);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    struct superBlock *superBloque = new struct superBlock;

    char *disk_location = listaMontaje->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb");
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);

    std::ofstream fs("dot.txt");

    fs << "digraph G {\n";
    fs << "rankdir=LR;\n";
    visitar_Inodo(0,superBloque,archivo,fs, 1);
    fs << "}";
    fs.close();

    if(verificarSubCadena(parametros[T_PATH].valor, ".pdf")){
        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpdf dot.txt -o ");
        strcat(instruccion,parametros[T_PATH].valor);
        system(instruccion);
    } else {
        char path[100]; path[0] = '\0';
        strcat(path, parametros[T_PATH].valor);
        strcat(path, ".pdf");
        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpdf dot.txt -o ");
        strcat(instruccion,path);
        system(instruccion);
    }
    return 1;
}

void visitar_Inodo(int indice_inodo, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group){

    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    char atime[80];
    strftime(atime, sizeof(atime), "%Y-%m-%d.%X", &inodo->i_atime);
    char ctime[80];
    strftime(ctime, sizeof(ctime), "%Y-%m-%d.%X", &inodo->i_ctime);
    char mtime[80];
    strftime(mtime, sizeof(mtime), "%Y-%m-%d.%X", &inodo->i_mtime);

    //Graficar el inodo
    fs << "\ninodo"<<indice_inodo<<" [group="<<dot_group<<" shape=plaintext color=\"orange\" style=\"filled\" label=<\n";
    fs << "<table color=\"black\">\n";
    fs <<   "<tr>\n";
    fs <<       "<td colspan=\"2\"><b>Inodo "<<indice_inodo<<"</b></td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>i_uid</td>\n";
    fs <<       "<td>"<<inodo->i_uid<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>i_gid</td>\n";
    fs <<       "<td>"<<inodo->i_gid<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>i_size</td>\n";
    fs <<       "<td>"<<inodo->i_size<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>i_atime</td>\n";
    fs <<       "<td>"<<atime<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>i_ctime</td>\n";
    fs <<       "<td>"<<ctime<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>i_mtime</td>\n";
    fs <<       "<td>"<<mtime<<"</td>\n";
    fs <<   "</tr>\n";
    for(int i = 0; i < 15; i++){
        fs <<   "<tr>\n";
        fs <<       "<td>i_block_"<<i<<"</td>\n";
        fs <<       "<td port=\""<<i<<"\">"<<inodo->i_block[i]<<"</td>\n";
        fs <<   "</tr>\n";
    }
    fs <<   "<tr>\n";
    fs <<       "<td>i_type</td>\n";
    fs <<       "<td>"<<inodo->i_type+0<<"</td>\n";
    fs <<   "</tr>\n";
    fs <<   "<tr>\n";
    fs <<       "<td>i_perm</td>\n";
    fs <<       "<td>"<<inodo->i_perm<<"</td>\n";
    fs <<   "</tr>\n";
    fs << "</table>\n";
    fs << ">];\n";

    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            fs << "inodo"<<indice_inodo<<":"<<i<<"->bloque"<<inodo->i_block[i]<<"\n";
            if(i == 0){
                visitar_Bloque_Carpeta_Archivo(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, fs, dot_group + 1);
            } else if(i < 12){
                visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, fs, dot_group + 1);
            } else if(i == 12){
                visitar_Puntero_Simple(inodo->i_block[i], inodo->i_type, superBloque, archivo, fs, dot_group + 1);
            } else if(i == 13){
                visitar_Puntero_Doble(inodo->i_block[i], inodo->i_type, superBloque, archivo, fs, dot_group + 1);
            } else if(i == 14){
                visitar_Puntero_Triple(inodo->i_block[i], inodo->i_type, superBloque, archivo, fs, dot_group + 1);
            }
        }
    }
}

void visitar_Bloque_Carpeta_Archivo(Condicion condicion, int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group){
    switch(i_type){
    case 1: //Archivo
        {
            struct fileBlock *file = new struct fileBlock;
            fseek(archivo, superBloque->s_block_start + indice * sizeof(fileBlock),SEEK_SET);
            fread(file,sizeof(fileBlock),1,archivo);

            fs << "\nbloque"<<indice<<" [group="<<dot_group<<" shape=plaintext color=\"yellow\" style=\"filled\" label=<\n";
            fs << "<table color=\"black\">\n";
            fs << "<tr>\n";
            fs << "<td>Bloque "<<indice<<"</td>\n";
            fs << "</tr>\n";
            fs << "<tr>\n";
            fs << "<td>";
            for(size_t i = 0; i < 64; i++){
                if(file->b_content[i] == '\0')
                    break;
                if(file->b_content[i] == '\n')
                    fs << "\\n";
                else if(file->b_content[i] == '\t')
                    fs << "\\t";
                else
                    fs << file->b_content[i];
            }
            fs << "</td>\n";
            fs << "</tr>\n";
            fs << "</table>\n";
            fs << ">];\n";
        }
        break;
    case 0: //Carpeta
        {
            struct folderBlock *folder = new struct folderBlock;
            fseek(archivo, superBloque->s_block_start + indice * sizeof(folderBlock),SEEK_SET );
            fread(folder,sizeof(folderBlock),1,archivo);

            fs << "\nbloque"<<indice<<" [group="<<dot_group<<" shape=plaintext color=\"tomato\" style=\"filled\" label=<\n";
            fs << "<table color=\"black\">\n";
            fs << "<tr>\n";
            fs << "<td colspan=\"2\">Bloque "<<indice<<"</td>\n";
            fs << "</tr>\n";

            for(int i = 0; i < 4; i++){
                if(folder->b_content[i].b_inode == -1){
                    fs << "<tr>\n";
                    fs << "<td> </td>\n";
                    fs << "<td>-1</td>\n";
                    fs << "</tr>\n";
                } else {
                    fs << "<tr>\n";
                    fs << "<td>"<<folder->b_content[i].b_name<<"</td>\n";
                    fs << "<td port=\""<<i<<"\">"<<folder->b_content[i].b_inode<<"</td>\n";
                    fs << "</tr>\n";
                }

            }
            fs << "</table>\n";
            fs << ">];\n";

            if(condicion == NO_IGNORAR_PRIMEROS_REGISTROS){
                for(int i = 0; i < 4; i++){
                    if(folder->b_content[i].b_inode == -1){
                        continue;
                    }
                    fs << "bloque"<<indice<<":"<<i<<"->inodo"<<folder->b_content[i].b_inode<<"\n";
                    visitar_Inodo(folder->b_content[i].b_inode, superBloque, archivo, fs, dot_group + 1);
                }
            }

            if(condicion == IGNORAR_PRIMEROS_REGISTROS){
                for(int i = 2; i < 4; i++){
                    if(folder->b_content[i].b_inode == -1){
                        continue;
                    }
                    fs << "bloque"<<indice<<":"<<i<<"->inodo"<<folder->b_content[i].b_inode<<"\n";
                    visitar_Inodo(folder->b_content[i].b_inode, superBloque, archivo, fs, dot_group + 1);
                }
            }
        }
        break;
    }


}

void visitar_Puntero_Simple(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    fs << "\nbloque"<<indice<<" [group="<<dot_group<<" shape=plaintext color=\"chartreuse3\" style=\"filled\" label=<\n";
    fs << "<table color=\"black\">\n";
    fs << "<tr>\n";
    fs << "<td>Bloque "<<indice<<"</td>\n";
    fs << "</tr>\n";

    for(int i = 0; i < 16; i++){
        fs << "<tr>\n";
        fs << "<td port=\""<<i<<"\">"<<bloquePunteros->b_pointers[i]<<"</td>\n";
        fs << "</tr>\n";

    }
    fs << "</table>\n";
    fs << ">];\n";

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        fs << "bloque"<<indice<<":"<<i<<"->bloque"<<bloquePunteros->b_pointers[i]<<"\n";
        visitar_Bloque_Carpeta_Archivo(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],i_type,superBloque,archivo,fs, dot_group + 1);
    }
}

void visitar_Puntero_Doble(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    fs << "\nbloque"<<indice<<" [group="<<dot_group<<" shape=plaintext color=\"chartreuse3\" style=\"filled\" label=<\n";
    fs << "<table color=\"black\">\n";
    fs << "<tr>\n";
    fs << "<td>Bloque "<<indice<<"</td>\n";
    fs << "</tr>\n";

    for(int i = 0; i < 16; i++){
        fs << "<tr>\n";
        fs << "<td port=\""<<i<<"\">"<<bloquePunteros->b_pointers[i]<<"</td>\n";
        fs << "</tr>\n";

    }
    fs << "</table>\n";
    fs << ">];\n";

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        fs << "bloque"<<indice<<":"<<i<<"->bloque"<<bloquePunteros->b_pointers[i]<<"\n";
        visitar_Puntero_Simple(bloquePunteros->b_pointers[i],i_type,superBloque,archivo,fs, dot_group + 1);
    }
}

void visitar_Puntero_Triple(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs, int dot_group){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    fs << "\nbloque"<<indice<<" [group="<<dot_group<<" shape=plaintext color=\"chartreuse3\" style=\"filled\" label=<\n";
    fs << "<table color=\"black\">\n";
    fs << "<tr>\n";
    fs << "<td>Bloque "<<indice<<"</td>\n";
    fs << "</tr>\n";

    for(int i = 0; i < 16; i++){
        fs << "<tr>\n";
        fs << "<td port=\""<<i<<"\">"<<bloquePunteros->b_pointers[i]<<"</td>\n";
        fs << "</tr>\n";

    }
    fs << "</table>\n";
    fs << ">];\n";

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        fs << "bloque"<<indice<<":"<<i<<"->bloque"<<bloquePunteros->b_pointers[i]<<"\n";
        visitar_Puntero_Doble(bloquePunteros->b_pointers[i],i_type,superBloque,archivo,fs, dot_group + 1);
    }
}

int graficarBlock(std::map<TipoParametro, struct parametro> parametros, ListaMontaje *listaMontaje){

    //Obtener el superBloque
    struct partition particion = listaMontaje->getPrimaryMountPartition(parametros[T_ID].valor);
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    struct superBlock *superBloque = new struct superBlock;
    int bloqueAnterior = -1;
    int bloqueActual = 0;
    char block_state;

    char *disk_location = listaMontaje->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb");
    fseek(archivo,particion.part_start,SEEK_SET);
    fread(superBloque,sizeof(superBlock),1,archivo);

    std::ofstream fs("dot.txt");
    fs << "digraph G {\n";
    fs << "rankdir=LR;\n";

    visitar_Inodo_BLOCK(0,superBloque,archivo,fs);

    //Obtener los bloques
    while(bloqueActual < superBloque->s_blocks_count ){
        fseek(archivo,superBloque->s_bm_block_start + bloqueActual*sizeof(char),SEEK_SET);
        fread(&block_state,sizeof(char),1,archivo);
        if(block_state){

            if(bloqueAnterior == -1){
                bloqueAnterior = bloqueActual;
            } else {
                fs << "bloque"<<bloqueAnterior<<"->bloque"<<bloqueActual<<";\n";
                bloqueAnterior = bloqueActual;
            }


        }
        bloqueActual++;
    }

    fs << "}";
    fs.close();
    fclose(archivo);

    if(verificarSubCadena(parametros[T_PATH].valor, ".pdf")){
        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpdf dot.txt -o ");
        strcat(instruccion,parametros[T_PATH].valor);
        system(instruccion);
    } else {
        char path[100]; path[0] = '\0';
        strcat(path, parametros[T_PATH].valor);
        strcat(path, ".pdf");
        char instruccion[50];
        instruccion[0] = '\0';
        strcat(instruccion,"dot -Tpdf dot.txt -o ");
        strcat(instruccion,path);
        system(instruccion);
    }
}

void visitar_Inodo_BLOCK(int indice_inodo, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs){

    struct inodeTable *inodo = new struct inodeTable;
    fseek(archivo,superBloque->s_inode_start + indice_inodo * sizeof(inodeTable),SEEK_SET);
    fread(inodo,sizeof(inodeTable),1,archivo);

    for(int i = 0; i < 15; i++) {
        if(inodo->i_block[i] == -1){
            continue;
        } else {
            if(i == 0){
                visitar_Bloque_Carpeta_Archivo_BLOCK(IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, fs);
            } else if(i < 12){
                visitar_Bloque_Carpeta_Archivo_BLOCK(NO_IGNORAR_PRIMEROS_REGISTROS, inodo->i_block[i], inodo->i_type, superBloque, archivo, fs);
            } else if(i == 12){
                visitar_Puntero_Simple_BLOCK(inodo->i_block[i], inodo->i_type, superBloque, archivo, fs);
            } else if(i == 13){
                visitar_Puntero_Doble_BLOCK(inodo->i_block[i], inodo->i_type, superBloque, archivo, fs);
            } else if(i == 14){
                visitar_Puntero_Triple_BLOCK(inodo->i_block[i], inodo->i_type, superBloque, archivo, fs);
            }
        }
    }
}

void visitar_Bloque_Carpeta_Archivo_BLOCK(Condicion condicion, int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs){
    switch(i_type){
    case 1: //Archivo
        {
            struct fileBlock *file = new struct fileBlock;
            fseek(archivo, superBloque->s_block_start + indice * sizeof(fileBlock),SEEK_SET);
            fread(file,sizeof(fileBlock),1,archivo);

            fs << "\nbloque"<<indice<<" [shape=plaintext color=\"yellow\" style=\"filled\" label=<\n";
            fs << "<table color=\"black\" border=\"0\">\n";
            fs << "<tr>\n";
            fs << "<td>Bloque Archivo "<<indice<<"</td>\n";
            fs << "</tr>\n";
            fs << "<tr>\n";
            fs << "<td>";
            for(size_t i = 0; i < 64; i++){
                if(file->b_content[i] == '\0')
                    break;
                if(i % 16 == 0 && i != 0)
                    fs << '\n';
                if(file->b_content[i] == '\n')
                    fs << "\\n";
                else if(file->b_content[i] == '\t')
                    fs << "\\t";
                else
                    fs << file->b_content[i];
            }
            fs << "</td>\n";
            fs << "</tr>\n";
            fs << "</table>\n";
            fs << ">];\n";
        }
        break;
    case 0: //Carpeta
        {
            struct folderBlock *folder = new struct folderBlock;
            fseek(archivo, superBloque->s_block_start + indice * sizeof(folderBlock),SEEK_SET );
            fread(folder,sizeof(folderBlock),1,archivo);

            fs << "\nbloque"<<indice<<" [shape=plaintext color=\"tomato\" style=\"filled\" label=<\n";
            fs << "<table color=\"black\" border=\"0\">\n";
            fs << "<tr>\n";
            fs << "<td colspan=\"2\">Bloque Carpeta "<<indice<<"</td>\n";
            fs << "</tr>\n";
            fs << "<tr>\n";
            fs << "<td>b_name</td>\n";
            fs << "<td>b_inode</td>\n";
            fs << "</tr>\n";

            for(int i = 0; i < 4; i++){
                if(folder->b_content[i].b_inode == -1){
                    fs << "<tr>\n";
                    fs << "<td></td>\n";
                    fs << "<td>-1</td>\n";
                    fs << "</tr>\n";
                } else {
                    fs << "<tr>\n";
                    fs << "<td>"<<folder->b_content[i].b_name<<"</td>\n";
                    fs << "<td>"<<folder->b_content[i].b_inode<<"</td>\n";
                    fs << "</tr>\n";
                }

            }
            fs << "</table>\n";
            fs << ">];\n";

            if(condicion == NO_IGNORAR_PRIMEROS_REGISTROS){
                for(int i = 0; i < 4; i++){
                    if(folder->b_content[i].b_inode == -1){
                        continue;
                    }
                    visitar_Inodo_BLOCK(folder->b_content[i].b_inode, superBloque, archivo, fs);
                }
            }

            if(condicion == IGNORAR_PRIMEROS_REGISTROS){
                for(int i = 2; i < 4; i++){
                    if(folder->b_content[i].b_inode == -1){
                        continue;
                    }
                    visitar_Inodo_BLOCK(folder->b_content[i].b_inode, superBloque, archivo, fs);
                }
            }
        }
        break;
    }



}

void visitar_Puntero_Simple_BLOCK(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    fs << "\nbloque"<<indice<<" [shape=plaintext color=\"chartreuse3\" style=\"filled\" label=<\n";
    fs << "<table color=\"black\" border=\"0\">\n";
    fs << "<tr>\n";
    fs << "<td>Bloque Apuntadores "<<indice<<"</td>\n";
    fs << "</tr>\n";
    fs << "<tr>\n";
    fs << "<td>\n";
    fs << bloquePunteros->b_pointers[0]<<"\n";

    for(int i = 1; i < 16; i++){
        fs << ", "<<bloquePunteros->b_pointers[i]<<"\n";
    }
    fs << "</td>\n";
    fs << "</tr>\n";
    fs << "</table>\n";
    fs << ">];\n";

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        visitar_Bloque_Carpeta_Archivo_BLOCK(NO_IGNORAR_PRIMEROS_REGISTROS,bloquePunteros->b_pointers[i],i_type,superBloque,archivo,fs);
    }
}

void visitar_Puntero_Doble_BLOCK(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    fs << "\nbloque"<<indice<<" [shape=plaintext color=\"chartreuse3\" style=\"filled\" label=<\n";
    fs << "<table color=\"black\" border=\"0\">\n";
    fs << "<tr>\n";
    fs << "<td>Bloque Apuntadores "<<indice<<"</td>\n";
    fs << "</tr>\n";
    fs << "<tr>\n";
    fs << "<td>\n";
    fs << bloquePunteros->b_pointers[0]<<"\n";

    for(int i = 1; i < 16; i++){
        fs << ", "<<bloquePunteros->b_pointers[i]<<"\n";
    }
    fs << "</td>\n";
    fs << "</tr>\n";
    fs << "</table>\n";
    fs << ">];\n";

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        visitar_Puntero_Simple_BLOCK(bloquePunteros->b_pointers[i],i_type,superBloque,archivo,fs);
    }
}

void visitar_Puntero_Triple_BLOCK(int indice, char i_type, struct superBlock *superBloque, FILE *archivo, std::ofstream &fs){

    struct pointersBlock *bloquePunteros = new struct pointersBlock;
    fseek(archivo, superBloque->s_block_start + indice * sizeof(pointersBlock),SEEK_SET);
    fread(bloquePunteros,sizeof(pointersBlock),1,archivo);

    fs << "\nbloque"<<indice<<" [shape=plaintext color=\"chartreuse3\" style=\"filled\" label=<\n";
    fs << "<table color=\"black\" border=\"0\">\n";
    fs << "<tr>\n";
    fs << "<td>Bloque Apuntadores "<<indice<<"</td>\n";
    fs << "</tr>\n";
    fs << "<tr>\n";
    fs << "<td>\n";
    fs << bloquePunteros->b_pointers[0]<<"\n";

    for(int i = 1; i < 16; i++){
        fs << ", "<<bloquePunteros->b_pointers[i]<<"\n";
    }
    fs << "</td>\n";
    fs << "</tr>\n";
    fs << "</table>\n";
    fs << ">];\n";

    for(int i = 0; i < 16; i++){
        if(bloquePunteros->b_pointers[i] == -1){
            continue;
        }
        visitar_Puntero_Doble_BLOCK(bloquePunteros->b_pointers[i],i_type,superBloque,archivo,fs);
    }
}



