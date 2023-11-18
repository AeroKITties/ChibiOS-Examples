#pragma once
#include "ch.h"
// Some functions for printing ower SWO (debug only)

void SWO_PrintChar(char const c, uint8_t const portNo);

void SWO_PrintString(char const* s, uint8_t const portNumber);
 
void printString(char const* str);
