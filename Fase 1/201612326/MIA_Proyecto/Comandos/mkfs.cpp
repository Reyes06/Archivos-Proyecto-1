#include "mkfs.h"

int ejecutarMkfs(struct comando comando, ListaMontaje *lista){

    std::map<TipoParametro, struct parametro> parametros = *(comando.parametros);

    //1. Obtener la particion montada
    struct partition *particion = lista->getPartition(parametros[T_ID].valor);
    if(particion == nullptr){
        std::cout<<"No se ha encontrado ninguna partición montado bajo el nombre '"<<parametros[T_ID].valor<<"'"<<std::endl;
        return 0;
    }

    //2. Obtener el super bloque

    return 1;
}
