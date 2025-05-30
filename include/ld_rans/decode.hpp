#pragma once
#include <vector>
#include <unordered_map>
#include <cstdint>
#include <cassert>
#include "ryg_rans/rans_byte.h"

namespace rans {
inline std::vector<uint8_t> decode(const std::vector<uint8_t>& compressed) {
    constexpr int PROB_BITS = 12;
    constexpr int PROB_SCALE = 1 << PROB_BITS;

    uint8_t* ptr = const_cast<uint8_t*>(compressed.data());
    uint16_t original_size = ptr[0] | (ptr[1] << 8);
    ptr += 2;

    uint8_t symbol_count = *ptr++;
    std::vector<std::pair<uint8_t, uint32_t>> freq;
    for (int i = 0; i < symbol_count; ++i) {
        uint8_t symbol = *ptr++;
        uint16_t f = ptr[0] | (ptr[1] << 8);
        ptr += 2;
        freq.emplace_back(symbol, f);
    }

    std::unordered_map<uint8_t, RansDecSymbol> symbols;
    std::vector<uint8_t> lut(PROB_SCALE);
    uint32_t cum = 0;
    for (auto& [symbol, f] : freq) {
        RansDecSymbol s;
        RansDecSymbolInit(&s, cum, f);
        for (uint32_t i = cum; i < cum + f; ++i) {
            lut[i] = symbol;
        }
        symbols[symbol] = s;
        cum += f;
    }

    RansState rans;
    RansDecInit(&rans, &ptr);

    std::vector<uint8_t> output;
    output.reserve(original_size);
    for (size_t i = 0; i < original_size; ++i) {
        uint32_t s = rans & (PROB_SCALE - 1);
        uint8_t symbol = lut[s];
        output.push_back(symbol);
        RansDecAdvanceSymbol(&rans, &ptr, &symbols[symbol], PROB_BITS);
    }

    return output;
}
}
