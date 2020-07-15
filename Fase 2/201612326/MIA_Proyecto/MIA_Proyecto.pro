TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        Analizador/parser.cpp \
        Analizador/scanner.cpp \
        Comandos/cat.cpp \
        Comandos/chgrp.cpp \
        Comandos/chmod.cpp \
        Comandos/chown.cpp \
        Comandos/cp.cpp \
        Comandos/edit.cpp \
        Comandos/exec.cpp \
        Comandos/fdisk.cpp \
        Comandos/file.cpp \
        Comandos/find.cpp \
        Comandos/login.cpp \
        Comandos/logout.cpp \
        Comandos/ls.cpp \
        Comandos/mkdir.cpp \
        Comandos/mkdisk.cpp \
        Comandos/mkfile.cpp \
        Comandos/mkfs.cpp \
        Comandos/mkgrp.cpp \
        Comandos/mkusr.cpp \
        Comandos/mount.cpp \
        Comandos/mv.cpp \
        Comandos/ren.cpp \
        Comandos/rep.cpp \
        Comandos/rmdisk.cpp \
        Comandos/rmgrp.cpp \
        Comandos/rmusr.cpp \
        Comandos/unmount.cpp \
        ListaDisponibles/listaDisponibles.cpp \
        ListaDisponibles/listaLogicoDisponibles.cpp \
        ListaMontaje/ListaMontaje.cpp \
        getters.cpp \
        main.cpp

HEADERS += \
    Analizador/comando.h \
    Analizador/parametro.h \
    Analizador/parser.h \
    Analizador/scanner.h \
    Comandos/cat.h \
    Comandos/chgrp.h \
    Comandos/chmod.h \
    Comandos/chown.h \
    Comandos/cp.h \
    Comandos/edit.h \
    Comandos/exec.h \
    Comandos/fdisk.h \
    Comandos/file.h \
    Comandos/find.h \
    Comandos/login.h \
    Comandos/logout.h \
    Comandos/ls.h \
    Comandos/mkdir.h \
    Comandos/mkdisk.h \
    Comandos/mkfile.h \
    Comandos/mkfs.h \
    Comandos/mkgrp.h \
    Comandos/mkusr.h \
    Comandos/mount.h \
    Comandos/mv.h \
    Comandos/ren.h \
    Comandos/rep.h \
    Comandos/rmdisk.h \
    Comandos/rmgrp.h \
    Comandos/rmusr.h \
    Comandos/unmount.h \
    Estructuras/ebr.h \
    Estructuras/fileBlock.h \
    Estructuras/folderBlock.h \
    Estructuras/inodeTable.h \
    Estructuras/journal.h \
    Estructuras/mbr.h \
    Estructuras/partition.h \
    Estructuras/pointersBlock.h \
    Estructuras/superBlock.h \
    Estructuras/user.h \
    Estructuras/validar.h \
    ListaDisponibles/listaDisponibles.h \
    ListaDisponibles/listaLogicoDisponibles.h \
    ListaDisponibles/nodoDisponible.h \
    ListaDisponibles/nodoLogicoDisponible.h \
    ListaMontaje/ListaMontaje.h \
    ListaMontaje/nodoDisco.h \
    ListaMontaje/nodoParticion.h \
    getters.h
