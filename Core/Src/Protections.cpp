#include "Protections.hpp"
#include "FlagUtils.hpp"
#include "CommunicationHandler.hpp"
#include <math.h>

static float s_lastError = 0.0f;
static int s_counter = 0;

bool Protections::checkEncoder(float delta) {
    if (flagGet(ERROR_FLAG))
        return false;
    if (fabs(delta) > PROT_MAX_ENCODER_DELTA) {
        setErrorCode(ENCODER_ERROR);
        flagSet(ERROR_FLAG);
        return true;
    }
    return false;
}

bool Protections::checkOutOfRange(float pos, float min, float max) {
    if (flagGet(ERROR_FLAG))
        return false;
    if (pos < min || pos > max) {
        setErrorCode(OUT_OF_RANGE_ERROR);
        flagSet(ERROR_FLAG);
        return true;
    }
    return false;
}

bool Protections::checkMovement(float error) {
    if (flagGet(ERROR_FLAG))
        return false;

    if (fabs(error) >= s_lastError)
        s_counter++;
    else
        s_counter = 0;

    if (s_counter >= PROT_COLLISION_TIMEOUT) {
        setErrorCode(COLISION_ERROR);
        flagSet(ERROR_FLAG);
        s_lastError = 0.0f;
        s_counter = 0;
        return true;
    }

    if (s_counter >= PROT_STALL_TIMEOUT) {
        setErrorCode(MOVE_FAILURE_ERROR);
        flagSet(ERROR_FLAG);
        s_lastError = 0.0f;
        s_counter = 0;
        return true;
    }

    s_lastError = fabs(error);
    return false;
}

void Protections::resetMovement() {
    s_lastError = 0.0f;
    s_counter = 0;
}

bool Protections::checkCommunication(uint32_t lastMsg, uint32_t now) {
    if (flagGet(ERROR_FLAG))
        return false;
    if (now - lastMsg > PROT_MASTER_TIMEOUT_MS) {
        setErrorCode(COMMUNICATION_LOST_ERROR);
        flagSet(ERROR_FLAG);
        return true;
    }
    return false;
}
