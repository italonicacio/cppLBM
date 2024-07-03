// #pragma once

#include <array>
#include <cinttypes>
#include <stdfloat>
#include <iostream>
#include <vector>
#include <numeric>


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

//em 89 std::float32_t não funciona quando tenho que definir template do array ...

constexpr auto pop_dir = GetPopDir<std::int8_t>();
constexpr auto pop_weights = GetPopWeights<float>();

constexpr std::float32_t cs2 = 1.0 / 3.0;
constexpr std::float32_t tau = 0.9;

constexpr std::array<std::uint32_t, dim> domain_size = {32, 32};

// o idx em zig é usize, que pode não ser necessariamente um uint64
constexpr std::array<std::uint32_t, dim> idx2pos(std::uint32_t idx) {
    if constexpr (dim == 2) {
        return {idx % domain_size[0], idx / domain_size[0]};
    } else {
        return { idx % domain_size[0], (idx / domain_size[0]) % domain_size[1], idx % (domain_size[0] * domain_size[1]) };
    }
}

template<typename T>
constexpr T pos2idx(std::array<T, dim> pos) {
    if constexpr (dim == 2) {
        return pos[0] + pos[1] * domain_size[0];
    } else {
        return pos[0] + domain_size[0] * (pos[1] + pos[2] * domain_size[1]);
    }
}

// template<typename T>
// T DotProd(T& x, T& y) {
    
//     return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<T>(0));
// }

template<typename T, typename V>
T DotProd(T& x, V& y) {
    
    return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<T>(0));
}

std::float32_t FuncFeq(std::float32_t rho, std::array<std::float32_t, dim>& u, std::uint32_t i) {
    const std::float32_t uc = DotProd(u, pop_dir[i]);
    const std::float32_t uu = DotProd(u, pop_dir[i]);

    return rho * pop_weights[i] * (1 + uc / cs2 + (uc * uc) / (2 * cs2 * cs2) - (uu) / (2 * cs2));

}

void Macroscopics (
    std::vector<std::array<float, n_pop>>& pop_arr,
    std::vector<float>& rho_arr,
    std::vector<std::array<float, dim>> u_arr
) {
    std::size_t i = 0;
    
    while(i < pop_arr.size()) {
        const auto pop = pop_arr[i];
        float rho = 0;
        for(auto& p : pop) {
            rho += p;
        }

        std::array<float, dim> u = {0};
        for(std::uint32_t j = 0; j < pop.size(); ++j) {
            auto p = pop[j];
            for(std::uint32_t d = 0; d < dim; ++d) {
                // duvida: pq em zig é u[d] += p * pop_dir[j]
                // sendo que pop_dir[j] é um array de 2 dimensões
                u[d] += p * pop_dir[j][d]; 
            }
            rho_arr[i] = rho;
            u_arr[i] = u;
        }
        ++i;
    }
}

void Collision(
    std::vector<std::array<float, n_pop>>& pop_arr,
    std::vector<float>& rho_arr,
    std::vector<std::array<float, dim>> u_arr
) {
    for(std::uint32_t idx = 0; idx < pop_arr.size(); ++idx) {
        const float rho = rho_arr[idx];
        auto u = u_arr[idx];
        const auto& pop = pop_arr[idx];

        for(std::uint32_t i = 0; i < pop.size(); ++i) {
            const float f = pop[i];
            const float feq = FuncFeq(rho, u, i);
            const float f_coll = (f - feq) / tau;
            pop_arr[idx][i] = f_coll;
        }
    }
}

void Streaming(    
    std::vector<std::array<float, n_pop>>& popA_arr,
    std::vector<std::array<float, n_pop>>& popB_arr
) {
    for(std::uint32_t idx; idx < popA_arr.size(); ++idx) {
        auto pos = idx2pos(idx);
        auto pop = popA_arr[idx];
        for(std::uint32_t i = 0; i < pop.size(); ++i) {
            std::float32_t p = pop[i];
            const std::array<std::int32_t, dim> pos_to = {
                (static_cast<std::int32_t>(pos[0]) + static_cast<std::int32_t>(pop_dir[0]) + domain_size[0]) % domain_size[0],
                (static_cast<std::int32_t>(pos[1]) + static_cast<std::int32_t>(pop_dir[1]) + domain_size[1]) % domain_size[1]
            };

            const std::uint32_t idx_to = pos2idx(pos_to);
            popB_arr[idx_to][i] = p;
            
        }
    }
}

void run_time_step(
    std::vector<std::array<float, n_pop>>& popA_arr,
    std::vector<std::array<float, n_pop>>& popB_arr,
    std::vector<float>& rho_arr,
    std::vector<std::array<float, dim>> u_arr,
    std::uint32_t time_step
) {
    std::vector<std::array<float, n_pop>> popMain_arr = (time_step % 2 == 0) ? popA_arr : popB_arr;
    std::vector<std::array<float, n_pop>> popAux_arr = (time_step % 2 == 0) ? popA_arr : popB_arr;

    Macroscopics(popMain_arr, rho_arr, u_arr);
    Collision(popMain_arr, rho_arr, u_arr);
    Streaming(popMain_arr, popAux_arr);
}

int32_t main() {
    
    std::cout << "Dim is:" << dim << std::endl;
    std::cout << "pop is:" << n_pop << std::endl;
    std::cout << "dim pop_dir: " << pop_dir.size() << std::endl;
    std::cout << "dim pop_dir 0:"  << pop_dir[0][0] << std::endl;

    std::cout << "dim pop_weights " << pop_weights.size() << std::endl;

    return 0;
}

