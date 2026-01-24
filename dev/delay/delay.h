/* delay.h
 *
 * Header file for delay.c
 *
 * Author: Ryan McHenry
 * Created: January 23, 2026
 * Last Modified: January 23, 2026
 */

#ifndef DELAY_H
#define DELAY_H

#include "types/types.h"

void enqueue(DelayedCommand *, ThreadQueue *, int);
void *poll(void *);

#endif