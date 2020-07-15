#ifndef LOGIN_H
#define LOGIN_H

#include <stdlib.h>
#include <iostream>
#include "Estructuras/user.h"
#include "Estructuras/superBlock.h"
#include "Estructuras/inodeTable.h"
#include "Estructuras/fileBlock.h"
#include "Estructuras/folderBlock.h"
#include "Estructuras/pointersBlock.h"
#include "ListaMontaje/ListaMontaje.h"
#include "Analizador/comando.h"

class Login
{
private:
    enum Opcion{
        VALIDAR_USUARIO,
        OBTENER_GID
    };
    Login();
    static int recorrer_Directo(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *buffer, char* user, char *pwd, struct user *usuario, Opcion dec);
    static int recorrer_Puntero_Simple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *buffer, char* user, char *pwd, struct user *usuario, Opcion dec);
    static int recorrer_Puntero_Doble(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *buffer, char* user, char *pwd, struct user *usuario, Opcion dec);
    static int recorrer_Puntero_Triple(int indice_bloque, struct superBlock *superBloque, FILE *archivo, char *buffer, char* user, char *pwd, struct user *usuario, Opcion dec);
    static int validarUsuario(char *buffer, char *usr, char *pwd, struct user *usuario);
    static int obtenerGid(char *buffer, struct user *usuario);
public:
    static int ejecutarLogin(struct comando comando, ListaMontaje *listaMontaje, struct user *usuario);
};

#endif // LOGIN_H
