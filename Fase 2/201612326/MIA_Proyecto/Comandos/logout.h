#ifndef LOGOUT_H
#define LOGOUT_H

#include "Estructuras/user.h"
#include <iostream>

class Logout
{
private:
    Logout();
public:
    static int ejecutarLogout(struct user *usuario);

};

#endif // LOGOUT_H
