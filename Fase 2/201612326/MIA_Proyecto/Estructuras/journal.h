#ifndef JOURNALING_H
#define JOURNALING_H

#include <time.h>

struct journal {
    char active;
    struct tm hora;
    char user[12];
    char operation[256];
};

#endif // JOURNALING_H
