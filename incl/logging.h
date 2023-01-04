#ifndef _LOGGING_H
#define _LOGGING_H

#define _color_error "\033[0;91m"
#define _color_info "\033[0;92m"
#define _color_warn "\033[0;93m"
#define _color_debug "\033[0;94m"
#define _color_reset "\033[0m"

#define _log_base(str, color, level, ...) printf(color "[%s] %s:%d (%s) --> " str _color_reset, level, __FILE__, __LINE__, __func__, ##__VA_ARGS__)

#define log_debug(str, ...) _log_base(str, _color_debug, "Debug", ##__VA_ARGS__)
#define log_error(str, ...) _log_base(str, _color_error, "Error", ##__VA_ARGS__)
#define log_info(str, ...) _log_base(str, _color_info, "Info", ##__VA_ARGS__)
#define log_warn(str, ...) _log_base(str, _color_warn, "Warn", ##__VA_ARGS__)

#endif