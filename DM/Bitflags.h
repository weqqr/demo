#pragma once

#include <DM/Traits.h>

namespace DM {
#define DM_BITFLAGS(BitFlags)                                                    \
    static inline BitFlags operator|(BitFlags lhs, BitFlags rhs)                 \
    {                                                                            \
        using U = UnderlyingType<BitFlags>;                                      \
        return static_cast<BitFlags>(static_cast<U>(lhs) | static_cast<U>(rhs)); \
    }                                                                            \
                                                                                 \
    static inline BitFlags operator&(BitFlags lhs, BitFlags rhs)                 \
    {                                                                            \
        using U = UnderlyingType<BitFlags>;                                      \
        return static_cast<BitFlags>(static_cast<U>(lhs) & static_cast<U>(rhs)); \
    }                                                                            \
                                                                                 \
    static inline bool operator==(BitFlags lhs, BitFlags rhs)                    \
    {                                                                            \
        using U = UnderlyingType<BitFlags>;                                      \
        return static_cast<U>(lhs) == static_cast<U>(rhs);                       \
    }                                                                            \
                                                                                 \
    static inline bool has_flags(BitFlags what, BitFlags flags)                  \
    {                                                                            \
        return (what & flags) == flags;                                          \
    }
}
