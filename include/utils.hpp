#pragma once

#include <algorithm>
#include <fstream>
#include <bit>
#include <chrono>

using ClockT = std::chrono::high_resolution_clock;


constexpr bool IsBigEndian() {
	if constexpr(std::endian::native == std::endian::big)
		return true;
	else if constexpr(std::endian::native == std::endian::little)
		return false;

}

inline std::string TimeConverter(const ClockT::duration& elapsedTime) {
	std::chrono::hours hr = std::chrono::duration_cast<std::chrono::hours>(elapsedTime);
	std::chrono::minutes min = std::chrono::duration_cast<std::chrono::minutes>(elapsedTime - hr);
	std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(elapsedTime - hr - min);
	std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime - hr - min - sec);

	std::string time = (hr.count() < 1 ? "" : std::to_string(hr.count()) + "hr ")      // Hour
		+ (min.count() < 1 ? "" : std::to_string(min.count()) + "min ") // Minutes
		+ (sec.count() < 1 ? "" : std::to_string(sec.count()) + "sec ") // Seconds
		+ std::to_string(ms.count()) + "ms";                            // Milliseconds

	return time;
}

inline void WriteArrayListToFile(const std::string& filename, const std::vector<std::uint8_t>& list) {
	// Abre o arquivo em modo binário
	std::ofstream file(filename, std::ios::binary);

	// Verifica se o arquivo foi aberto com sucesso
	if(!file.is_open()) {
		throw std::ios_base::failure("Falha ao abrir o arquivo");
	}

	// Escreve os dados do vetor no arquivo
	file.write(reinterpret_cast<const char*>(list.data()), list.size());

	// Fecha o arquivo
	file.close();
}

inline void EmplaceSlice(std::vector<std::uint8_t>& string, const std::string& s) {
	for(char c : s) {
		string.emplace_back(static_cast<std::uint8_t>(c));
	}
}
