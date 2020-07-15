TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
        Analizador/parser.cpp \
        Analizador/scanner.cpp \
        Comandos/exec.cpp \
        Comandos/fdisk.cpp \
        Comandos/mkdisk.cpp \
        Comandos/mkfs.cpp \
        Comandos/mount.cpp \
        Comandos/rep.cpp \
        Comandos/rmdisk.cpp \
        Comandos/unmount.cpp \
        ListaDisponibles/listaDisponibles.cpp \
        ListaDisponibles/listaLogicoDisponibles.cpp \
        ListaMontaje/ListaMontaje.cpp \
        main.cpp

HEADERS += \
    Analizador/comando.h \
    Analizador/parametro.h \
    Analizador/parser.h \
    Analizador/scanner.h \
    Comandos/exec.h \
    Comandos/fdisk.h \
    Comandos/mkdisk.h \
    Comandos/mkfs.h \
    Comandos/mount.h \
    Comandos/rep.h \
    Comandos/rmdisk.h \
    Comandos/unmount.h \
    Estructuras/ebr.h \
    Estructuras/fileBlock.h \
    Estructuras/folderBlock.h \
    Estructuras/inodeTable.h \
    Estructuras/mbr.h \
    Estructuras/partition.h \
    Estructuras/pointersBlock.h \
    Estructuras/superBlock.h \
    Estructuras/validar.h \
    ListaDisponibles/listaDisponibles.h \
    ListaDisponibles/listaLogicoDisponibles.h \
    ListaDisponibles/nodoDisponible.h \
    ListaDisponibles/nodoLogicoDisponible.h \
    ListaMontaje/ListaMontaje.h \
    ListaMontaje/nodoDisco.h \
    ListaMontaje/nodoParticion.h
