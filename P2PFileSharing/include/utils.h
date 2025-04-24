#pragma once
#include<iostream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>


namespace logger
{
	class Logger {
		
	public:
		void log(std::string message);
	};
}

namespace GlobalUID
{
	class UIDGenerator
	{
	public:
		std::string generate_uid();
	};
}

namespace Peers
{
	struct PeerInfo {
		std::string ip_address;
		unsigned short port; // FIXED type
		std::chrono::system_clock::time_point last_active;
		std::vector<std::string> shared_files;


		PeerInfo() = default;

		PeerInfo(const std::string& ip, unsigned short p, const std::vector<std::string>& files)
			: ip_address(ip), port(p), shared_files(files) {
			last_active = std::chrono::system_clock::now();
		}
	};
}

