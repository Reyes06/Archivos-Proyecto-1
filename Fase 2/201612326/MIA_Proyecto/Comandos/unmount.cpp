#include "unmount.h"

int ejecutarUnmount(struct comando comando, ListaMontaje *listaMontaje){
    std::map<TipoParametro, struct parametro> parametros (*comando.parametros);
    return listaMontaje->desmontarParticion(parametros[T_ID].valor);
}
