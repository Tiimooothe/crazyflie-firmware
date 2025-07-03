/**
 * @file puf.cpp
 * @brief Puf class implementation.
 * 
 * This file holds the puf class implementation.
 * 
 */

#include "puf.hpp"

/// @brief This function represent the coputation of the PUF. SHA256 is a one-way function used to simulate a PUF behaviour.
void sha256_raw(const unsigned char* data, size_t len, const unsigned char * salt, size_t saltSize, unsigned char* output) {
    hash_state md;
    sha256_init(&md);
    if (salt != NULL && saltSize > 0) {
        sha256_process(&md, salt, saltSize);
    }
    sha256_process(&md, data, len);
    sha256_done(&md, output);
}

/// @brief Constructor
puf::puf() : salt{} { 
    generate_random_bytes(const_cast<unsigned char*>(salt), PUF_SIZE);
}

puf::puf(unsigned char * salt) : salt{} { 
    memcpy(this->salt, salt, PUF_SIZE);
}

/// @brief Initiate the PUF computation.
/// @param input 
/// @param size 
/// @param output 
void puf::process(const unsigned char * input, size_t size, unsigned char * output) const{
    sha256_raw(input, size, this->salt, sizeof(this->salt), output);
}




