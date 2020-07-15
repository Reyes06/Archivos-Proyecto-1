#include "logout.h"

Logout::Logout()
{

}

int Logout::ejecutarLogout(struct user *usuario){
    if(!usuario->active){
        std::cout<<"No hay ningun usuario logueado en el sistema"<<std::endl;
        return 0;
    }
    usuario->active = 0;
    return 1;
}
