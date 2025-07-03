/**
 * @file puf.hpp
 * @brief Puf class header
 * 
 * This file holds the puf class header.
 * 
 */

#ifndef PUF_HPP
#define PUF_HPP

#include "utils.hpp"

/// @brief This class models a PUF.
class puf
{
private:
    unsigned char salt[PUF_SIZE];
public:
    puf();
    puf(unsigned char * salt);

    void process(const unsigned char * input, size_t size, unsigned char * output) const;
    
    // void printSalt() const{
    //     print_hex(salt,PUF_SIZE);
    // }

};

#endif