#ifndef FLAG_UTILS_HPP
#define FLAG_UTILS_HPP

#include <stdint.h>
#include <stdbool.h>

#define COMMUNICATION_OK_FLAG 0
#define ERROR_FLAG 1
#define ACTION_MOVE_FLAG 2
#define ACTION_STOP_FLAG 3
#define ACTION_IDLE_FLAG 4
#define ACTION_RESUME_FLAG 5

inline volatile uint32_t flagBits = 0;

inline void flagSet(uint8_t flag) {
    if (flag < 32)
        flagBits |= (1UL << flag);
}
inline void flagClear(uint8_t flag) {
    if (flag < 32)
        flagBits &= ~(1UL << flag);
}
inline bool flagGet(uint8_t flag) {
    if (flag >= 32) return false;
    return (flagBits >> flag) & 1UL;
}
inline void flagClearAll() { flagBits = 0; }

#endif
