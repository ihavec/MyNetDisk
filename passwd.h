#ifndef __PASSWD_H__
#define __PASSWD_H__

#include "factory.h"
#include <crypt.h>

int sign_up(pfactory_t, message_t *);
int sign_in(pfactory_t, message_t *);

#endif
