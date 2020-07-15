                        /********************** 
                         * Declaraciones en C *
                         **********************/
%{

#include <list>
#include <iterator>
#include <map>
#include "comando.h"
#include <iostream>
#include <stdlib.h>
#include <math.h>
#include "string.h"

extern int yylex(void);
extern char *yytext;
extern FILE *yyin;

extern int linea;
extern int columna;


void yyerror(char *s) {
	std::cout<<"ERROR SINTACTICO: No se esperaba '"<<yytext<<"'. Fila: "<<linea<<" Columna: "<<columna<<std::endl;
}

struct comando comando;


%}

                        /*************************
                          Declaraciones de Bison *
                         *************************/


%union
{
    char TEXT[256];
    struct parametro PARAM;
    struct comando COM;
    std::map<TipoParametro, struct parametro>* PARAMS;
}

%code requires {
    /*Este fragmento de código aparecerá en el parser.h.... y por alguna razón, también en el parser.cpp*/
    
    #include <map>
    #include "comando.h"
    #include <iostream>
    #include <math.h>
    #include "string.h"
}

%token <TEXT> guion
%token <TEXT> igual
%token <TEXT> comentario

%token <TEXT> mkdisk
%token <TEXT> rmdisk
%token <TEXT> fdisk
%token <TEXT> mount
%token <TEXT> unmount
%token <TEXT> exec
%token <TEXT> rep

%token <TEXT> size
%token <TEXT> fit
%token <TEXT> unit
%token <TEXT> path
%token <TEXT> type
%token <TEXT> deleteParam
%token <TEXT> name
%token <TEXT> add
%token <TEXT> r_id

%token <TEXT> mkfs
%token <TEXT> login
%token <TEXT> logout
%token <TEXT> mkgrp
%token <TEXT> rmgrp
%token <TEXT> mkusr
%token <TEXT> rmusr
%token <TEXT> r_chmod
%token <TEXT> mkfile
%token <TEXT> cat
%token <TEXT> rem
%token <TEXT> edit
%token <TEXT> ren
%token <TEXT> r_mkdir
%token <TEXT> cp
%token <TEXT> mv
%token <TEXT> find
%token <TEXT> r_chown
%token <TEXT> chgrp
%token <TEXT> r_pause

%token <TEXT> usr
%token <TEXT> pwd
%token <TEXT> grp
%token <TEXT> ugo
%token <TEXT> r
%token <TEXT> p
%token <TEXT> cont
%token <TEXT> file
%token <TEXT> dest
%token <TEXT> p_ruta

%token <TEXT> ruta
%token <TEXT> valor
%token <TEXT> rutaComilla
%token <TEXT> valorComilla

%type <COM> COMANDO
%type <PARAM> PARAMETRO
%type <PARAMS> PARAMETROS

%start INICIO;


%%
                /***********************
                * T_Reglas Gramaticales *
                ***********************/
INICIO:
    INICIO COMANDO
    { }
|   COMANDO
    {
        comando = $1;
    }
;


COMANDO:
    comentario
    {
        $$.tipo = T_COMENTARIO;
        strcpy($$.comentario, $1);
    }
|   mkdisk PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_MKDISK;
    }
|   rmdisk PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_RMDISK;
    }
|   fdisk PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_FDISK;
    }
|   mount PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_MOUNT;
    }
|   unmount PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_UNMOUNT;
    }
|   exec PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_EXEC;
    }
|   rep PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_REP;
    }
|   mkfs PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_MKFS;
    }
|   login PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_LOGIN;
    }
|   logout
    {
        $$.tipo = T_LOGOUT;
    }
|   mkgrp PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_MKGRP;
    }
|   rmgrp PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_RMGRP;
    }
|   mkusr PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_MKUSR;
    }
|   rmusr PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_RMUSR;
    }
|   r_chmod PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_CHMOD;
    }
|   mkfile PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_MKFILE;
    }
|   cat PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_CAT;
    }
|   rem PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_REM;
    }
|   edit PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_EDIT;
    }
|   ren PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_REN;
    }
|   r_mkdir PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_MKDIR;
    }
|   cp PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_CP;
    }
|   mv PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_MV;
    }
|   find PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_FIND;
    }
|   r_chown PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_CHOWN;
    }
|   chgrp PARAMETROS
    {
        $$.parametros = $2;
        $$.tipo = T_CHGRP;
    }
|   r_pause
    {
        $$.tipo = T_PAUSE;
    }
;

PARAMETROS:
    PARAMETROS PARAMETRO
    {
        
        switch($2.tipo){
            case T_SIZE:
                (*$1)[T_SIZE]=$2;
                break;
            case T_FIT:
                (*$1)[T_FIT]=$2;
                break;
            case T_UNIT:
                (*$1)[T_UNIT]=$2;
                break;
            case T_PATH:
                (*$1)[T_PATH]=$2;
                break;
            case T_TYPE:
                (*$1)[T_TYPE]=$2;
                break;
            case T_DELETE:
                (*$1)[T_DELETE]=$2;
                break;
            case T_NAME:
                (*$1)[T_NAME]=$2;
                break;
            case T_ADD:
                (*$1)[T_ADD]=$2;
                break;
            case T_ID:
                (*$1)[T_ID]=$2;
                break;
            case T_USR:
                (*$1)[T_USR]=$2;
                break;
            case T_PWD:
                (*$1)[T_PWD]=$2;
                break;
            case T_GRP:
                (*$1)[T_GRP]=$2;
                break;
            case T_UGO:
                (*$1)[T_UGO]=$2;
                break;
            case T_R:
                (*$1)[T_R]=$2;
                break;
            case T_P:
                (*$1)[T_P]=$2;
                break;
            case T_CONT:
                (*$1)[T_CONT]=$2;
                break;
            case T_FILE:
                (*$1)[T_FILE]=$2;
                break;
            case T_DEST:
                (*$1)[T_DEST]=$2;
                break;
            case T_RUTA:
                (*$$)[T_RUTA]=$2;
                break;
        }
        $$ = $1;
    }
