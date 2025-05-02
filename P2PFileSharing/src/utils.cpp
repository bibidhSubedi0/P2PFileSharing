
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


Peers::PeerInfo::PeerInfo(std::string ip, unsigned short p, std::string username, const std::vector<std::string>& files)
    :ip_address(ip), port(p),username(username), shared_files(files) {
    last_active = std::chrono::system_clock::now();
}

int FileHandling::makeBinaryChunks(const std::string& filename, std::size_t chunkSize)
{
    {
        std::ifstream file(filename, std::ios::binary);
        if (!file) throw std::runtime_error("Cannot open input file.");

        int index = 0;
        while (!file.eof()) {
            std::vector<char> buffer(chunkSize);
            file.read(buffer.data(), chunkSize);
            std::streamsize bytesRead = file.gcount();

            if (bytesRead == 0) break; // nothing read

            std::string chunkName = "chunk_" + std::to_string(index++);
            std::ofstream out(chunkName, std::ios::binary);
            if (!out) throw std::runtime_error("Cannot create chunk file.");

            out.write(buffer.data(), bytesRead);
        }

        return index;
    }
}

void FileHandling::reconstructFile(const std::string& outputFilename, int chunkCount)
{
    std::ofstream out(outputFilename, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot open output file.");

    for (int i = 0; i < chunkCount; i++) {
        std::string chunkName = "chunk_" + std::to_string(i);
        std::ifstream chunk(chunkName, std::ios::binary);
        if (!chunk) throw std::runtime_error("Cannot open chunk: " + chunkName);

        std::vector<char> buffer((std::istreambuf_iterator<char>(chunk)),
            std::istreambuf_iterator<char>());
        out.write(buffer.data(), buffer.size());
    }
}
