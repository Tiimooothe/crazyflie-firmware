/**
 * @file UAV.cpp
 * @brief UAV class implementation.
 * 
 * This file implements the UAV class. 
 * 
 */
#include "UAV.hpp"

/// @brief Constructor
UAVData::UAVData(
    const unsigned char* x, const unsigned char* c, const unsigned char* r, 
    const unsigned char* xLock, const unsigned char* secret
) : x(nullptr), c(nullptr), r(nullptr), xLock(nullptr), secret(nullptr) {
    if (x) { this->x = new unsigned char[PUF_SIZE]; memcpy(this->x, x, PUF_SIZE); }
    if (c) { this->c = new unsigned char[PUF_SIZE]; memcpy(this->c, c, PUF_SIZE); }
    if (r) { this->r = new unsigned char[PUF_SIZE]; memcpy(this->r, r, PUF_SIZE); }
    if (xLock) { this->xLock = new unsigned char[PUF_SIZE]; memcpy(this->xLock, xLock, PUF_SIZE); }
    if (secret) { this->secret = new unsigned char[PUF_SIZE]; memcpy(this->secret, secret, PUF_SIZE); }
}

/// @brief Destructor
UAVData::~UAVData() {
    delete[] x;
    delete[] c;
    delete[] r;
    delete[] xLock;
    delete[] secret;
}

/// @brief Copy Constructor
UAVData::UAVData(const UAVData& other) : x(nullptr), c(nullptr), r(nullptr), xLock(nullptr), secret(nullptr) {
    if (other.x) { this->x = new unsigned char[PUF_SIZE]; memcpy(this->x, other.x, PUF_SIZE); }
    if (other.c) { this->c = new unsigned char[PUF_SIZE]; memcpy(this->c, other.c, PUF_SIZE); }
    if (other.r) { this->r = new unsigned char[PUF_SIZE]; memcpy(this->r, other.r, PUF_SIZE); }
    if (other.xLock) { this->xLock = new unsigned char[PUF_SIZE]; memcpy(this->xLock, other.xLock, PUF_SIZE); }
    if (other.secret) { this->secret = new unsigned char[PUF_SIZE]; memcpy(this->secret, other.secret, PUF_SIZE); }
}

/// @brief Copy Assignment Operator
UAVData& UAVData::operator=(const UAVData& other) {
    if (this == &other) return *this; // Avoid self-assignment

    delete[] x; delete[] c; delete[] r; delete[] xLock; delete[] secret;

    x = (other.x) ? new unsigned char[PUF_SIZE] : nullptr;
    c = (other.c) ? new unsigned char[PUF_SIZE] : nullptr;
    r = (other.r) ? new unsigned char[PUF_SIZE] : nullptr;
    xLock = (other.xLock) ? new unsigned char[PUF_SIZE] : nullptr;
    secret = (other.secret) ? new unsigned char[PUF_SIZE] : nullptr;

    if (x) memcpy(x, other.x, PUF_SIZE);
    if (c) memcpy(c, other.c, PUF_SIZE);
    if (r) memcpy(r, other.r, PUF_SIZE);
    if (xLock) memcpy(xLock, other.xLock, PUF_SIZE);
    if (secret) memcpy(secret, other.secret, PUF_SIZE);

    return *this;
}

/// @brief Getter Methods
const unsigned char* UAVData::getX() const { return x; }
const unsigned char* UAVData::getC() const { return c; }
const unsigned char* UAVData::getR() const { return r; }
const unsigned char* UAVData::getXLock() const { return xLock; }
const unsigned char* UAVData::getSecret() const { return secret; }

/// @brief Setter Methods
void UAVData::setX(const unsigned char* newX) { updatePointer(x, newX); }
void UAVData::setC(const unsigned char* newC) { updatePointer(c, newC); }
void UAVData::setR(const unsigned char* newR) { updatePointer(r, newR); }
void UAVData::setXLock(const unsigned char* newXLock) { updatePointer(xLock, newXLock); }
void UAVData::setSecret(const unsigned char* newSecret) { updatePointer(secret, newSecret); }

/// @brief Helper function to safely update pointers
void UAVData::updatePointer(unsigned char*& dest, const unsigned char* src) {
    // DEBUG_PRINT("DEBUG !!\n");
    delete[] dest;  // Free previous memory if allocated
    if (src) {
        dest = new unsigned char[PUF_SIZE];
        memcpy(dest, src, PUF_SIZE);
    } else {
        dest = nullptr;
    }
}

// UAV
/// @brief Constructor implementation
UAV::UAV(std::string id) : id(id), PUF() {}

UAV::UAV(std::string id, unsigned char * salt) : id(id), PUF(salt) {}

/// @brief Method implementation
std::string UAV::getId() {
    return this->id;
}

/// @brief Add an UAV to the UAV table.
/// @param id 
/// @param x 
/// @param c 
/// @param r 
/// @param xLock 
/// @param secret 
void UAV::addUAV(
        const std::string& id, 
        const unsigned char* x,
        const unsigned char* c,
        const unsigned char* r,
        const unsigned char* xLock,
        const unsigned char* secret
    ){
    uavTable.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(id),
        std::forward_as_tuple(x,c,r,xLock,secret)
    );
}

/// @brief Remove an UAV from the UAv table.
/// @param id 
/// @return 
bool UAV::removeUAV(const std::string& id) {
    return uavTable.erase(id) > 0;
}

/// @brief Get a UAV data in the UAV table.
/// @param id 
/// @return 
UAVData* UAV::getUAVData(const std::string& id) {
    auto it = uavTable.find(id);
    return (it != uavTable.end()) ? &(it->second) : nullptr;
}

/// @brief Call the UAV internal PUF for a computation.
/// @param input 
/// @param response 
void UAV::callPUF(const unsigned char * input, unsigned char * response){
    this->PUF.process(input, sizeof(input), response);
}

int UAV::init_connection_client(){
    std::unordered_map<std::string, std::string> map;
    std::string str = "pong";

    socketModule.createMap(1);
    socketModule.addKeyValue("ping",str);
    socketModule.sendMsg();
    
    socketModule.receiveMsg(map);

    while(map.empty()){
      DEBUG_PRINT("(client) : Waiting for counterpart's answer ...\n");
      socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(map);
    }

    DEBUG_PRINT("(client) : Connected with counterpart ...\n");
    return 0;
}

int UAV::init_connection_server(){
    std::unordered_map<std::string, std::string> map;
    socketModule.receiveMsg(map);

    while(map.empty()){
      DEBUG_PRINT("(server) : Waiting for counterpart's answer ...\n");
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(map);
    }

    std::string str = "ping";

    socketModule.createMap(1);
    socketModule.addKeyValue("pong",str);
    socketModule.sendMsg();

    DEBUG_PRINT("(server) : Connected with counterpart ...\n");
    return 0;
}

