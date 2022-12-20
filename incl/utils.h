#ifndef _UTILS_H
#define _UTILS_H

// deps
#include <libirecovery.h>

irecv_client_t get_client();
int send_file(const char *filename);
int run_command(const char *command);

#define file_exists(file) ((access(file, F_OK)) != -1)

#endif /* _UTILS_H */