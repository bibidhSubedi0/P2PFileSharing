
#include "../include/utils.h"
#include <chrono>
#include <format>

void logger::Logger::log(std::string message) {
    auto now = std::chrono::system_clock::now();
    std::string timestamp = std::format("{:%Y-%m-%d %H:%M:%S}", now);
    std::cout << "[" << timestamp << "] " << message << std::endl;
}

std::string GlobalUID::UIDGenerator::generate_uid() {
	static boost::uuids::random_generator generator;
	boost::uuids::uuid uuid = generator();
	return to_string(uuid);
}