/// @brief Print the UAV data.
/// @param none
int UAV::enrolment_client(){
    // DEBUG_PRINT("\nEnrolment process begins.\n");
    // DEBUG_PRINT("Stack watermark: %lu\n", uxTaskGetStackHighWaterMark(NULL));

    #ifdef MEASUREMENTS_DETAILLED
        long long start;
        long long end;
        long long idlCycles = 0;
        long long opCycles = 0;
        CycleCounter counter;
    #endif

    
    // A enroll with B
    // A computes the challenge for B
    unsigned char xB[PUF_SIZE];
    generate_random_bytes(xB, PUF_SIZE);
    // DEBUG_PRINT("xB : "); print_hex(xB, PUF_SIZE);

    // Creates B in the memory of A and save xB 
    this->addUAV("B", xB);

    // Creates the challenge for B
    unsigned char CB[PUF_SIZE];
    this->callPUF(xB, CB);
    // DEBUG_PRINT("CB : "); print_hex(CB, PUF_SIZE);

    // Sends CB
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("CB", CB, PUF_SIZE);
    
    this->socketModule.sendMsg();
    // DEBUG_PRINT("Sent CB.\n");
        
    // Wait for B's response (with RB)
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(2);
    this->socketModule.receiveMsg(msg);
    while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // // DEBUG_PRINT(printMsg(msg);;);
    
    // Check if an error occurred
    if (msg.empty()) {
        // DEBUG_PRINT("Error occurred: content is empty!");
        return 0;
    }

    unsigned char RB[PUF_SIZE];
    extractValueFromMap(msg,"RB",RB,PUF_SIZE);
    // DEBUG_PRINT("RB : "); print_hex(RB, PUF_SIZE);

    msg.clear();

    this->getUAVData("B")->setR(RB);

    // DEBUG_PRINT("\nB is enroled to A\n");

    // B enroll with A
    // A receive CA. It saves CA.
    this->socketModule.receiveMsg(msg);
    while(msg.empty()){
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // // DEBUG_PRINT(printMsg(msg);;);
    
    // Check if an error occurred
    if (msg.empty()) {
        // DEBUG_PRINT("Error occurred: content is empty!");
        return 0;
    }

    unsigned char CA[PUF_SIZE];
    extractValueFromMap(msg,"CA",CA,PUF_SIZE);
    // DEBUG_PRINT("CA : "); print_hex(CA, PUF_SIZE);
    
    msg.clear();

    this->getUAVData("B")->setC(CA);

    // A computes RA
    unsigned char RA[PUF_SIZE];
    this->callPUF(CA, RA);
    // DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);
    
    // A sends RA   
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("RA", RA, PUF_SIZE);

    this->socketModule.sendMsg();

    return 0;
}

/// @brief Enrolment of the UAV.
/// @param none
/// @return 0 if success, 1 if failure
int UAV::enrolment_server(){
    // DEBUG_PRINT("\nEnrolment process begins.\n");
    
    #ifdef MEASUREMENTS_DETAILLED
        long long start;
        long long end;
        long long idlCycles = 0;
        long long opCycles = 0;
        CycleCounter counter;
    #endif
    
    // B waits for B's message  (with CB)
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(2);
    this->socketModule.receiveMsg(msg);
    while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    
    // Check if an error occurred
    if (msg.empty()) {
        // DEBUG_PRINT("Error occurred: content is empty");
        return -1;
    }
    
    // B receive CB. It creates A in the memory of B and save CB.
    unsigned char CB[PUF_SIZE];
    extractValueFromMap(msg,"CB",CB,PUF_SIZE);
    // DEBUG_PRINT("CB : "); print_hex(CB, PUF_SIZE);
        
    msg.clear();

    this->addUAV("A", nullptr, CB);

    // B computes RB
    unsigned char RB[PUF_SIZE];
    this->callPUF(CB, RB);
    // DEBUG_PRINT("RB : "); print_hex(RB, PUF_SIZE);

    // B sends RB
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("RB", RB, PUF_SIZE);

    this->socketModule.sendMsg();
    // DEBUG_PRINT("Sent RB.\n"); 

    msg.clear();

    // DEBUG_PRINT("\nB is enroled to A\n");

    // B enroll with A
    // Computes xA
    unsigned char xA[PUF_SIZE];
    generate_random_bytes(xA, PUF_SIZE);
    // DEBUG_PRINT("xA : "); print_hex(xA, PUF_SIZE);

    // Save xA
    this->getUAVData("A")->setX(xA);

    // Creates the challenge for A
    unsigned char CA[PUF_SIZE];
    this->callPUF(xA, CA);
    // DEBUG_PRINT("CA : "); print_hex(CA, PUF_SIZE);

    // Sends CA
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("CA", CA, PUF_SIZE);

    this->socketModule.sendMsg();
    // DEBUG_PRINT("Sent CA.\n");
    
    msg.clear();

    // B receive RA and saves it. 
    this->socketModule.receiveMsg(msg);
    while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    
    // Check if an error occurred
    if (msg.empty()) {
        // DEBUG_PRINT("Error occurred: content is empty!\n");
        return -1;
    }

    unsigned char RA[PUF_SIZE];
    extractValueFromMap(msg,"RA",RA,PUF_SIZE);
    // DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);
    
    this->getUAVData("A")->setR(RA);
    // DEBUG_PRINT("\nA is enroled to B\n");
    
    return 0;
}

/// @brief Authenticate the UAV.
/// @param none
/// @return 0 if success, 1 if failure
int UAV::autentication_client(){
    // The client initiate the authentication process
    // DEBUG_PRINT("\nAutentication process begins.\n");

    #ifdef MEASUREMENTS_DETAILLED
        long long start;
        long long end;
        long long idlCycles = 0;
        long long opCycles = 0;
        CycleCounter counter;
    #endif
    
    
    // A generates a nonce NA 
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA, PUF_SIZE);
    // DEBUG_PRINT("NA : "); print_hex(NA, PUF_SIZE);

    const unsigned char * CA = this->getUAVData("B")->getC();
    if (CA == nullptr){
        // DEBUG_PRINT("No expected challenge in memory for this UAV.\n");
        return 1;
    }
    
    unsigned char M0[PUF_SIZE];
    xor_buffers(NA,CA,PUF_SIZE,M0);
    // DEBUG_PRINT("M0 : "); print_hex(M0, PUF_SIZE);

    // A sends its ID and NA to B 
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M0", M0, PUF_SIZE);
    
    this->socketModule.sendMsg();
    // DEBUG_PRINT("Sent ID and M0.\n");
    
    // A waits for the answer
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(3);
    this->socketModule.receiveMsg(msg);
    while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }

    // Check if an error occurred
    if (msg.empty()) {
        // DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // A recover M1 and the hash
    unsigned char M1[PUF_SIZE];
    extractValueFromMap(msg,"M1",M1,PUF_SIZE);

    unsigned char hash1[PUF_SIZE];
    extractValueFromMap(msg,"hash1",hash1,PUF_SIZE);

    msg.clear();

    // A computes RA using CA in memory
    unsigned char RA[PUF_SIZE];
    this->callPUF(CA,RA);
    // DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);
    
    // A retrieve NB from M1 
    unsigned char NB[PUF_SIZE];
    xor_buffers(M1, NA, PUF_SIZE, NB);
    xor_buffers(NB, RA, PUF_SIZE, NB);
    // DEBUG_PRINT("NB : "); print_hex(NB, PUF_SIZE);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    hash_state* ctx = initHash();
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);
    // DEBUG_PRINT("hash1Check : "); print_hex(hash1Check, PUF_SIZE);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    // DEBUG_PRINT("A verify B's hash : %d\n" ,res); 

    if(res == 0){
        // DEBUG_PRINT("The autentication failed. A will try to verify the hash with an old challenge if it exists.\n");

        // A will recover the old challenge 
        const unsigned char * xLock = this->getUAVData("B")->getXLock();
        if (xLock == nullptr){
            // DEBUG_PRINT("No old challenge in memory for the requested UAV.\n");
            return 1;
        }
        const unsigned char * secret = this->getUAVData("B")->getSecret();
        if (secret == nullptr){
            // DEBUG_PRINT("No old challenge in memory for the requested UAV.\n");
            return 1;
        }
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
        unsigned char CAOld[PUF_SIZE]; 
        xor_buffers(lock, secret, PUF_SIZE, CAOld);

        // A will calculate the Nonce A that the server calculated with the wrong CA 
        unsigned char NAOld[PUF_SIZE];
        xor_buffers(M0, CAOld, PUF_SIZE, NAOld);
        // DEBUG_PRINT("NAOld : "); print_hex(NAOld, PUF_SIZE);

        // A will calculate the old response 
        unsigned char RAOld[PUF_SIZE];
        this->callPUF(CAOld, RAOld);
        // DEBUG_PRINT("RAOld : "); print_hex(RAOld, PUF_SIZE);

        // A will deduce NB from the old response
        unsigned char NBOld[PUF_SIZE];
        xor_buffers(M1, RAOld, PUF_SIZE, NBOld);
        xor_buffers(NBOld, NAOld, PUF_SIZE, NBOld);
        // DEBUG_PRINT("NBOld : "); print_hex(NBOld, PUF_SIZE);

        // A now tries to verify the hash with this value
        ctx = initHash();
        addToHash(ctx, CAOld, PUF_SIZE);
        addToHash(ctx, NBOld, PUF_SIZE);
        addToHash(ctx, RAOld, PUF_SIZE);
        addToHash(ctx, NAOld, PUF_SIZE);
        calculateHash(ctx, hash1Check);

        res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
        if (res == 0){
            // DEBUG_PRINT("Even with the old challenge, autentication has failed.\n");
            return 1;
        }
        // DEBUG_PRINT("B has been autenticated by A with the old challenge.\n");

        // A will now change the values to be the one obtained of the old challenge
        this->getUAVData("B")->setC(CAOld);
        memcpy(RA, RAOld, PUF_SIZE);
        memcpy(NB, NBOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);

    }

    // DEBUG_PRINT("B's hash has been verified. B is autenticated to A.\n");

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    this->callPUF(NB,RAp);
    // DEBUG_PRINT("RAp : "); print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NA,RAp,PUF_SIZE,M2);
    // DEBUG_PRINT("M2 : "); print_hex(M2, PUF_SIZE);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);

    // Send M2, and a hash of NB, RA, RAp, NA

    socketModule.createMap(3);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M2", M2, PUF_SIZE);
    socketModule.addKeyValue("hash2", hash2, PUF_SIZE);

    this->socketModule.sendMsg();
    // DEBUG_PRINT("Sent ID, M2 and hash2.\n");
    
    msg.clear();

    // A waits for B's ACK
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    
    bool messageInvalid = false;

    unsigned char hash3[PUF_SIZE];
    unsigned char hash3Check[PUF_SIZE];

    // Verify hash
    if (msg.empty()) {
        // DEBUG_PRINT("Error: message is empty\n");
        // DEBUG_PRINT("Received an empty MsgPack message!\n");
        messageInvalid = true;
    } 
    else if (!extractValueFromMap(msg,"hash3",hash3,PUF_SIZE)){
        // DEBUG_PRINT("Error: message structure or fields are invalid\n");
        messageInvalid = true;
    }
    else{
        // Verify hash3
        ctx = initHash();
        addToHash(ctx, RAp, PUF_SIZE);
        addToHash(ctx, NB, PUF_SIZE);
        addToHash(ctx, NA, PUF_SIZE);
        calculateHash(ctx, hash3Check);
        // DEBUG_PRINT("hash3Check : "); print_hex(hash3Check, PUF_SIZE);
    }

    // Check if an error occurred
    if (messageInvalid || memcmp(hash3, hash3Check, PUF_SIZE) != 0) {
        // DEBUG_PRINT("Error occurred: content is empty\n");

        // A reach a timeout or didn't received the ACK 
        // DEBUG_PRINT("Received an empty MsgPack message!\n");
    
        // A will save concealed current CA in CAOld
        unsigned char xLock[PUF_SIZE];
        generate_random_bytes(xLock, PUF_SIZE);
    
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
    
        unsigned char concealedCA[PUF_SIZE];
        xor_buffers(CA,lock,PUF_SIZE,concealedCA);
    
        this->getUAVData("B")->setXLock(xLock);
        this->getUAVData("B")->setSecret(concealedCA);

        // Then A saves the new challenge in CA
        this->getUAVData("B")->setC(NB);

        return 1;
    }

    // Then A saves the new challenge in CA
    this->getUAVData("B")->setC(NB);

    // Finished
    // DEBUG_PRINT("\nThe two UAV autenticated each other.\n");
    
    return 0;
}

