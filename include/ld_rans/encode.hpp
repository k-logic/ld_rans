#pragma once
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <sys/mman.h>
#include <stdexcept> 
#include <memory>
#include "external/ryg_rans/rans_byte.h"

namespace rans {
inline std::vector<uint8_t> encode(const std::vector<uint8_t>& input) {
    constexpr int PROB_BITS = 12;
    constexpr int PROB_SCALE = 1 << PROB_BITS;
    constexpr size_t BUF_SIZE = 8096;

    // ソートされた順番の頻度リストの作成(処理重め)
    std::unordered_map<uint8_t, uint32_t> freq_map;
    for (uint8_t b : input) freq_map[b]++;
    std::vector<std::pair<uint8_t, uint32_t>> freq(freq_map.begin(), freq_map.end());
    std::sort(freq.begin(), freq.end());

    // モデルの初期化処理(累積頻度の構築)
    std::unordered_map<uint8_t, RansEncSymbol> symbols;
    uint32_t cum = 0;
    for (auto& [symbol, f] : freq) {
        RansEncSymbol sym;
        RansEncSymbolInit(&sym, cum, f, PROB_BITS);
        symbols[symbol] = sym;
        cum += f;
    }

    assert(cum <= PROB_SCALE);

    // mmap によるメモリ確保
    char* raw_buf = (char*)mmap(nullptr, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (raw_buf == MAP_FAILED) {
        throw std::runtime_error("mmap failed");
    }
    auto buf_sptr = std::shared_ptr<uint8_t>((uint8_t*)raw_buf, [=](uint8_t* p) { munmap(p, BUF_SIZE); });
    
    // rANS エンコード
    uint8_t* ptr = buf_sptr.get() + BUF_SIZE;
    RansState rans;
    RansEncInit(&rans);
    for (auto it = input.rbegin(); it != input.rend(); ++it) {
        RansEncPutSymbol(&rans, &ptr, &symbols[*it]);
    }
    RansEncFlush(&rans, &ptr);
    std::vector<uint8_t> compressed(ptr, buf_sptr.get() + BUF_SIZE);

    // ヘッダー構築
    std::vector<uint8_t> result;
    uint16_t original_size = static_cast<uint16_t>(input.size());
    result.push_back(original_size & 0xFF);
    result.push_back((original_size >> 8) & 0xFF);

    uint8_t symbol_count = static_cast<uint8_t>(freq.size());
    result.push_back(symbol_count);
    for (const auto& [symbol, f] : freq) {
        result.push_back(symbol);
        result.push_back(f & 0xFF);
        result.push_back((f >> 8) & 0xFF);
    }

    result.insert(result.end(), compressed.begin(), compressed.end());
    return result;
}
}
