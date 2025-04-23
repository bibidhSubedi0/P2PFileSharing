#include "../include/utils.h"
#include <regex>

bool test::is_valid_ip(const std::string& ip) {
    const std::regex ipv4_pattern(
        R"(^(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)"
        R"(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)"
        R"(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)"
        R"(\.(25[0-5]|2[0-4]\d|1\d\d|[1-9]?\d)$)"
    );
    return std::regex_match(ip, ipv4_pattern);
}