/// @brief Authenticate the UAV and establish a session key K.
/// @param none
/// @return 0 if success, 1 if failure
int UAV::autentication_key_client(){
    // The client initiate the authentication process
    DEBUG_PRINT("\nAutentication process begins.\n");
    #ifdef MEASUREMENTS_DETAILLED
        long long start;
        long long end;
        long long idlCycles = 0;
        long long opCycles = 0;
        CycleCounter counter;
    #endif

    // A generates a nonce NA 
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA, PUF_SIZE);
    DEBUG_PRINT("NA : "); print_hex(NA, PUF_SIZE);

    const unsigned char * CA = this->getUAVData("B")->getC();
    if (CA == nullptr){
        DEBUG_PRINT("No expected challenge in memory for this UAV.\n");
        return 1;
    }
    
    unsigned char M0[PUF_SIZE];
    xor_buffers(NA,CA,PUF_SIZE,M0);
    DEBUG_PRINT("M0 : "); print_hex(M0, PUF_SIZE);

    // A sends its ID and NA to B 
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M0", M0, PUF_SIZE);
    
    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID and M0.\n");
            
    // A waits for the answer
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(4);
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);
    
    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // A recover M1 and the hash
    unsigned char M1[PUF_SIZE];
    extractValueFromMap(msg,"M1",M1,PUF_SIZE);

    unsigned char hash1[PUF_SIZE];
    extractValueFromMap(msg,"hash1",hash1,PUF_SIZE);

    msg.clear();

    // A computes RA using CA in memory
    unsigned char RA[PUF_SIZE];
    this->callPUF(CA,RA);
    DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);
    
    // A retrieve NB from M1 
    unsigned char NB[PUF_SIZE];
    xor_buffers(M1, NA, PUF_SIZE, NB);
    xor_buffers(NB, RA, PUF_SIZE, NB);
    DEBUG_PRINT("NB : "); print_hex(NB, PUF_SIZE);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    hash_state * ctx = initHash();
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);
    DEBUG_PRINT("hash1Check : "); print_hex(hash1Check, PUF_SIZE);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    DEBUG_PRINT("A verify B's hash : %d\n" ,res); 

    if(res == 0){
        DEBUG_PRINT("The autentication failed. A will try to verify the hash with an old challenge if it exists.\n");

        // A will recover the old challenge 
        const unsigned char * xLock = this->getUAVData("B")->getXLock();
        if (xLock == nullptr){
            DEBUG_PRINT("No old challenge in memory for the requested UAV.\n");
            return 1;
        }
        const unsigned char * secret = this->getUAVData("B")->getSecret();
        if (secret == nullptr){
            DEBUG_PRINT("No old challenge in memory for the requested UAV.\n");
            return 1;
        }
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
        unsigned char CAOld[PUF_SIZE]; 
        xor_buffers(lock, secret, PUF_SIZE, CAOld);

        // A will calculate the Nonce A that the server calculated with the wrong CA 
        unsigned char NAOld[PUF_SIZE];
        xor_buffers(M0, CAOld, PUF_SIZE, NAOld);
        DEBUG_PRINT("NAOld : "); print_hex(NAOld, PUF_SIZE);

        // A will calculate the old response 
        unsigned char RAOld[PUF_SIZE];
        this->callPUF(CAOld, RAOld);
        DEBUG_PRINT("RAOld : "); print_hex(RAOld, PUF_SIZE);

        // A will deduce NB from the old response
        unsigned char NBOld[PUF_SIZE];
        xor_buffers(M1, RAOld, PUF_SIZE, NBOld);
        xor_buffers(NBOld, NAOld, PUF_SIZE, NBOld);
        DEBUG_PRINT("NBOld : "); print_hex(NBOld, PUF_SIZE);

        // A now tries to verify the hash with this value
        ctx = initHash();
        addToHash(ctx, CAOld, PUF_SIZE);
        addToHash(ctx, NBOld, PUF_SIZE);
        addToHash(ctx, RAOld, PUF_SIZE);
        addToHash(ctx, NAOld, PUF_SIZE);
        calculateHash(ctx, hash1Check);

        res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
        if (res == 0){
            DEBUG_PRINT("Even with the old challenge, autentication has failed.\n");
            return 1;
        }
        DEBUG_PRINT("B has been autenticated by A with the old challenge.\n");

        // A will now change the values to be the one obtained of the old challenge
        this->getUAVData("B")->setC(CAOld);
        memcpy(RA, RAOld, PUF_SIZE);
        memcpy(NB, NBOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);

    }

    DEBUG_PRINT("B's hash has been verified. B is autenticated to A.\n");

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    this->callPUF(NB,RAp);
    DEBUG_PRINT("RAp : "); print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NA,RAp,PUF_SIZE,M2);
    DEBUG_PRINT("M2 : "); print_hex(M2, PUF_SIZE);

    // Generates key
    unsigned char S[PUF_SIZE];
    generate_random_bytes(S, PUF_SIZE);
    DEBUG_PRINT("S : "); print_hex(S, PUF_SIZE);

    unsigned char MK[PUF_SIZE];
    xor_buffers(S,NA,PUF_SIZE,MK);
    xor_buffers(MK,NB,PUF_SIZE,MK);

    unsigned char K[PUF_SIZE];
    deriveKeyUsingHKDF(NA, NB, S, PUF_SIZE, K);
    DEBUG_PRINT("K : "); print_hex(K, PUF_SIZE);

    // A sends M2, and a hash of NB, RA, RAp, NA, K
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    addToHash(ctx, K, PUF_SIZE);
    calculateHash(ctx, hash2);
    DEBUG_PRINT("hash2 : "); print_hex(hash2, PUF_SIZE);

    // Send M2, MK, and a hash of NB, RA, RAp, NA, K
    socketModule.createMap(4);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M2", M2, PUF_SIZE);
    socketModule.addKeyValue("MK", MK, PUF_SIZE);
    socketModule.addKeyValue("hash2", hash2, PUF_SIZE);

    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID, M2, MK and hash2.\n");
    
    msg.clear();

    // A waits for B's ACK
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);
    
    bool messageInvalid = false;

    unsigned char hash3[PUF_SIZE];
    unsigned char hash3Check[PUF_SIZE];

    // Verify hash
    if (msg.empty()) {
        DEBUG_PRINT("Error: message is empty\n");
        DEBUG_PRINT("Received an empty MsgPack message!\n");
        messageInvalid = true;
    } 
    else if (!extractValueFromMap(msg,"hash3",hash3,PUF_SIZE)){
        DEBUG_PRINT("Error: message structure or fields are invalid\n");
        messageInvalid = true;
    }
    else{
        // Verify hash3
        ctx = initHash();
        addToHash(ctx, RAp, PUF_SIZE);
        addToHash(ctx, K, PUF_SIZE);
        addToHash(ctx, NB, PUF_SIZE);
        addToHash(ctx, NA, PUF_SIZE);
        calculateHash(ctx, hash3Check);
        DEBUG_PRINT("hash3Check : "); print_hex(hash3Check, PUF_SIZE);
    }

    // Check if an error occurred
    if (messageInvalid || memcmp(hash3, hash3Check, PUF_SIZE) != 0) {
        DEBUG_PRINT("Error occurred: content is empty\n");

        // A reach a timeout or didn't received the ACK 
        DEBUG_PRINT("Received an empty MsgPack message!\n");
    
        // A will save concealed current CA in CAOld
        unsigned char xLock[PUF_SIZE];
        generate_random_bytes(xLock, PUF_SIZE);
    
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
    
        unsigned char concealedCA[PUF_SIZE];
        xor_buffers(CA,lock,PUF_SIZE,concealedCA);
    
        this->getUAVData("B")->setXLock(xLock);
        this->getUAVData("B")->setSecret(concealedCA);

        // Then A saves the new challenge in CA
        this->getUAVData("B")->setC(NB);

        return 1;
    }

    // Then A saves the new challenge in CA
    this->getUAVData("B")->setC(NB);

    // Finished
    DEBUG_PRINT("\nThe two UAV autenticated each other.\n");
        return 0;
}

