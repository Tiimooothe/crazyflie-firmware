/**
 * @file SocketModule.cpp
 * @brief SocketModule implementation
 * 
 * This file holds the SocketModules class implementation.
 * 
 */


#include "socketModuleRadio.hpp"

SocketModuleRadio* SocketModuleRadio::instance = nullptr;
SemaphoreHandle_t queueMutex;

void p2pcallbackHandler(P2PPacket* p) {
    DEBUG_PRINT("Handler called for packet of size %d!\n",p->size);
    if (!p || !SocketModuleRadio::instance) return;
    
    if (xSemaphoreTake(queueMutex, portMAX_DELAY)) {
        SocketModuleRadio::instance->packetQueue.push(*p);
        xSemaphoreGive(queueMutex);
    }

    DEBUG_PRINT("Handler exit!\n");
}

void socketRadioInit(void) {
    queueMutex = xSemaphoreCreateMutex();
    p2pRegisterCB(p2pcallbackHandler);
}

/// @brief Constructor: Initializes socket
SocketModuleRadio::SocketModuleRadio() : recv_buffer{{0},0}, send_buffer{{0},0}, connectionIsOpen{false} {
    SocketModuleRadio::instance = this;

    cmp_init(&ctx_recv, &recv_buffer, _reader, NULL, NULL);
    cmp_init(&ctx_send, &send_buffer, NULL, NULL, _writer);

    socketRadioInit();
}

bool SocketModuleRadio::_reader(cmp_ctx_t *ctx, void *data, size_t count){
    CmpBuffer *recv_buffer = static_cast<CmpBuffer *>(ctx->buf);        // Static function cannot access instance members

    if (recv_buffer->cursor + count > BUFFER_SIZE){
        return false;
    }
    memcpy(data, &recv_buffer->buffer[recv_buffer->cursor], count);
    recv_buffer->cursor += count;       // Update cursor
    return true;
}

size_t SocketModuleRadio::_writer(cmp_ctx_t *ctx, const void *data, size_t count){
    CmpBuffer *send_buffer = static_cast<CmpBuffer *>(ctx->buf);         // Static function cannot access instance members

    if (send_buffer->cursor + count > BUFFER_SIZE){
        return false;
    }
    memcpy(&send_buffer->buffer[send_buffer->cursor], data, count);
    send_buffer->cursor += count;       // Update cursor
    return true;
}

void SocketModuleRadio::createMap(uint32_t size){
    cmp_write_map(&ctx_send, size);
}

void SocketModuleRadio::addKeyValue(const char  * key, const std::string &value) {
    cmp_write_str(&ctx_send, key, strlen(key));
    cmp_write_str(&ctx_send, value.c_str(), value.size());
}

void SocketModuleRadio::addKeyValue(const char  * key, const unsigned char *value, size_t valueLen) {
    cmp_write_str(&ctx_send, key, strlen(key));
    cmp_write_bin(&ctx_send, value, valueLen);
}

void SocketModuleRadio::addKeyValue(const char *key, const unsigned char L[CHALLENGE_SIZE][PUF_SIZE]) {
    cmp_write_str(&ctx_send, key, strlen(key));
    cmp_write_bin(&ctx_send, reinterpret_cast<const unsigned char*>(L), CHALLENGE_SIZE * PUF_SIZE);
}

void SocketModuleRadio::sendMsg(){
    size_t len = send_buffer.cursor;
    DEBUG_PRINT("About to send %d bytes\n", len);
    if (len > P2P_MAX_DATA_SIZE) {
        DEBUG_PRINT("Error: Message too large for CRTP packet (%zu bytes)\n", len);
        return;
    }

    P2PPacket p_reply;
    p_reply.port=0x00;
    p_reply.size=len;
    memcpy(&p_reply.data[0], &send_buffer.buffer, len);

    // Store the last packet 
    lastSentPacket = p_reply;

    radiolinkSendP2PPacketBroadcast(&p_reply);

    send_buffer.cursor = 0; // Clear the buffer by moving the cursor
}

void SocketModuleRadio::resendLastPacket() {
    DEBUG_PRINT("Resending last packet (%d bytes)\n", lastSentPacket.size);
    radiolinkSendP2PPacketBroadcast(&lastSentPacket);
}

/**
 * @brief Receive a message on the msgPack format and return it in the unordered_map msg.  
 * 
 * @param msg 
 */