|   PARAMETRO
    {
        $$ = new std::map<TipoParametro, struct parametro>;
        switch($1.tipo){
            case T_SIZE:
                (*$$)[T_SIZE]=$1;
                break;
            case T_FIT:
                (*$$)[T_FIT]=$1;
                break;
            case T_UNIT:
                (*$$)[T_UNIT]=$1;
                break;
            case T_PATH:
                (*$$)[T_PATH]=$1;
                break;
            case T_TYPE:
                (*$$)[T_TYPE]=$1;
                break;
            case T_DELETE:
                (*$$)[T_DELETE]=$1;
                break;
            case T_NAME:
                (*$$)[T_NAME]=$1;
                break;
            case T_ADD:
                (*$$)[T_ADD]=$1;
                break;
            case T_ID:
                (*$$)[T_ID]=$1;
                break;
            case T_USR:
                (*$$)[T_USR]=$1;
                break;
            case T_PWD:
                (*$$)[T_PWD]=$1;
                break;
            case T_GRP:
                (*$$)[T_GRP]=$1;
                break;
            case T_UGO:
                (*$$)[T_UGO]=$1;
                break;
            case T_R:
                (*$$)[T_R]=$1;
                break;
            case T_P:
                (*$$)[T_P]=$1;
                break;
            case T_CONT:
                (*$$)[T_CONT]=$1;
                break;
            case T_FILE:
                (*$$)[T_FILE]=$1;
                break;
            case T_DEST:
                (*$$)[T_DEST]=$1;
                break;
            case T_RUTA:
                (*$$)[T_RUTA]=$1;
                break;
        }
    }
;

PARAMETRO:
    size igual valor
    {
        $$.tipo = T_SIZE;
        strcpy($$.valor,$3);
    }
|   fit igual valor
    {
        $$.tipo = T_FIT;
        strcpy($$.valor,$3);
    }
|   unit igual valor
    {
        $$.tipo = T_UNIT;
        strcpy($$.valor,$3);
    }
|   type igual valor
    {
        $$.tipo = T_TYPE;
        strcpy($$.valor,$3);
    }
|   deleteParam igual valor
    {
        $$.tipo = T_DELETE;
        strcpy($$.valor,$3);
    }
|   name igual valor
    {
        $$.tipo = T_NAME;
        strcpy($$.valor,$3);
    }
|   name igual valorComilla
    {
        $$.tipo = T_NAME;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   add igual valor
    {
        $$.tipo = T_ADD;
        strcpy($$.valor,$3);
    }
|   add igual guion valor
    {
        $$.tipo = T_ADD;
        strcpy($$.valor,$3);
        strcat($$.valor,$4);
    }
|   r_id igual valor
    {
        $$.tipo = T_ID;
        strcpy($$.valor,$3);
    }
|   p_ruta igual ruta
    {
        $$.tipo = T_RUTA;
        strcpy($$.valor,$3);
    }
|   p_ruta igual rutaComilla
    {
        $$.tipo = T_RUTA;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   path igual ruta
    {
        $$.tipo = T_PATH;
        strcpy($$.valor,$3);
    }
|   path igual rutaComilla
    {
        $$.tipo = T_PATH;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   usr igual valor
    {
        $$.tipo = T_USR;
        strcpy($$.valor,$3);
    }
|   usr igual valorComilla
    {
        $$.tipo = T_USR;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   pwd igual valor
    {
        $$.tipo = T_PWD;
        strcpy($$.valor,$3);
    }
|   pwd igual valorComilla
    {
        $$.tipo = T_PWD;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   grp igual valor
    {
        $$.tipo = T_GRP;
        strcpy($$.valor,$3);
    }
|   grp igual valorComilla
    {
        $$.tipo = T_GRP;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   ugo igual valor
    {
        $$.tipo = T_UGO;
        strcpy($$.valor,$3);
    }
|   r
    {
        $$.tipo = T_R;
        strcpy($$.valor,"1");
    }
|   p
    {
        $$.tipo = T_P;
        strcpy($$.valor,"1");
    }
|   cont igual valor
    {
        $$.tipo = T_CONT;
        strcpy($$.valor,$3);
    }
|   cont igual valorComilla
    {
        $$.tipo = T_CONT;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   cont igual ruta
    {
        $$.tipo = T_CONT;
        strcpy($$.valor,$3);
    }
|   cont igual rutaComilla
    {
        $$.tipo = T_CONT;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   file igual ruta
    {
        $$.tipo = T_FILE;
        strcpy($$.valor,$3);
    }
|   file igual rutaComilla
    {
        $$.tipo = T_FILE;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
|   dest igual ruta
    {
        $$.tipo = T_DEST;
        strcpy($$.valor,$3);
    }
|   dest igual rutaComilla
    {
        $$.tipo = T_DEST;
        char* aux = $3;
        aux++;
        aux[strlen(aux)-1]='\0';
        strcpy($$.valor,aux);
    }
;


%%
/**********************
 * Codigo C Adicional *
 **********************/

