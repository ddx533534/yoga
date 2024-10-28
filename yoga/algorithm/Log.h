#include <iostream>
#include <unordered_map>

namespace facebook::yoga {
// 定义颜色枚举类型
enum class Color { RED, GREEN, YELLOW, BLUE, RESET };
void logWithColor(const std::string& message, Color color);
} // namespace facebook::yoga