#pragma once
#include <string>
#include "BitSync/skCrypter.h"
#pragma comment(lib, "BitSync.lib")

namespace BitSync {

	void init_ctx(std::string&& id);

	std::tuple<int, std::string, const std::string*, const std::string*, bool> license_ctx(std::string license);
	std::string filestream(std::string license);
	std::string log(std::string msg);
}

std::string appid = skCrypt("2ECEF1EB0769BA4067C001B7122A67686EB9B3833946C9FD2214665D3983DC74C87DF077C5BFB8E2298ED98042834E0C").decrypt();
