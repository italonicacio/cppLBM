#pragma once

#include "defines.hpp"


// esse pode retornar uint32_t, no zig ta retornando assim
constexpr auto idx2pos(std::uint32_t idx) {
	// constexpr std::int32_t converted_idx = static_cast<std::int32_t>(idx);
	if constexpr(dim == 2) {
		return std::array<std::uint32_t, 2>{
			static_cast<std::uint32_t>(idx% domain_size[0]),
				static_cast<std::uint32_t>(idx / domain_size[0])
		};
	} else {
		return std::array<std::uint32_t, 3>{
			static_cast<std::uint32_t>(idx% domain_size[0]),
				static_cast<std::uint32_t>((idx / domain_size[0]) % domain_size[1]),
				static_cast<std::uint32_t>(idx / (domain_size[0] * domain_size[1]))
		};
	}
}


constexpr std::uint32_t pos2idx(std::array<std::uint32_t, dim> pos) {
	if constexpr(dim == 2) {
		return pos[0] + pos[1] * domain_size[0];
	} else {
		return pos[0] + domain_size[0] * (pos[1] + pos[2] * domain_size[1]);
	}
}

inline std::uint32_t idxPop(std::array<std::uint32_t, dim> pos, std::uint32_t i) {
	return i + n_pop * (pos[0] + pos[1] * domain_size[0]);
}
