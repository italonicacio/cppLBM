// #pragma once

#include <array>
#include <cinttypes>
#include <stdfloat>
#include <iostream>


enum class VelSet {D2Q9, D3Q19};


constexpr VelSet vel_set_use = VelSet::D2Q9;

template<bool>
struct always_false : std::false_type {};

template<VelSet V>
constexpr std::size_t GetDim() {

    if constexpr (V == VelSet::D2Q9) {
        return 2;
    } else if constexpr (V == VelSet::D3Q19) {
        return 3;
    } else {
        static_assert(always_false<true>::value, "Unsupported VelSet");
    }

    //Static assert não funciona com o if constexpr sem usar template
}

template<VelSet V>
constexpr std::size_t GetPop() {
    if constexpr (V == VelSet::D2Q9) {
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
            {0, 0}, {1, 0}, {0, 1}, {-1, 0}, {0, -1},
            {1, 1}, {-1, 1}, {-1, -1}, {1, -1}
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

//em 89 std::float32_t não funciona

constexpr auto pop_dir = GetPopDir<std::int8_t>();
constexpr auto pop_weights = GetPopWeights<float>();

int32_t main() {
    
    std::cout << "Dim is:" << dim << std::endl;
    std::cout << "pop is:" << n_pop << std::endl;
    std::cout << "dim pop_dir: " << pop_dir.size() << std::endl;
    std::cout << "dim pop_dir 0:"  << pop_dir[0][0] << std::endl;

    std::cout << "dim pop_weights " << pop_weights.size() << std::endl;

    return 0;
}

