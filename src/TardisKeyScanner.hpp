//
// Created by Robert Jacobson on 10/22/19.
//

#pragma once

#ifdef ARDUINO_ARM_TARDIS

#include "Tardis.hpp"
#include "TardisKeyScannerDescription.hpp"
#include "TardisLedDriver.hpp"
#include "AtomicBitfield.hpp"
#include "kaleidoscope/driver/BaseKeyScanner.h"

namespace kaleidoscope::hardware::empros {

typedef AtomicBitfield<256> keydata_t;


    // region Tardis Coord Table

    /*
     * In the followig table:
     *      -- if the index interpreted as the key index, then `table[index][1]` gives
     *         the encoded hardware address.
     *      -- if the index interpreted as the encoded hardware address, then
     *         `table[index][2]` gives the key index.
     *
     */
    #define TARDIS_COORD_TABLE                                                                                         \
        {                                                                                                              \
            {248, 250}, {240, 225}, {232, 200}, {224, 175}, {216, 150}, {208, 234}, {200, 209}, {192, 184},            \
                {184, 249}, {176, 224}, {168, 199}, {160, 174}, {152, 149}, {144, 159}, {136, 134}, {164, 126},        \
                {128, 248}, {237, 223}, {221, 198}, {205, 173}, {189, 148}, {173, 233}, {157, 208}, {133, 183},        \
                {138, 247}, {249, 222}, {241, 197}, {233, 172}, {225, 147}, {217, 158}, {209, 133}, {201, 151},        \
                {193, 246}, {185, 221}, {177, 196}, {169, 171}, {161, 146}, {153, 232}, {145, 207}, {137, 182},        \
                {129, 245}, {238, 220}, {222, 195}, {206, 170}, {190, 145}, {174, 157}, {158, 132}, {147, 176},        \
                {139, 244}, {250, 219}, {242, 194}, {234, 169}, {226, 144}, {218, 231}, {210, 206}, {202, 181},        \
                {194, 243}, {186, 218}, {178, 193}, {170, 168}, {162, 143}, {154, 156}, {146, 131}, {130, 201},        \
                {239, 242}, {223, 217}, {207, 192}, {191, 167}, {175, 142}, {159, 230}, {148, 205}, {140, 180},        \
                {251, 241}, {243, 216}, {235, 191}, {227, 166}, {219, 141}, {211, 155}, {203, 130}, {195, 226},        \
                {187, 240}, {179, 215}, {171, 190}, {163, 165}, {155, 140}, {131, 229}, {245, 204}, {229, 179},        \
                {213, 239}, {197, 214}, {181, 189}, {165, 164}, {149, 139}, {141, 154}, {252, 129}, {244, 238},        \
                {236, 213}, {228, 188}, {220, 163}, {212, 138}, {204, 228}, {196, 203}, {188, 178}, {180, 237},        \
                {172, 212}, {254, 187}, {255, 162}, {132, 137}, {246, 153}, {230, 128}, {214, 236}, {198, 211},        \
                {182, 186}, {166, 161}, {150, 136}, {142, 227}, {253, 202}, {215, 177}, {156, 235}, {247, 210},        \
                {231, 185}, {199, 160}, {183, 135}, {167, 152}, {151, 127}, {143, 16}, {15, 40}, {126, 63}, {110, 85}, \
                {94, 107}, {78, 23}, {62, 14}, {46, 39}, {30, 24}, {14, 48}, {124, 71}, {116, 93}, {108, 115},         \
                {100, 125}, {92, 13}, {84, 38}, {76, 62}, {68, 47}, {60, 70}, {52, 92}, {44, 114}, {36, 124},          \
                {28, 12}, {20, 37}, {12, 61}, {4, 84}, {31, 118}, {125, 22}, {109, 46}, {93, 69}, {77, 11}, {61, 36},  \
                {45, 60}, {29, 83}, {13, 15}, {123, 91}, {115, 113}, {107, 123}, {99, 10}, {91, 35}, {83, 59},         \
                {75, 82}, {67, 104}, {59, 21}, {51, 45}, {43, 68}, {35, 9}, {27, 34}, {19, 58}, {11, 81}, {3, 103},    \
                {47, 90}, {119, 112}, {103, 122}, {87, 8}, {71, 33}, {55, 57}, {39, 80}, {23, 102}, {7, 20},           \
                {122, 44}, {114, 67}, {106, 7}, {98, 32}, {90, 56}, {82, 79}, {74, 101}, {66, 89}, {58, 111},          \
                {50, 121}, {42, 6}, {34, 31}, {26, 55}, {18, 78}, {10, 100}, {2, 19}, {63, 43}, {118, 66}, {102, 5},   \
                {86, 30}, {70, 54}, {54, 77}, {38, 99}, {22, 88}, {6, 110}, {121, 117}, {113, 4}, {105, 29}, {97, 53}, \
                {89, 76}, {81, 98}, {73, 18}, {65, 42}, {57, 65}, {49, 3}, {41, 28}, {33, 52}, {25, 75}, {17, 97},     \
                {9, 87}, {1, 109}, {79, 120}, {117, 2}, {101, 27}, {85, 51}, {69, 74}, {53, 96}, {37, 17}, {21, 41},   \
                {5, 64}, {120, 1}, {112, 26}, {104, 50}, {96, 73}, {88, 95}, {80, 86}, {72, 108}, {64, 119}, {56, 0},  \
                {48, 25}, {40, 49}, {32, 72}, {24, 94}, {16, 116}, {8, 105}, {                                         \
                0, 106                                                                                                 \
            }                                                                                                          \
        }
// endregion


class TardisKeyScanner : public kaleidoscope::driver::BaseKeyScanner<TardisKeyScannerDescription> {
    friend class Tardis;
public:

