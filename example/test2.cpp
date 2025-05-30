#include <ld_rans/rans.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <format>


int main() {
  std::vector<uint8_t> input_8bit;
  for (int i = 0; i < 800; ++i) input_8bit.push_back(42);
  for (int i = 0; i < 100; ++i) input_8bit.push_back(i % 256);

  std::cout << std::format("Original size: {} bytes\n", input_8bit.size());
  std::cout << "Original 8bit data:\n";
  for (size_t i = 0; i < input_8bit.size(); ++i) {
    std::cout << std::format("{:02X} ", input_8bit[i]);
    if ((i + 1) % 8 == 0) std::cout << '\n';
  }
  std::cout << '\n';
  
  // rnsエンコード
  auto compressed = rans::encode(input_8bit);

  std::cout << std::format("Compressed size: {} bytes\n", compressed.size());
  std::cout << "Compressed data:\n";
  for (size_t i = 0; i < compressed.size(); ++i) {
    std::cout << std::format("{:02X} ", compressed[i]);
    if ((i + 1) % 16 == 0) std::cout << '\n';
  }
  std::cout << '\n';

  // rnsデコード
  auto decoded = rans::decode(compressed);

  std::cout << std::format("Restore size: {} bytes\n", decoded.size());
  std::cout << "Restore data:\n";
  for (size_t i = 0; i < decoded.size(); ++i) {
    std::cout << std::format("{:02X} ", decoded[i]);
    if ((i + 1) % 16 == 0) std::cout << '\n';
  }
  std::cout << '\n';

  // 検証
  bool match = (decoded == input_8bit);
  std::cout << std::format("Decode match: {}\n", match ? "YES" : "NO");
  double ratio = static_cast<double>(compressed.size()) / input_8bit.size();
  std::cout << std::format("Compression ratio: {:.2f}\n", ratio);

  return 0;
}

// 8bitに丸める（下位8bit抽出の簡易例）
/*
std::vector<uint8_t> input_8bit;
input_8bit.reserve(input_16bit.size());
for (uint16_t val : input_16bit) {
    input_8bit.push_back(static_cast<uint8_t>(val & 0xFF)); // 簡易量子化
}
*/
