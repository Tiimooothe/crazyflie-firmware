#include "utils.hpp"

void utilsTest(void){
    std::string hello = "Hello from utils !";
    DEBUG_PRINT("string : %s\n",hello.c_str());
    return ;
}

// Poor man's PRNG seed
static uint32_t lcg_seed = 8766;

uint32_t lcg_rand() {
    lcg_seed = 1664525 * lcg_seed + 1013904223;  // LCG parameters (Numerical Recipes)
    return lcg_seed;
}

void generate_random_bytes(unsigned char* buffer, size_t size) {
    size_t i = 0;
    while (i < size) {
        uint32_t rnd = lcg_rand();  // Get 4 random bytes at a time
        size_t to_copy = (size - i > 4) ? 4 : (size - i);
        std::memcpy(buffer + i, &rnd, to_copy);
        i += to_copy;
    }
}

void print_hex(const unsigned char *buffer, size_t length) {
    for (size_t i = 0; i < length; i++) {
        DEBUG_PRINT("%02x",(unsigned int) buffer[i]);
    }
    DEBUG_PRINT("\n");
}

void xor_buffers(const unsigned char* input1, const unsigned char* input2, size_t size, unsigned char* output) {
    if (output == input1 || output == input2) {
        // XOR into a temporary buffer to avoid interference
        unsigned char* temp_output = new unsigned char[size];
        
        for (size_t i = 0; i < size; ++i) {
            temp_output[i] = input1[i] ^ input2[i];
        }
        
        // Copy the result back to the original output buffer if needed
        if (output == input1) {
            for (size_t i = 0; i < size; ++i) {
                output[i] = temp_output[i];
            }
        } else {
            for (size_t i = 0; i < size; ++i) {
                output[i] = temp_output[i];
            }
        }

        delete[] temp_output;  // Clean up
    } else {
        // No interference, XOR directly into the output buffer
        for (size_t i = 0; i < size; ++i) {
            output[i] = input1[i] ^ input2[i];
        }
    }
}

 hash_state* initHash(){
    hash_state* ctx = new hash_state();
    sha256_init(ctx);
    return ctx;
}

void addToHash(hash_state* ctx, const unsigned char* data, size_t size){
    sha256_process(ctx, data, size);
}

void addToHash(hash_state* ctx, const std::string& str){
    sha256_process(ctx, reinterpret_cast<const unsigned char*>(str.data()), str.size());
}

void calculateHash(hash_state* ctx, unsigned char * output){
    unsigned char fullHash[32];
    sha256_done(ctx, fullHash);  // writes full 32-byte hash

    // Then copy only 8 bytes you need
    std::memcpy(output, fullHash, PUF_SIZE);  // Safe
    delete ctx;
}

void deriveKeyUsingHKDF(const unsigned char* NA, const unsigned char* NB, const unsigned char* S,
    size_t keyLength, unsigned char* derivedKey) {
    // Combine NA and NB into the input key material (IKM)
    unsigned char input_key_material[64];  // 32 bytes + 32 bytes = 64 bytes
    std::memcpy(input_key_material, NA, 32);
    std::memcpy(input_key_material + 32, NB, 32);

    
    // LibTomCrypt HKDF setup
    int err;
    unsigned char prk[32]; // Pseudorandom Key (SHA256 output size)
    unsigned char T[32];
    unsigned int hash_len = 32;
    unsigned int n = (keyLength + hash_len - 1) / hash_len;
    unsigned int outpos = 0;
    unsigned char ctr = 1;
    unsigned long hash_len_l = hash_len;
    // Extract step: PRK = HMAC-Hash(salt, IKM)
    // static bool hash_registered = false;
    DEBUG_PRINT("find hash : %d\n", find_hash("sha256"));
    if (!(find_hash("sha256") >= 0)) {
        register_hash(&sha256_desc);
        // hash_registered = true;
        DEBUG_PRINT("find hash : %d\n", find_hash("sha256"));

    }
    err = hmac_memory(find_hash("sha256"), S, 32, input_key_material, sizeof(input_key_material), prk, &hash_len_l);
    if (err != CRYPT_OK) {
        // handle error
        std::cout << "err = " << err << std::endl;
        return;
    }
    
    // Expand step
    unsigned long tlen = 0;
    unsigned char prev[32];
    hmac_state hmac;
    for (unsigned int i = 0; i < n; ++i) {
        hmac_init(&hmac, find_hash("sha256"), prk, hash_len);

        if (i > 0) {
            hmac_process(&hmac, prev, hash_len);
        }
        // info is empty, so skip
        hmac_process(&hmac, &ctr, 1);
        tlen = hash_len;
        hmac_done(&hmac, T, &tlen);

        unsigned int to_copy = (outpos + hash_len > keyLength) ? (keyLength - outpos) : hash_len;
        std::memcpy(derivedKey + outpos, T, to_copy);
        outpos += to_copy;
        std::memcpy(prev, T, hash_len);
        ctr++;
    }
}

bool extractValueFromMap(std::unordered_map<std::string, std::string> map, std::string key , unsigned char * output, size_t size){

    auto it = map.find(key);
    if (it == map.end()) {
        std::cerr << "Error: key " << key << " not found.\n";
        return false;
    }

    const std::string& valStr = it->second;

    if (valStr.size() != size) {
        std::cerr << "Error: value has incorrect size (" << valStr.size() << ").\n";
        return false;
    }
    std::memcpy(output, valStr.data(), size);
    return true;
}

void warmup(){
    register_hash(&sha256_desc);
}