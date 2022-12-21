#ifndef _UTILS_H
#define _UTILS_H

// deps
#include <libirecovery.h>

// std c library
#include <stdio.h>

irecv_client_t get_client();
int send_file(const char *filename);
int run_command(const char *command);
int set_env(const char *key, const char *value);
void read_all(char **ptr, FILE *fp);

#define file_exists(file) ((access(file, F_OK)) != -1)

#endif /* _UTILS_H */