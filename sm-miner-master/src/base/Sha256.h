#ifndef SHA256_H
#define SHA256_H

#include <stdint.h>

void SHA256_Full(uint32_t *state, uint32_t *data, const uint32_t *st);
void SHA256_EncodeMessage(const uint8_t* message, uint32_t messageLen, uint8_t* hash);

#endif // SHA256_H
