#ifndef __COMMAND_H__
#define __COMMAND_H__

#include "factory.h"

void change_dir(pfactory_t, message_t *);
void make_dir(pfactory_t, message_t *);
void list_file(pfactory_t, message_t *);
void remove_file(pfactory_t, message_t *);
void remove_dir(pfactory_t, message_t *);
void log_out();
void quit();
void quit2();
void re_name(pfactory_t, message_t *);

#endif
