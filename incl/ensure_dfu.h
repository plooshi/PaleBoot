#ifndef _ENSURE_DFU_H
#define _ENSURE_DFU_H

// std c library
#include <stdbool.h>

bool ensure_dfu(bool semi_tethered);
bool ensure_dfu_no_fix();

#endif /* _ENSURE_DFU_H */