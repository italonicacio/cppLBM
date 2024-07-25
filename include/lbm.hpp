#pragma once

#include <array>
#include <cinttypes>
#include <stdfloat>
#include <vector>
#include <numeric>
#include <iostream>

enum class VelSet { D2Q9, D3Q19 };


constexpr VelSet vel_set_use = VelSet::D2Q9;

template<bool>
struct always_false : std::false_type {};

template<VelSet V>
constexpr std::size_t GetDim() {

	if constexpr(V == VelSet::D2Q9) {
		return 2;
	} else if constexpr(V == VelSet::D3Q19) {
		return 3;
	} else {
		static_assert(always_false<true>::value, "Unsupported VelSet");
	}

	//Static assert não funciona com o if constexpr sem usar template
}

template<VelSet V>
constexpr std::size_t GetPop() {
	if constexpr(V == VelSet::D2Q9) {
		return 9;
	} else if constexpr(V == VelSet::D3Q19) {
		return 19;
	} else {
		static_assert(always_false<true>::value, "Unsupported VelSet");
	}
	//Static assert não funciona com o if constexpr sem usar template
}

constexpr std::size_t dim = GetDim<vel_set_use>();
constexpr std::size_t n_pop = GetPop<vel_set_use>();

// example with lambda
// constexpr auto pop_dir = [] {
//     if constexpr (vel_set_use == VelSet::D2Q9) {
//         return std::array<std::array<std::int32_t, dim>, n_pop> {{
//             {{0, 0}}, {{1, 0}}, {{0, 1}}, {{-1, 0}}, {{0, -1}},
//             {{1, 1}}, {{-1, 1}}, {{-1, -1}}, {{1, -1}}
//         }};
//     } 
// }();

// constexpr auto pop_weights = [] {
//     if constexpr (vel_set_use == VelSet::D2Q9) {
//         return std::array<float, n_pop>{
//             4.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
//             1.0f / 36.0f, 1.0f / 36.0f, 1.0f / 36.0f, 1.0f / 36.0f
//         };
//     }
// }();

template<typename T>
constexpr auto GetPopDir() {
	if constexpr(vel_set_use == VelSet::D2Q9) {
		return std::array<std::array<T, dim>, n_pop> {{
			{0, 0}, { 1, 0 }, { 0, 1 }, { -1, 0 }, { 0, -1 },
			{ 1, 1 }, { -1, 1 }, { -1, -1 }, { 1, -1 }
			}};
	}
}

template<typename T>
constexpr auto GetPopWeights() {
	if constexpr(vel_set_use == VelSet::D2Q9) {
		return std::array<T, n_pop>{
			4.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f, 1.0f / 9.0f,
				1.0f / 36.0f, 1.0f / 36.0f, 1.0f / 36.0f, 1.0f / 36.0f
		};
	}
}

//em 89 std::float32_t não funciona quando tenho que definir template do array ...

constexpr auto pop_dir = GetPopDir<std::int8_t>();
constexpr auto pop_weights = GetPopWeights<float>();

constexpr std::float32_t cs2 = 1.0 / 3.0;
constexpr std::float32_t tau = 0.9;

constexpr std::array<std::uint32_t, dim> domain_size = { 32, 32 };
constexpr std::uint32_t array_size = domain_size[0] * domain_size[1] * (dim == 2 ? 1 : domain_size[2]);

// o idx em zig é usize, que pode não ser necessariamente um uint64
// ESSA não funciona essa função, o compilador não consegue definir o valor de saida
// ai o dim é ajustado, mas o if não consegue fazer com que 
// constexpr std::array<std::uint32_t, dim> idx2pos(std::uint32_t idx) {

//     #if vel_set_use == VelSet::D2Q9
//         return {idx % domain_size[0], idx / domain_size[0]};
//     # else
//         return { idx % domain_size[0], (idx / domain_size[0]) % domain_size[1], idx / (domain_size[0] * domain_size[1]) };
//     #endif
// }


// esse pode retornar uint32_t, no zig ta retornando assim
constexpr auto idx2pos(std::uint32_t idx) {
	// constexpr std::int32_t converted_idx = static_cast<std::int32_t>(idx);
	if constexpr(dim == 2) {
		return std::array<std::int32_t, 2>{
			static_cast<std::int32_t>(idx% domain_size[0]),
				static_cast<std::int32_t>(idx / domain_size[0])
		};
	} else {
		return std::array<std::int32_t, 3>{
			static_cast<std::int32_t>(idx% domain_size[0]),
				static_cast<std::int32_t>((idx / domain_size[0]) % domain_size[1]),
				static_cast<std::int32_t>(idx / (domain_size[0] * domain_size[1]))
		};
	}
}

template<typename T>
constexpr T pos2idx(std::array<T, dim> pos) {
	if constexpr(dim == 2) {
		return pos[0] + pos[1] * domain_size[0];
	} else {
		return pos[0] + domain_size[0] * (pos[1] + pos[2] * domain_size[1]);
	}
}

inline std::uint32_t idxPop(std::array<std::int32_t, dim> pos, std::uint32_t i) {
	return (pos2idx(pos)) * n_pop + i;
}

// template<typename T>
// T DotProd(T& x, T& y) {

//     return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<T>(0));
// }


struct LBMArrays {
	std::vector<std::float32_t> popA;
	std::vector<std::float32_t> popB;
	std::vector<std::float32_t> ux;
	std::vector<std::float32_t> uy;
	std::vector<std::float32_t> rho;

	LBMArrays() :
		popA(array_size* n_pop),
		popB(array_size* n_pop),
		ux(array_size),
		uy(array_size),
		rho(array_size) {
	}

	void Initialize();

	void ExportArrays(std::uint32_t time_step);
};


void RunTimeStep(LBMArrays& lbm_array, std::uint32_t time_step);

LBMArrays RunSimulation(const std::uint32_t max_steps = 100);