/// @brief Authenticate the UAV.
/// @param none
/// @return 0 if success, 1 if failure
int UAV::autentication_server(){
    // The client initiate the autentication process
    DEBUG_PRINT("\nAutentication process begins.\n");
    #ifdef MEASUREMENTS_DETAILLED
        long long start;
        long long end;
        long long idlCycles = 0;
        long long opCycles = 0;
        CycleCounter counter;
    #endif
    
    
    // B receive the initial message
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(3);
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    
    // Check if an error occurred
    if (msg.empty()) {
        // DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // B recover M0
    unsigned char M0[PUF_SIZE];
    extractValueFromMap(msg,"M0",M0,PUF_SIZE);

    msg.clear();
        
    // B retrieve xA from memory and computes CA
    const unsigned char * xA = this->getUAVData("A")->getX();
    if (xA == nullptr){
        // DEBUG_PRINT("No challenge in memory for the requested UAV.\n");
        return 1;
    }
    // DEBUG_PRINT("xA : "); print_hex(xA, PUF_SIZE);
    
    unsigned char CA[PUF_SIZE];
    this->callPUF(xA,CA);
    // DEBUG_PRINT("CA : "); print_hex(CA, PUF_SIZE);
    
    unsigned char NA[PUF_SIZE];
    xor_buffers(M0, CA, PUF_SIZE, NA);
    // DEBUG_PRINT("NA : "); print_hex(NA, PUF_SIZE);

    // B then creates a nonce NB and the secret message M1 
    unsigned char gammaB[PUF_SIZE];
    unsigned char NB[PUF_SIZE];
    generate_random_bytes(gammaB, PUF_SIZE);
    this->callPUF(gammaB,NB);
    // DEBUG_PRINT("NB : "); print_hex(NB, PUF_SIZE);

    unsigned char M1[PUF_SIZE];
    const unsigned char * RA = this->getUAVData("A")->getR();
    if (RA == nullptr){
        // DEBUG_PRINT("No response in memory for the requested UAV.\n");
        return 1;
    }
    // DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);
    xor_buffers(RA,NA,PUF_SIZE,M1);
    xor_buffers(M1,NB,PUF_SIZE,M1);
    // DEBUG_PRINT("M1 : "); print_hex(M1, PUF_SIZE);

    // B sends its ID, M1 and a hash of CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    hash_state * ctx = initHash();
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    // DEBUG_PRINT("hash1 : "); print_hex(hash1, PUF_SIZE);
    
    socketModule.createMap(3);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M1", M1, PUF_SIZE);
    socketModule.addKeyValue("hash1", hash1, PUF_SIZE);

    this->socketModule.sendMsg();
    // DEBUG_PRINT("Sent ID, M1 and hash1.\n");

    msg.clear();

    // B waits for A response (M2)
    this->socketModule.receiveMsg(msg);
    while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        // DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }
    // B recovers M2 and hash2
    unsigned char M2[PUF_SIZE];
    extractValueFromMap(msg,"M2",M2,PUF_SIZE);
    
    unsigned char hash2[PUF_SIZE];
    extractValueFromMap(msg,"hash2",hash2,PUF_SIZE);
    
    msg.clear();
    
    // B retrieve RAp from M2
    unsigned char RAp[PUF_SIZE];
    xor_buffers(M2, NA, PUF_SIZE, RAp);
    // DEBUG_PRINT("RAp : "); print_hex(RAp, PUF_SIZE);
    
    // B verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2Check);
    
    int res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    // DEBUG_PRINT("B verify A's hash : %d\n",res);
    
    if(res == 0){
        // DEBUG_PRINT("The hashes do not correspond.\n");
        return 1;
    }
    
    // DEBUG_PRINT("A's hash has been verified. A is autenticated to B.\n");
    
    // B changes its values
    // this->getUAVData("A")->setX(gammaB);
    // this->getUAVData("A")->setR(RAp);
    
    // B sends a hash of RAp, NB, NA as an ACK
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    // DEBUG_PRINT("hash3 : "); print_hex(hash3, PUF_SIZE);
    
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("hash3", hash3, PUF_SIZE);
    
    this->socketModule.sendMsg();
    // Finished
    // DEBUG_PRINT("Sent ID and hash3.\n");
    // DEBUG_PRINT("\nThe two UAV autenticated each other.\n");
    
    return 0;
}

/// @brief Authenticate the UAV and establish a session key K.
/// @param none
/// @return 0 if success, 1 if failure
int UAV::autentication_key_server(){
    // The client initiate the autentication process
    DEBUG_PRINT("\nAutentication process begins.\n");
    #ifdef MEASUREMENTS_DETAILLED
        long long start;
        long long end;
        long long idlCycles = 0;
        long long opCycles = 0;
        CycleCounter counter;
        #endif
        
    // B receive the initial message
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(4);
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);
    
    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }
    
    // B recover M0
    unsigned char M0[PUF_SIZE];
    extractValueFromMap(msg,"M0",M0,PUF_SIZE);

    msg.clear();
    
    // B retrieve xA from memory and computes CA
    const unsigned char * xA = this->getUAVData("A")->getX();
    if (xA == nullptr){
        DEBUG_PRINT("No challenge in memory for the requested UAV.\n");
        return 1;
    }
    DEBUG_PRINT("xA : "); print_hex(xA, PUF_SIZE);
    
    unsigned char CA[PUF_SIZE];
    this->callPUF(xA,CA);
    DEBUG_PRINT("CA : "); print_hex(CA, PUF_SIZE);
    
    unsigned char NA[PUF_SIZE];
    xor_buffers(M0, CA, PUF_SIZE, NA);
    DEBUG_PRINT("NA : "); print_hex(NA, PUF_SIZE);

    // B then creates a nonce NB and the secret message M1 
    unsigned char gammaB[PUF_SIZE];
    unsigned char NB[PUF_SIZE];
    generate_random_bytes(gammaB, PUF_SIZE);
    this->callPUF(gammaB,NB);
    DEBUG_PRINT("NB : "); print_hex(NB, PUF_SIZE);

    unsigned char M1[PUF_SIZE];
    const unsigned char * RA = this->getUAVData("A")->getR();
    if (RA == nullptr){
        DEBUG_PRINT("No response in memory for the requested UAV.\n");
        return 1;
    }
    DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);
    xor_buffers(RA,NA,PUF_SIZE,M1);
    xor_buffers(M1,NB,PUF_SIZE,M1);
    DEBUG_PRINT("M1 : "); print_hex(M1, PUF_SIZE);

    // B sends its ID, M1 and a hash of CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    hash_state * ctx = initHash();
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    DEBUG_PRINT("hash1 : "); print_hex(hash1, PUF_SIZE);
    
    socketModule.createMap(3);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M1", M1, PUF_SIZE);
    socketModule.addKeyValue("hash1", hash1, PUF_SIZE);

    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID, M1 and hash1.\n");
    
    msg.clear();

    // B waits for A response (M2)
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);
    
    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // B recovers M2, MK and hash2
    unsigned char M2[PUF_SIZE];
    extractValueFromMap(msg,"M2",M2,PUF_SIZE);

    unsigned char MK[PUF_SIZE];
    extractValueFromMap(msg,"MK",MK,PUF_SIZE);
    
    unsigned char hash2[PUF_SIZE];
    extractValueFromMap(msg,"hash2",hash2,PUF_SIZE);

    msg.clear();

    // B retrieve RAp from M2
    unsigned char RAp[PUF_SIZE];
    xor_buffers(M2, NA, PUF_SIZE, RAp);
    DEBUG_PRINT("RAp : "); print_hex(RAp, PUF_SIZE);

    // B retrieve S from MK
    unsigned char S[PUF_SIZE];
    xor_buffers(MK, NA, PUF_SIZE, S);
    xor_buffers(S, NB, PUF_SIZE, S);
    DEBUG_PRINT("S : "); print_hex(S, PUF_SIZE);

    unsigned char K[PUF_SIZE];
    deriveKeyUsingHKDF(NA, NB, S, PUF_SIZE, K);
    DEBUG_PRINT("K : "); print_hex(K, PUF_SIZE);

    // B verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    addToHash(ctx, K, PUF_SIZE);
    calculateHash(ctx, hash2Check);
    DEBUG_PRINT("hash2Check : "); print_hex(hash2Check, PUF_SIZE);
    
    int res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    DEBUG_PRINT("B verify A's hash : %d\n", res);
    
    if(res == 0){
        DEBUG_PRINT("The hashes do not correspond.\n");
        return 1;
    }

    DEBUG_PRINT("A's hash has been verified. A is autenticated to B.\n");

    // B changes its values
    this->getUAVData("A")->setX(gammaB);
    this->getUAVData("A")->setR(RAp);

    // B sends a hash of RAp, K, NB, NA as an ACK
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, K, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    DEBUG_PRINT("hash3 : "); print_hex(hash3, PUF_SIZE);

    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("hash3", hash3, PUF_SIZE);

    this->socketModule.sendMsg();
    // Finished
    DEBUG_PRINT("Sent ID and hash3.\n");
    DEBUG_PRINT("\nThe two UAV autenticated each other.\n");
    
    return 0;
}

