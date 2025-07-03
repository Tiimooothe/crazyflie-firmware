/**
 * @file SocketModule.hpp
 * @brief SocketModule header
 * 
 * This file holds the SocketModules class header.
 * 
 */

#ifndef SocketModuleRadio_HPP
#define SocketModuleRadio_HPP

// #include <string>
#include <unordered_map>
#include <iomanip>
#include <cstring> 
#include <fstream>
#include <iostream>
#include <queue>

#include "utils.hpp"

extern "C"
{
    
    #include <string.h>
    #include <stdint.h>
    #include <stdbool.h>
    #include <stdio.h>

    #include "radiolink.h"
    #include "configblock.h"
    #include "com.h"

    #include "app.h"

    #include "FreeRTOS.h"
    #include "semphr.h"
    #include "task.h"
    #include "queue.h"

    #include "crtp.h"
    #include "cmp.h"

    #include "debug.h"
}

#define BUFFER_SIZE     512
#define CRTP_PORT_CUSTOM 0x0D

struct CmpBuffer {
    uint8_t buffer[BUFFER_SIZE];
    size_t cursor;
};

/// @brief Socket module class. Its job is to manage everything connection related for a server and a client.
class SocketModuleRadio {
private:
    // int socket_fd;          // Socket file descriptor
    // int connection_fd;      // Used when acting as a server
    // struct sockaddr_in address;
    
    std::string targetDroneId;
    
    CmpBuffer recv_buffer;  // cmp receive buffer structure containing the buffer and a cursor
    CmpBuffer send_buffer;  // cmp sending buffer structure containing the buffer and a cursor
    bool connectionIsOpen;

    cmp_ctx_t ctx_recv;     // cmp receive context
    cmp_ctx_t ctx_send;     // cmp sending context
    
    static bool _reader(cmp_ctx_t *ctx, void *data, size_t count);          // cmp internal reading function
    static size_t _writer(cmp_ctx_t *ctx, const void *data, size_t count);  // cmp internal writting function
    
    public:
    P2PPacket lastSentPacket;
    
    static SocketModuleRadio* instance;
    std::queue<P2PPacket> packetQueue;
    
    SocketModuleRadio();  // Constructor
    
    // Delete copy constructor and copy assignment operator
    SocketModuleRadio(const SocketModuleRadio&) = delete;
    SocketModuleRadio& operator=(const SocketModuleRadio&) = delete;

    // Delete move constructor and move assignment operator
    SocketModuleRadio(SocketModuleRadio&&) = delete;
    SocketModuleRadio& operator=(SocketModuleRadio&&) = delete;


    ~SocketModuleRadio(); // Destructor

    
    /**
     * @brief Create a map of size "size" into the sending buffer
     * 
     * @param size 
     */
    void createMap(uint32_t size);

    /**
     * @brief Add a key value to the sending buffer. This overload adds a std::string to the buffer
     * 
     * @param key 
     * @param value 
     */
    void addKeyValue(const char * key, const std::string &value);

    /**
     * @brief Add a key value to the sending buffer. This overload adds an unsigned char * to the buffer
     * 
     * @param key 
     * @param value 
     * @param valueLen 
     */
    void addKeyValue(const char * key, const unsigned char *value, size_t valueLen);
    
    /**
     * @brief  Add a key value to the sending buffer. This overload adds an array of unsigned char * to the buffer
     * 
     * @param key 
     * @param LC 
     */
    void addKeyValue(const char * key, const unsigned char LC[CHALLENGE_SIZE][PUF_SIZE]);
    
    /**
     * @brief Send a msgPack over the socket.
     * 
     * @param ctx cmp context of the message to send
     * @param len Length of the message to send
     */
    void sendMsg();
    void resendLastPacket();

    /**
     * @brief Receive a message on the msgPack format and return it in the unordered_map "msg".  
     * 
     * @param msg 
     */
    void receiveMsg(std::unordered_map<std::string,std::string> &msg, const char * waitForKey = "");

    /**
     * @brief Print the content of a MsgPack value.
     * 
     * @param msg 
     */
    void printMsg(CmpBuffer &buffer, size_t size);
    
};

#endif
