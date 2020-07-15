#include "mkfs.h"

int ejecutarMkfs(struct comando comando, ListaMontaje *lista){

    std::map<TipoParametro, struct parametro> parametros = *(comando.parametros);

    //1. Obtener la particion montada
    struct partition particion = lista->getPrimaryMountPartition(parametros[T_ID].valor);

    //2. Validar que la particion se haya encontrado
    if(particion.part_status == 0){
        std::cout<<"No se ha encontrado una particion montada bajo el ID '"<<parametros[T_ID].valor<<"'"<<std::endl;
        std::cout<<"(No se estan validando particiones logicas)"<<std::endl;
        return 0;
    }

    //3. Verificar si el tamano de la particion es suficientemente grande para formatearla
    int size_min = sizeof(superBlock) + 1*sizeof(journal) + 1 + 3 + 1*sizeof(inodeTable) + 3*sizeof(fileBlock);
    if(particion.part_size < size_min){
        std::cout<<"La particion no cuenta con el espacio minimo suficiente para ser formateada"<<std::endl;
        std::cout<<"Tamano particion: "<<particion.part_size<<" bytes"<<std::endl;
        std::cout<<"Tamano minimo necesario: "<<size_min<<" bytes"<<std::endl;
        return 0;
    }

    //4. Calcular el valor de "n"
    int n = (particion.part_size - sizeof(superBlock))/(1 + sizeof(journal) + 3 + sizeof(inodeTable) + 3*sizeof(fileBlock));

    //5. Obtener el super bloque, journal, inodo y folderblock (inicial)
    struct superBlock *superBloque = Getters::getNewSuperBlock(n,particion.part_start);
    struct journal *journaling = Getters::getNewJournal();
    struct inodeTable *inodoRaiz = Getters::getNewFolderInode(1,1,0);
    struct folderBlock *carpetaRaiz = Getters::getNewFolderBlock(0,0);

    //6. Enlazar estructuras
    superBloque->s_first_blo++;
    superBloque->s_first_ino++;
    superBloque->s_free_blocks_count--;
    superBloque->s_free_inodes_count--;

    //7. Setear la estructura inicial EXT3 en la particion
    char cero = 0;
    char uno = 1;
    char *disk_location = lista->getDiskLocation(parametros[T_ID].valor);
    FILE *archivo = fopen(disk_location,"rb+");
    fseek(archivo,particion.part_start,SEEK_SET);

    fwrite(superBloque,sizeof(superBlock),1,archivo);

    for(int i = 0; i < n;i++){
        fwrite(journaling,sizeof(journal),1,archivo);
    }

    fwrite(&uno,sizeof(char),1,archivo);
    for(int i = 0; i < n-1;i++){
        fwrite(&cero,sizeof(char),1,archivo);
    }

    fwrite(&uno,sizeof(char),1,archivo);
    for(int i = 0; i < 3*n-1; i++){
        fwrite(&cero,sizeof(char),1,archivo);
    }


    fwrite(inodoRaiz,sizeof(inodeTable),1,archivo);
    fseek(archivo, superBloque->s_block_start, SEEK_SET);
    fwrite(carpetaRaiz,sizeof(folderBlock),1,archivo);
    fclose(archivo);

    //8. Crear el archivo users.txt
    crearArchivoUsers(parametros[T_ID].valor, lista);

    return 1;
}

void crearArchivoUsers(char* idParticion, ListaMontaje *lista){

    FILE *archivo = fopen("/home/eddy/users.txt","w+");
    fclose(archivo);

    std::ofstream fs("/home/eddy/users.txt");
    fs << "1,G,root\n";
    fs << "1,U,root,root,123\n";
    fs.close();

    struct parametro *p_path = new struct parametro;
    strcpy(p_path->valor, "/users.txt");

    struct parametro *p_cont = new struct parametro;
    strcpy(p_cont->valor, "/home/eddy/users.txt");

    std::map<TipoParametro, struct parametro> parametros_mkfile;
    parametros_mkfile[T_PATH] = *p_path;
    parametros_mkfile[T_CONT] = *p_cont;

    struct comando nuevoComando;
    nuevoComando.tipo = T_MKFILE;
    nuevoComando.parametros = &parametros_mkfile;

    struct user usuario;
    strcpy(usuario.partition_id, idParticion);
    strcpy(usuario.usr,"root");
    strcpy(usuario.grp,"root");
    strcpy(usuario.pwd,"123");
    usuario.gid = 1;
    usuario.uid = 1;
    usuario.active = 1;

    Mkfile::ejecutarMkfile(nuevoComando,lista, &usuario);
}

