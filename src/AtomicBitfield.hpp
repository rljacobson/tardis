/**
 * @brief Minimalist bit field implementation using STM32 bit band aliases.
 *
 * Bit band operations are both faster than read-modify-write instructions and are atomic.
 * Atomicity means they are interrupt safe. This assumes the data is only accessed in a bit band
 * region, i.e. in SRAM.
 *
 * WARNING: As with arrays, no bounds checking is performed.
 *
 */

//
// Created by Robert Jacobson on 10/23/19.
//

#pragma once

#include <cstdint>

#include "imo.h"

#ifdef __ARM__

// ToDo(algernon): What namespace should this be in? The code is valid for any architecture with
//  a bit band. One only need define `BB_ALIAS` and/or `PERIPH_BB_ALIAS` which compute the bit
//  band aliasing address as in `Cartex_M.hpp`.

// Atomic Bitfield sounds like the name of a punk rock band.
template<size_t BitCount__>
class AtomicBitfield{
public:
    AtomicBitfield(){
        bb_data_alias_ = BB_ALIAS(data_, 0);
    }
    
    // Gives how many bytes are required to store `n_bits` number of bits.
    static constexpr size_t nBytesForBits(size_t n_bits) {
        return (n_bits % 8) ? n_bits / 8 + 1 : n_bits / 8;
    }
    
    static constexpr size_t n_bits_ = BitCount__;
    static constexpr size_t n_bytes_ = nBytesForBits(BitCount__);
    
    uint32_t &operator[](u_int32_t index) {
        return bb_data_alias_[index];
    }
    
    // Does what it says on the tin. This is sometimes called the string's Hamming weight.
    uint32_t countOnes(){
        uint32_t *bit_ptr = bb_data_alias_;
        uint32_t count = 0;
        uint32_t Remaining = n_bits_;
    
        while (Remaining--)
            count += *(bit_ptr++);
        return count;
    }
    

private:
    uint8_t data_[n_bytes_] = {0};
    uint32_t bb_data_alias_;
};

#endif // __ARM__
