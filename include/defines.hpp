#pragma once

#include <array>
#include <cinttypes>
#include <stdfloat>
#include <vector>
#include <numeric>
#include <iostream>


constexpr std::float32_t cs2 = 1.0 / 3.0;

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


// Parameters
constexpr std::float32_t tau = 0.9;
constexpr std::array<std::uint32_t, dim> domain_size = { 32, 32 };
constexpr std::uint32_t n_nodes = domain_size[0] * domain_size[1] * (dim == 2 ? 1 : domain_size[2]);
