#include <ld_rans/rans.hpp>
#include <iostream>
#include <format>

int main() {
    std::vector<uint8_t> input;
    for (int i = 0; i < 800; ++i) input.push_back(42);
    for (int i = 0; i < 100; ++i) input.push_back(i % 256);

    auto compressed = rans::encode(input);
    auto restored = rans::decode(compressed);
    
    std::cout << std::format("Original size: {} bytes\n", input.size());
    std::cout << std::format("Compressed size: {} bytes\n", compressed.size());
    std::cout << std::format("Restored size: {} bytes\n", restored.size());
    std::cout << std::format("Decode match: {} \n", (restored == input ? "YES" : "NO"));
}
