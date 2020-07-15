#include <iostream>
#include "Analizador/parser.h"
#include "Analizador/scanner.h"
#include "Comandos/exec.h"
#include "Estructuras/user.h"

using namespace std;
extern struct comando comando;


int main()
{

    ListaMontaje *listaMotaje = new ListaMontaje();
    struct user *usuario = new struct user;
    usuario->active = 0;

    char entrada[150];
    std::cout<<"Sistema de archivos: V1.0"<<std::endl;
    std::cout<<"Eddy Arnoldo Reyes Hernandez"<<std::endl;
    std::cout<<"201612326"<<std::endl;

    while(true){
        std::cout<<"==============================================================================="<<std::endl;
        cin.getline(entrada,150,'\n');
        char saltoLinea[2];
        saltoLinea[0] = '\n';
        saltoLinea[1] = '\0';
        strcat(entrada, saltoLinea);

        if(parsearArchivo(entrada) != 0){
            for(int i = 0; i < 20; i++){
                parsearArchivo("#CHAPUZ PARA LIMPIAR LA ENTRADA DEL PARSER\n");
            }
            std::cout<<"No se pudo parsear el comando"<<std::endl;
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
            continue;
        }

        //1.5 Validar si es comentario
        if(comando.tipo == T_COMENTARIO){
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
            continue;
        }

        //2. Validar parametros del comando
        if(!validar(comando)){
            std::cout<<"Parámetros no válidos"<<std::endl;
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
            continue;
        }

        //3. Ejecutar comando
        if(ejecutar(comando, listaMotaje, usuario)){
            std::cout<<"Finalizado correctamente"<<std::endl;
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
        } else {
            std::cout<<"Finalizado con errores"<<std::endl;
            std::cout<<"==============================================================================="<<std::endl;
            std::cout<<std::endl;
            std::cout<<std::endl;
        }
    }
    return 0;
}
