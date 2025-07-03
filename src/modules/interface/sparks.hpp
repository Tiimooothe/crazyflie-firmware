#pragma once

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#include "tomcrypt_private.h"

#ifdef __cplusplus
extern "C" {
#endif

void sparksTaskInit(void);
bool sparksTaskTest(void);
void sparksTaskEnqueueInput(int value);

#ifdef __cplusplus
}
#endif

void sparksTaskInitCPP();
bool sparksTaskTestCPP();
void sparksTaskEnqueueInputCPP(int value);


