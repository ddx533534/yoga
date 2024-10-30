#include <iostream>
#include <unordered_map>

namespace facebook::yoga {
// 定义颜色枚举类型
enum class Color { RED, GREEN, YELLOW, BLUE, PURPLE, RESET };
void logWithColor(const std::string& message, Color color);

void log(const std::string& message);
} // namespace facebook::yoga