/// @brief Pre-enrolment function for initialize the authentication of UAV A
/// @param none
/// @return 0 if succeded, 1 if failed
int UAV::preEnrolment(){
    // A waits for BS's query
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(2);
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
    }

    unsigned char LC[CHALLENGE_SIZE][PUF_SIZE];
    extractValueFromMap(msg,"data",LC[0],CHALLENGE_SIZE*PUF_SIZE);

    // Generates the responses
    unsigned char LR[CHALLENGE_SIZE][PUF_SIZE];
    for (int i = 0; i < CHALLENGE_SIZE; i++) {
        this->callPUF(LC[i], LR[i]);
        DEBUG_PRINT("LR[%d]: ",i); print_hex(LR[i], PUF_SIZE);
    }

    // Send the responses back
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("data", (const unsigned char *)LR,  CHALLENGE_SIZE * PUF_SIZE);

    this->socketModule.sendMsg();

    return 0;
}

int UAV::preEnrolmentRetrival(){

    DEBUG_PRINT("\nC will now retrieve A's credentials.\n");

    // Wait for A's credentials
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(3);
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // Retrieve CA and RA from the msg
    unsigned char CA[PUF_SIZE];
    extractValueFromMap(msg,"CA",CA,PUF_SIZE);
    
    unsigned char RA[PUF_SIZE];
    extractValueFromMap(msg,"RA",RA,PUF_SIZE);

    DEBUG_PRINT("CA : "); print_hex(CA, PUF_SIZE);
    DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);

    // Conceals RA
    unsigned char xLock[PUF_SIZE];
    generate_random_bytes(xLock, PUF_SIZE);
    DEBUG_PRINT("xLock : "); print_hex(xLock, PUF_SIZE);

    unsigned char lock[PUF_SIZE];
    this->callPUF(xLock, lock);

    unsigned char secret[PUF_SIZE];
    xor_buffers(RA, lock, PUF_SIZE, secret);
    DEBUG_PRINT("secret : "); print_hex(secret, PUF_SIZE);


    this->addUAV("A", nullptr, CA, nullptr, xLock, secret);
    DEBUG_PRINT("\nC has retrieved A's credentials.\n");

    return 0;
}

