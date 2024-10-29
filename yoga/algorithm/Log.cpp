#include "yoga/algorithm/Log.h"
#include <iostream>

namespace facebook::yoga {
// 创建颜色映射
const std::unordered_map<Color, std::string> colorMap = {
    {Color::RED, "\033[31m"},
    {Color::GREEN, "\033[32m"},
    {Color::YELLOW, "\033[33m"},
    {Color::BLUE, "\033[34m"},
    {Color::RESET, "\033[0m"},
    {Color::PURPLE, "\033[35m"}};
void logWithColor(const std::string& message, Color color) {
  std::cout << colorMap.at(color) << message << "\n" << "\033[0m" << std::endl;
}
} // namespace facebook::yoga