void SocketModuleRadio::receiveMsg(std::unordered_map<std::string,std::string> &msg, const char * waitForKey) {
    DEBUG_PRINT("receivedMsg called!\n");
    // char buffer[1024] = {0};
    size_t bytesReceived = -1;

    xSemaphoreTake(queueMutex, portMAX_DELAY);
    if (packetQueue.empty()) {
        DEBUG_PRINT("packetQueue was empty \n");
        xSemaphoreGive(queueMutex);  
        return;
    }

    DEBUG_PRINT("packetQueue not empty \n");

    while(!packetQueue.empty()){

    // Pop the front packet
    P2PPacket packet = packetQueue.front();  // Look at the front
    packetQueue.pop();                       // Then remove it
    xSemaphoreGive(queueMutex);                    
    
    bytesReceived = packet.size;
    std::memcpy(&recv_buffer.buffer[recv_buffer.cursor], packet.data, bytesReceived);

    if (bytesReceived > 0 && bytesReceived != (size_t)-1) {
        msg.clear();

        DEBUG_PRINT("Received %d bytes.\n",bytesReceived);

        for (size_t i = 0; i < bytesReceived; i++) {
            DEBUG_PRINT("%02x ", recv_buffer.buffer[i]);
        }
        DEBUG_PRINT("\n");
        
        // printMsg(recv_buffer,bytesReceived);
        
        uint32_t map_size;
        if (!cmp_read_map(&ctx_recv, &map_size)) {
            // Handle error: failed to read map header
            DEBUG_PRINT("Failed to read map size: %d\n", ctx_recv.error);
            recv_buffer.cursor = 0; 
            return;
        }

        DEBUG_PRINT("Received message : ");

        for (uint32_t i = 0; i < map_size; i++) {
            uint32_t key_len = 64;
            char key[key_len];

            if (!cmp_read_str(&ctx_recv, key, &key_len)) {
                DEBUG_PRINT("Failed to read key data: %d\n", ctx_recv.error);
                recv_buffer.cursor = 0; 
                return;
            }
        
            uint32_t val_len = CHALLENGE_SIZE*PUF_SIZE;
            char val[val_len];
            
            size_t saved_cursor = recv_buffer.cursor;
            cmp_object_t obj;
            if (!cmp_read_object(&ctx_recv, &obj)) {
                // handle error
            }
            recv_buffer.cursor = saved_cursor;

            switch(obj.type) {
                case CMP_TYPE_BIN8:
                if (!cmp_read_bin(&ctx_recv, val, &val_len)) {
                        DEBUG_PRINT("Failed to read val data : %d\n",ctx_recv.error);
                        recv_buffer.cursor = 0; 
                        return;
                    }
                    break;
                    case CMP_TYPE_FIXSTR:
                    if (!cmp_read_str(&ctx_recv, val, &val_len)) {
                        DEBUG_PRINT("Failed to read val data : %d\n",ctx_recv.error);
                        recv_buffer.cursor = 0; 
                        return;      
                    }
                    break;
            }
                
            msg.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(std::string(key, key_len)),
            std::forward_as_tuple(std::string(val, val_len))  
            );
            }
            if(waitForKey != nullptr && waitForKey[0] != '\0'){
                if (msg.find(waitForKey) != msg.end()) {
                    recv_buffer.cursor = 0;
                    return;
                } else {
                    msg.clear();
                    recv_buffer.cursor = 0;
                }
            }
            else{
                recv_buffer.cursor = 0;     // Clear the buffer by moving the cursor
                return ;
            }              
        }    
        else if (bytesReceived == 0) {
            std::cerr << "Connection closed by peer." << std::endl;
            recv_buffer.cursor = 0;
            return;
        } 
        else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::cerr << "Receive timeout!" << std::endl;
            } 
            else {
                perror("Receive failed");
            }
            recv_buffer.cursor = 0;
            return;
        }
    }
}

void SocketModuleRadio::printMsg(CmpBuffer &buffer, size_t size) {
    DEBUG_PRINT("Sending message : \n");
    // std::cout << "Raw buffer = "; print_hex(buffer.buffer,size); std::cout << std::endl;
    // Create a temporary copy of the buffer for safe reading
    CmpBuffer temp_buffer;
    memcpy(temp_buffer.buffer, buffer.buffer, size);
    temp_buffer.cursor = 0;

    // Create a new CMP context with the temporary buffer
    cmp_ctx_t temp_ctx;
    cmp_init(&temp_ctx, &temp_buffer.buffer, _reader, NULL, NULL);

    uint32_t map_size = 0;
    if (!cmp_read_map(&temp_ctx, &map_size)) {
        DEBUG_PRINT("Failed to read map size: %d\n", temp_ctx.error);
        return;
    }

    DEBUG_PRINT("The map has a size %ld\n",map_size);

    for (uint32_t i = 0; i < map_size; i++) {
        uint32_t key_len = 64;
        char key[key_len];

        if (!cmp_read_str(&temp_ctx, key, &key_len)) {
            DEBUG_PRINT("Failed to read key data: %d\n", temp_ctx.error);
            break;
        }
        
        size_t saved_cursor = temp_buffer.cursor;
        cmp_object_t obj;
        if (!cmp_read_object(&temp_ctx, &obj)) {
            DEBUG_PRINT("Failed to read object: error code %d\n",temp_ctx.error);
            return;
        }
        temp_buffer.cursor = saved_cursor;

        uint32_t val_len = CHALLENGE_SIZE*PUF_SIZE;
        char val[val_len];

        switch(obj.type) {
            case CMP_TYPE_BIN8:
                if (!cmp_read_bin(&temp_ctx, val, &val_len)) {
                    DEBUG_PRINT("Failed to read val data : %d\n",temp_ctx.error);
                    return;
                }
                DEBUG_PRINT("%s : ",key); print_hex(reinterpret_cast<unsigned char*>(val),val_len);
                break;
            case CMP_TYPE_FIXSTR:
                if (!cmp_read_str(&temp_ctx, val, &val_len)) {
                    DEBUG_PRINT("Failed to read val data : %d\n",temp_ctx.error);
                    return;
                }
                DEBUG_PRINT("%s : %s",key,val);
                break;
        }
    }
    DEBUG_PRINT("Cursor at the end of print = %d\n",temp_buffer.cursor);
}

/// @brief Destructor ensures the connection is closed
SocketModuleRadio::~SocketModuleRadio() {
    // closeConnection();
}

