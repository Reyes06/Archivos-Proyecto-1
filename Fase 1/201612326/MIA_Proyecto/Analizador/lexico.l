                /*****************
                 * Declaraciones *
                 ****************/ 

%{

#include "parser.h"
#include <string.h>
#include <iostream>
	
int linea=1;
int columna=1;

%}

                /*****************
                 *Configuraciones*
                 ****************/ 

%option case-insensitive
%option noyywrap

                /********************
                 *Regular Expresions*
                 *******************/ 
comentario [#][^\n]+
valor [_a-zñÑ0-9]+
ruta ([/][_a-zñÑ0-9]+)+([.][_a-z0-9]+)?
valorComilla [\"][ _a-z0-9ñÑ]+[\"]
rutaComilla [\"]([/][ _a-z0-9]+)+([.][_a-z0-9]+)?[\"]

%%
                /*********************
                * Reglas y Acciones *
                **********************/

                        /******************************************************************
                                                    * Fase1 *
                        ******************************************************************/

"mkdisk" 	        { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(mkdisk);}
"rmdisk" 	        { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(rmdisk);}
"fdisk" 	        { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(fdisk);}
"mount" 	        { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(mount);}
"unmount" 	        { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(unmount);}
"exec" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(exec);}
"rep" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(rep);}

"-size" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(size);}
"-fit" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(fit);}
"-unit" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(unit);}
"-path" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(path);}
"-type" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(type);}
"-delete" 	        { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(deleteParam);}
"-name" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(name);}
"-add" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(add);}
"-id" 	            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(r_id);}

                        /******************************************************************
                                                    * Fase2 *
                        ******************************************************************/

"mkfs"              { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(mkfs);}
"login"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(login);}
"logout"            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(logout);}
"mkgrp"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(mkgrp);}
"rmgrp"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(rmgrp);}
"mkusr"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(mkusr);}
"rmusr"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(rmusr);}
"chmod"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(r_chmod);}
"mkfile"            { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(mkfile);}
"cat"               { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext); return(cat);}
"rem"               { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(rem);}
"edit"              { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(edit);}
"ren"               { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(ren);}
"mkdir"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(r_mkdir);}
"cp"                { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(cp);}
"mv"                { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(mv);}
"find"              { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(find);}
"chown"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(r_chown);}
"chgrp"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(chgrp);}
"pause"             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(r_pause);}
"loss"              { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(loss);}
"recovery"          { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(recovery);}

"-fs"                { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(fs);}
"-usr"               { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(usr);}
"-pwd"               { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(pwd);}
"-grp"               { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(grp);}
"-ugo"               { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(ugo);}
"-r"                 { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(r);}
"-p"                 { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(p);}
"-cont"              { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(cont);}
"-file"              { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(file);}
"-dest"              { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(dest);}

"-"                 { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(guion);}
"="                 { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(igual);}

{valor}             { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(valor);}
{ruta}              { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(ruta);}
{rutaComilla}       { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(rutaComilla);}
{valorComilla}      { columna = columna + strlen(yytext); strcpy(yylval.TEXT, yytext);  return(valorComilla);}
{comentario}        { columna = 1; linea++; strcpy(yylval.TEXT, yytext); return(comentario);}

[\n] 	            { columna = 1; linea++;}
[ \t\r\f]           { ;}
 


.	{
        std::cout<<"ERROR LEXICO: No se reconoce el simbolo '"<<yytext<<"'";
        std::cout<<"Fila: "<<linea<<" Columna: "<<columna<<std::endl;
    }


%%
