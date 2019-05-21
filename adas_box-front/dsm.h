#ifndef DSM_H
#define DSM_H

#include "data.h"
/**
 * dsm initialize
 */

int dsm_init(struct Data* data);

/**
 * process dsm one frame
 */

int dsm_process(struct Data* data);

/**
 * dsm destroy
 */

int dsm_fini();

#endif