    static void scanMatrix(){
        Tardis::readMatrix();
        actOnMatrixScan();
    }
    static void readMatrix(){
        Tardis::readMatrix();
    };

    // ToDo(algernon): What actions are supposed to be taken?
    static void actOnMatrixScan(){}

    // Masking a key out means that any other event than a release will be ignored until said
    // release.
    static void maskKey(KeyAddr key_addr){
        keyMask_[coord_to_index_(key_addr)] = 1;
    }
    static void unMaskKey(KeyAddr key_addr){
        keyMask_[coord_to_index_(key_addr)] = 0;
    }
    static bool isKeyMasked(KeyAddr key_addr){
        return keyMask_[coord_to_index_(key_addr)];
    }

    // Is the set of "held" keys determined by the key state (`isKeyswitchPressed`)?
    // static void maskHeldKeys();

    // Is the key at the given position currently pressed?
    static bool isKeyswitchPressed(KeyAddr key_addr){
        return keyState_[coord_to_index_(key_addr)];
    }

    static uint8_t pressedKeyswitchCount(){
        return static_cast<uint8_t>(keyState_.countOnes());
    }

    // Was the key at the given position pressed on the previous scan?
    static bool wasKeyswitchPressed(KeyAddr key_addr){
        return prevKeyState_[coord_to_index_(key_addr)];
    }
    // The number of key switches pressed in the previous scan.
    static uint8_t previousPressedKeyswitchCount(){
        return static_cast<uint8_t>(prevKeyState_.countOnes());
    }

    static void setKeyscanInterval(uint8_t interval){
        keyScanFrequency = interval;
    }
    static uint8_t getKeyscanInterval(){
        return keyScanFrequency;
    }

protected:

    /****************************
     * Key addressing functions *
     ****************************/

    // See the comment introducing `TARDIS_COORD_TABLE` above for the technical details.
    constexpr static const uint8_t coord_table_[251][2]TARDIS_COORD_TABLE;
    // Array index to physical circuit address.
    static KeyAddr index_to_key_coord(uint8_t index){
        uint8_t encoded_key_coord = coord_table_[index][0];
        // Lower 3 bits are row, upper 5 are column.
        return KeyAddr(encoded_key_coord & 7U, encoded_key_coord >> 3U);
    }
    // Physical circuit address to to array index.
    static uint8_t coord_to_index_(KeyAddr coord){
        return coord_table_[coord.row() + (coord.col() << 3U)][1];
    }
    // Same as `coord_to_index` except the coordinates are encoded.
    static uint8_t encoded_coord_to_index_(uint8_t coord){
        return coord_table_[coord][1];
    }

    static void setKeyState(uint8_t row, uint8_t col, unsigned int val = 1){
        // The coordinates of (row, col) are in physical circuit address coordinates.
        TardisKeyScanner::keyState_[
            TardisKeyScanner::encoded_coord_to_index_(
                TardisKeyScanner::coord_to_index_(
                    KeyAddr((row - 1) % 8, (col - 1) % 16)
                )
            )
        ] = val;
    }

    // ToDo: Why is this not keyScanFrequency?
    static uint8_t keyScanInterval;
    static uint8_t keyScanFrequency;
    static keydata_t prevKeyState_;
    static keydata_t keyState_;
    static keydata_t keyMask_;

};

}

#endif
