#ifndef UTILS__HPP
#define UTILS__HPP

#include <string> 
#include <iostream>

#include <cstring>    // For std::memcpy
#include <cstdint>
#include <cstddef>

#include <unordered_map>

extern "C"
{
  #include "app.h"
 
  #include "FreeRTOS.h"
  #include "task.h"
  #include "cmp.h"
  #include "tomcrypt_private.h"

  #include "debug.h"
}

#define PUF_SIZE 8
#define CHALLENGE_SIZE 2

void utilsTest(void);
void generate_random_bytes(unsigned char* buffer, size_t size);
void print_hex(const unsigned char *buffer, size_t length);

void xor_buffers(const unsigned char* input1, const unsigned char* input2, size_t size, unsigned char* output);
hash_state* initHash();
void addToHash(hash_state* ctx, const unsigned char* data, size_t size);
void addToHash(hash_state* ctx, const std::string& str);
void calculateHash(hash_state* ctx, unsigned char * output);
void deriveKeyUsingHKDF(const unsigned char* NA, const unsigned char* NB, const unsigned char* S, size_t keyLength, unsigned char* derivedKey);
bool extractValueFromMap(std::unordered_map<std::string, std::string> map, std::string key , unsigned char * output, size_t size);
void warmup();

#endif