#pragma once

#define MMO_ASSERT(condition, message) \
    do { \
        if (!(condition)) { \
            /* Add actual logging/abort here later */ \
        } \
    } while (false)