/// @brief If the PreEnrolment doesnt fail, this comes to help
/// @param none
/// @return 0 if succeded, 1 if failed
int UAV::supplementaryAuthenticationInitial(){

    // Waits for C demands
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(3);    
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // A retrieve the id of the UAV trying to connect
    std::string idC = msg["id"];

    msg.clear();
    
    // and search it's table for a corresponding UAV 
    UAVData* data = this->getUAVData(idC);
    if (data) {
        DEBUG_PRINT("UAVData found! Not supposed to happen ?! Quit.\n");
        return 1;
    } 
    DEBUG_PRINT("UAVData not found for idC.\n");
    DEBUG_PRINT("Initiate supplementary authentication\n");
    
    // A generates an nonce NA
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA, PUF_SIZE);
    DEBUG_PRINT("NA : "); print_hex(NA, PUF_SIZE);

    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("NA", NA, PUF_SIZE);

    // A sends 
    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID and NA.\n");

    msg.clear();

    // Waits for C's response 
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // A recover M1, CA and the hash
    unsigned char CA[PUF_SIZE];
    extractValueFromMap(msg,"CA",CA,PUF_SIZE);

    unsigned char M1[PUF_SIZE];
    extractValueFromMap(msg,"M1",M1,PUF_SIZE);

    unsigned char hash1[PUF_SIZE];
    extractValueFromMap(msg,"hash1",hash1,PUF_SIZE);

    msg.clear();

    // A computes RA using transmitted CA
    unsigned char RA[PUF_SIZE];
    this->callPUF(CA, RA);
    DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);
    
    // A retrieve NC from M1 
    unsigned char NC[PUF_SIZE];
    xor_buffers(M1, RA, PUF_SIZE, NC);
    DEBUG_PRINT("NC : "); print_hex(NC, PUF_SIZE);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    hash_state * ctx = initHash();
    addToHash(ctx, idC);
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);
    DEBUG_PRINT("hash1Check : "); print_hex(hash1Check, PUF_SIZE);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    DEBUG_PRINT("A verify C's hash : %d\n",res);

    if(res == 0){
        DEBUG_PRINT("The autentication failed.\n");
        return res;
    }

    DEBUG_PRINT("C's hash has been verified. C is autenticated to A.\n");

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    this->callPUF(NC,RAp);
    DEBUG_PRINT("RAp : "); print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NC,RAp,PUF_SIZE,M2);
    DEBUG_PRINT("M2 : "); print_hex(M2, PUF_SIZE);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);

    // TODO : send M2, and a hash of NB, RA, RAp, NA

    socketModule.createMap(3);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M2", M2, PUF_SIZE);
    socketModule.addKeyValue("hash2", hash2, PUF_SIZE);

    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID, M2 and hash2.\n");

    msg.clear();

    // A waits for C's ACK
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");

        // A reach a timeout or didn't received the ACK 
        DEBUG_PRINT("Received an empty MsgPack message!\n");
    
        // A will save concealed current CA in CAOld
        unsigned char xLock[PUF_SIZE];
        generate_random_bytes(xLock, PUF_SIZE);
    
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
    
        unsigned char concealedCA[PUF_SIZE];
        xor_buffers(CA,lock,PUF_SIZE,concealedCA);
    
        this->getUAVData(idC)->setXLock(xLock);
        this->getUAVData(idC)->setSecret(concealedCA);

        // Then A saves the new challenge in CA
        this->addUAV(idC, nullptr, NC);

        return 1;
    }

    // Then A saves the new challenge in CA
    this->addUAV(idC, nullptr, NC);

    // Finished
    DEBUG_PRINT("\nThe two UAV autenticated each other.\n");

    return 0;
}

