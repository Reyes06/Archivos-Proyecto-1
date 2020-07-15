#ifndef USER_H
#define USER_H

struct user {
    char active;
    char usr[15];
    char pwd[15];
    char grp[15];
    char partition_id[10];
    int gid;
    int uid;
};

#endif // USER_H
