#ifndef PROTECTIONS_HPP
#define PROTECTIONS_HPP

#include <stdint.h>

#define PROT_MAX_ENCODER_DELTA      100.0f
#define PROT_COLLISION_TIMEOUT      50
#define PROT_STALL_TIMEOUT          300
#define PROT_MASTER_TIMEOUT_MS      1000

class Protections {
public:
    static bool checkEncoder(float delta);
    static bool checkOutOfRange(float pos, float min, float max);
    static bool checkMovement(float error);
    static void resetMovement();
    static bool checkCommunication(uint32_t lastMsg, uint32_t now);
};

#endif