int UAV::supplementaryAuthenticationSup(){

    // C will now try to connect to A
    socketModule.createMap(1);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    
    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID.\n");
    
    // Wait for answer
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(4);
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // Retrieve NA 
    unsigned char NA[PUF_SIZE];
    extractValueFromMap(msg,"NA",NA,PUF_SIZE);

    msg.clear();

    // C retrieve CA from memory and recover RA
    const unsigned char * CA = this->getUAVData("A")->getC();
    if (CA == nullptr){
        DEBUG_PRINT("No challenge in memory for the requested UAV.\n");
        return 1;
    }
    DEBUG_PRINT("CA : "); print_hex(CA, PUF_SIZE);

    const unsigned char * xLock = this->getUAVData("A")->getXLock();
    DEBUG_PRINT("xLock : "); print_hex(xLock, PUF_SIZE);
    const unsigned char * secret = this->getUAVData("A")->getSecret();
    DEBUG_PRINT("secret : "); print_hex(secret, PUF_SIZE);
    unsigned char lock[PUF_SIZE];
    this->callPUF(xLock,lock);
    unsigned char RA[PUF_SIZE];
    xor_buffers(lock, secret, PUF_SIZE, RA);
    DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);

    // C then creates a nonce NC and the secret message M1 
    unsigned char gammaC[PUF_SIZE];
    unsigned char NC[PUF_SIZE];
    generate_random_bytes(gammaC, PUF_SIZE);
    this->callPUF(gammaC,NC);
    DEBUG_PRINT("NC : "); print_hex(NC, PUF_SIZE);

    unsigned char M1[PUF_SIZE];

    xor_buffers(RA,NC,PUF_SIZE,M1);
    DEBUG_PRINT("M1 : "); print_hex(M1, PUF_SIZE);

    // C sends its ID, M1 and a hash of idC, CA, NB, RA, NA to A
    unsigned char hash1[PUF_SIZE];
    hash_state * ctx = initHash();
    addToHash(ctx, this->getId());
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1);
    DEBUG_PRINT("hash1 : "); print_hex(hash1, PUF_SIZE);
    
    socketModule.createMap(4);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("CA", CA, PUF_SIZE);
    socketModule.addKeyValue("M1", M1, PUF_SIZE);
    socketModule.addKeyValue("hash1", hash1, PUF_SIZE);

    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID, CA, M1 and hash1.\n");

    msg.clear();

    // Wait for A's response 
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // B recovers M2 and hash2
    unsigned char M2[PUF_SIZE];
    extractValueFromMap(msg,"M2",M2,PUF_SIZE);

    unsigned char hash2[PUF_SIZE];
    extractValueFromMap(msg,"hash2",hash2,PUF_SIZE);

    msg.clear();

    // B retrieve RAp from M2
    unsigned char RAp[PUF_SIZE];
    xor_buffers(M2, NC, PUF_SIZE, RAp);
    DEBUG_PRINT("RAp : "); print_hex(RAp, PUF_SIZE);

    // B verify the hash
    unsigned char hash2Check[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2Check);

    int res = memcmp(hash2, hash2Check, PUF_SIZE) == 0;
    DEBUG_PRINT("C verify A's hash : %d\n",res);

    if(res == 0){
        DEBUG_PRINT("The hashes do not correspond.\n");
        return 1;
    }

    DEBUG_PRINT("A's hash has been verified. A is autenticated to C.\n");

    // B changes its values
    this->getUAVData("A")->setX(gammaC);
    this->getUAVData("A")->setR(RAp);

    // B sends a hash of RAp, NB, NA as an ACK
    unsigned char hash3[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NC, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash3);
    DEBUG_PRINT("hash3 : "); print_hex(hash3, PUF_SIZE);
    
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("hash3", hash3, PUF_SIZE);

    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID and hash3.\n");

    msg.clear();

    // Finished
    DEBUG_PRINT("\nThe two UAV autenticated each other.\n");

    return 0;
}

/// @brief The autentication from UAV A failed
/// @param none 
/// @return 0 if succeded, 1 if failed
int UAV::failed_autentication_client(){
    // The client initiate the authentication process
    DEBUG_PRINT("\nAutentication process begins.\n");

    // A generates a nonce NA 
    unsigned char NA[PUF_SIZE];
    generate_random_bytes(NA, PUF_SIZE);
    DEBUG_PRINT("NA : "); print_hex(NA, PUF_SIZE);

    const unsigned char * CA = this->getUAVData("B")->getC();
    if (CA == nullptr){
        DEBUG_PRINT("No expected challenge in memory for this UAV.\n");
        return 1;
    }
    
    unsigned char M0[PUF_SIZE];
    xor_buffers(NA,CA,PUF_SIZE,M0);
    DEBUG_PRINT("M0 : "); print_hex(M0, PUF_SIZE);

    // A sends its ID and NA to B 
    socketModule.createMap(2);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M0", M0, PUF_SIZE);
    
    this->socketModule.sendMsg();
    DEBUG_PRINT("Sent ID and M0.\n");
        
    // A waits for the answer
    std::unordered_map<std::string, std::string> msg;
    msg.reserve(3);
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }
    // DEBUG_PRINT(printMsg(msg);;);

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty!");
        return -1;
    }

    // A recover M1 and the hash
    unsigned char M1[PUF_SIZE];
    extractValueFromMap(msg,"M1",M1,PUF_SIZE);

    unsigned char hash1[PUF_SIZE];
    extractValueFromMap(msg,"hash1",hash1,PUF_SIZE);

    msg.clear();

    // A computes RA using CA in memory
    unsigned char RA[PUF_SIZE];
    this->callPUF(CA,RA);
    DEBUG_PRINT("RA : "); print_hex(RA, PUF_SIZE);
    
    // A retrieve NB from M1 
    unsigned char NB[PUF_SIZE];
    xor_buffers(M1, NA, PUF_SIZE, NB);
    xor_buffers(NB, RA, PUF_SIZE, NB);
    DEBUG_PRINT("NB : "); print_hex(NB, PUF_SIZE);

    // A verify the hash
    unsigned char hash1Check[PUF_SIZE];
    hash_state * ctx = initHash();
    addToHash(ctx, CA, PUF_SIZE);
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash1Check);
    DEBUG_PRINT("hash1Check : "); print_hex(hash1Check, PUF_SIZE);

    bool res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
    DEBUG_PRINT("A verify B's hash : %d\n" ,res); 

    if(res == 0){
        DEBUG_PRINT("The autentication failed. A will try to verify the hash with an old challenge if it exists.\n");

        // A will recover the old challenge 
        const unsigned char * xLock = this->getUAVData("B")->getXLock();
        if (xLock == nullptr){
            DEBUG_PRINT("No old challenge in memory for the requested UAV.\n");
            return 1;
        }
        const unsigned char * secret = this->getUAVData("B")->getSecret();
        if (secret == nullptr){
            DEBUG_PRINT("No old challenge in memory for the requested UAV.\n");
            return 1;
        }
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
        unsigned char CAOld[PUF_SIZE]; 
        xor_buffers(lock, secret, PUF_SIZE, CAOld);

        // A will calculate the Nonce A that the server calculated with the wrong CA 
        unsigned char NAOld[PUF_SIZE];
        xor_buffers(M0, CAOld, PUF_SIZE, NAOld);
        DEBUG_PRINT("NAOld : "); print_hex(NAOld, PUF_SIZE);

        // A will calculate the old response 
        unsigned char RAOld[PUF_SIZE];
        this->callPUF(CAOld, RAOld);
        DEBUG_PRINT("RAOld : "); print_hex(RAOld, PUF_SIZE);

        // A will deduce NB from the old response
        unsigned char NBOld[PUF_SIZE];
        xor_buffers(M1, RAOld, PUF_SIZE, NBOld);
        xor_buffers(NBOld, NAOld, PUF_SIZE, NBOld);
        DEBUG_PRINT("NBOld : "); print_hex(NBOld, PUF_SIZE);

        // A now tries to verify the hash with this value
        ctx = initHash();
        addToHash(ctx, CAOld, PUF_SIZE);
        addToHash(ctx, NBOld, PUF_SIZE);
        addToHash(ctx, RAOld, PUF_SIZE);
        addToHash(ctx, NAOld, PUF_SIZE);
        calculateHash(ctx, hash1Check);

        res = memcmp(hash1, hash1Check, PUF_SIZE) == 0;
        if (res == 0){
            DEBUG_PRINT("Even with the old challenge, autentication has failed.\n");
            return 1;
        }
        DEBUG_PRINT("B has been autenticated by A with the old challenge.\n");

        // A will now change the values to be the one obtained of the old challenge
        this->getUAVData("B")->setC(CAOld);
        memcpy(RA, RAOld, PUF_SIZE);
        memcpy(NB, NBOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);
        memcpy(NA, NAOld, PUF_SIZE);

    }

    DEBUG_PRINT("B's hash has been verified. B is autenticated to A.\n");

    // A will now conputes the new challenge and M2
    unsigned char RAp[PUF_SIZE];
    this->callPUF(NB,RAp);
    DEBUG_PRINT("RAp : "); print_hex(RAp, PUF_SIZE);

    unsigned char M2[PUF_SIZE];
    xor_buffers(NA,RAp,PUF_SIZE,M2);
    DEBUG_PRINT("M2 : "); print_hex(M2, PUF_SIZE);

    // A sends M2, and a hash of NB, RA, RAp, NA
    unsigned char hash2[PUF_SIZE];
    ctx = initHash();
    addToHash(ctx, NB, PUF_SIZE);
    addToHash(ctx, RA, PUF_SIZE);
    addToHash(ctx, RAp, PUF_SIZE);
    addToHash(ctx, NA, PUF_SIZE);
    calculateHash(ctx, hash2);

    // Send M2, and a hash of NB, RA, RAp, NA

    socketModule.createMap(3);
    socketModule.addKeyValue("id", (const unsigned char *)"A", 1);
    socketModule.addKeyValue("M2", M2, PUF_SIZE);
    socketModule.addKeyValue("hash2", hash2, PUF_SIZE);
  
    // ##################### MODIFICATION TO FAIL #####################
    
    // The message is not sent to B
    
    // this->socketModule.sendMsg();
    // DEBUG_PRINT("Sent ID, M2 and hash2.\n");
    
    // ################################################################

    // A waits for B's ACK
    this->socketModule.receiveMsg(msg);
while(msg.empty()){
    //   socketModule.resendLastPacket();
      vTaskDelay(M2T(10));
      socketModule.receiveMsg(msg);
    }

    // Check if an error occurred
    if (msg.empty()) {
        DEBUG_PRINT("Error occurred: content is empty\n");

        // A reach a timeout or didn't received the ACK 
        DEBUG_PRINT("Received an empty MsgPack message!\n");
    
        // A will save concealed current CA in CAOld
        unsigned char xLock[PUF_SIZE];
        generate_random_bytes(xLock, PUF_SIZE);
    
        unsigned char lock[PUF_SIZE];
        this->callPUF(xLock, lock);
    
        unsigned char concealedCA[PUF_SIZE];
        xor_buffers(CA,lock,PUF_SIZE,concealedCA);
    
        this->getUAVData("B")->setXLock(xLock);
        this->getUAVData("B")->setSecret(concealedCA);

        // Then A saves the new challenge in CA
        this->getUAVData("B")->setC(NB);

        return 1;
    }

    // Then A saves the new challenge in CA
    this->getUAVData("B")->setC(NB);

    // Finished
    DEBUG_PRINT("\nThe two UAV autenticated each other.\n");
    
    return 0;
}
