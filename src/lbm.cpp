// #pragma once

#include <array>
#include <cinttypes>
#include <stdfloat>
#include <iostream>
#include <vector>
#include <numeric>
#include <chrono>


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

constexpr auto idx2pos(std::uint32_t idx) {
    // constexpr std::int32_t converted_idx = static_cast<std::int32_t>(idx);
    if constexpr (dim == 2) {
        return std::array<std::int32_t, 2>{
            static_cast<std::int32_t>(idx % domain_size[0]), 
            static_cast<std::int32_t>(idx / domain_size[0])
        };
    } else {
        return std::array<std::int32_t, 3>{ 
            static_cast<std::int32_t>(idx % domain_size[0]), 
            static_cast<std::int32_t>((idx / domain_size[0]) % domain_size[1]), 
            static_cast<std::int32_t>(idx / (domain_size[0] * domain_size[1])) 
        };
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

std::uint32_t idxPop(std::array<std::int32_t, dim> pos, std::uint32_t i) {
    return (pos2idx(pos)) * i;
}

// template<typename T>
// T DotProd(T& x, T& y) {
    
//     return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<T>(0));
// }


std::float32_t DotProd(const std::array<std::float32_t, dim>& x,  const std::array<std::int8_t, dim>& y) {

    return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<std::float32_t>(0));
}

std::float32_t FuncFeq(std::float32_t rho, std::array<std::float32_t, dim>& u, std::uint32_t i) {
    const std::float32_t uc = DotProd(u, pop_dir[i]);
    const std::float32_t uu = DotProd(u, pop_dir[i]);

    return rho * pop_weights[i] * (1 + uc / cs2 + (uc * uc) / (2 * cs2 * cs2) - (uu) / (2 * cs2));

}

void Macroscopics (
    std::vector<std::float32_t>& pop_arr,
    std::vector<std::float32_t>& rho_arr,
    std::vector<std::float32_t>& ux_arr,
    std::vector<std::float32_t>& uy_arr
) {
    
    for(std::size_t idx = 0; idx < array_size; ++idx) {
        const auto pos = idx2pos(idx);
        std::array<std::float32_t, n_pop> pop;
        for(std::size_t j = 0; j < n_pop; ++j) {
            pop[j] = pop_arr[idxPop(pos, j)];
        }

        std::float32_t rho = 0;
        for(auto& p : pop) {
            rho += p;
        }

        std::array<std::float32_t, dim> u{0};
        for(std::uint32_t j = 0; j < n_pop; ++j) {
            for(std::uint32_t d = 0; d < dim; ++d) {

                u[d] += pop[j] * pop_dir[j][d] / rho; 
            }
            rho_arr[idx] = rho;
            ux_arr[idx] = u[0];
            uy_arr[idx] = u[1];

        }
    }
}

void Collision(
    std::vector<std::float32_t>& pop_arr,
    std::vector<std::float32_t>& rho_arr,
    std::vector<std::float32_t>& ux_arr,
    std::vector<std::float32_t>& uy_arr

) {
    for(std::uint32_t idx = 0; idx < array_size; ++idx) {
        const std::float32_t rho = rho_arr[idx];
        auto ux = ux_arr[idx];
        auto uy = uy_arr[idx];
        std::array<std::float32_t, dim> u = {ux, uy};
        const auto pos = idx2pos(idx);
        std::array<std::float32_t, n_pop> pop;
        for(std::size_t j = 0; j < n_pop; ++j) {
            pop[j] = pop_arr[idxPop(pos, j)];
        }

        for(std::uint32_t i = 0; i < pop.size(); ++i) {
            const std::float32_t f = pop[i];
            const std::float32_t feq = FuncFeq(rho, u, i);
            const std::float32_t f_coll = (f - feq) / tau;
            pop_arr[idxPop(pos, i)] = f_coll;
        }
    }
}

void Streaming(    
    std::vector<std::float32_t>& popA_arr,
    std::vector<std::float32_t>& popB_arr
) {
    for(std::uint32_t idx; idx < array_size; ++idx) {
        auto pos = idx2pos(idx);
        for(std::uint32_t i = 0; i < n_pop; ++i) {
            const std::array<std::int32_t, dim> pos_to = {
                (static_cast<std::int32_t>(pos[0]) + static_cast<std::int32_t>(pop_dir[i][0]) + domain_size[0]) % domain_size[0],
                (static_cast<std::int32_t>(pos[1]) + static_cast<std::int32_t>(pop_dir[i][1]) + domain_size[1]) % domain_size[1]
            };

            const std::uint32_t idx_to = pos2idx(pos_to);
            popB_arr[idxPop(pos_to, i)] = popA_arr[idxPop(pos, i)];
            
        }
    }
}

struct LBMArrays {
    std::vector<std::float32_t> popA;
    std::vector<std::float32_t> popB;
    std::vector<std::float32_t> ux;
    std::vector<std::float32_t> uy;
    std::vector<std::float32_t> rho;
    
    LBMArrays() : 
        popA(array_size * n_pop),
        popB(array_size * n_pop),
        ux(array_size),
        uy(array_size),
        rho(array_size)
    {}
};

void run_time_step(
    LBMArrays& lbm_array,
    std::uint32_t time_step
) {
    std::vector<std::float32_t> popMain_arr = (time_step % 2 == 0) ? lbm_array.popA : lbm_array.popB;
    std::vector<std::float32_t> popAux_arr = (time_step % 2 == 0) ? lbm_array.popA : lbm_array.popB;

    Macroscopics(popMain_arr, lbm_array.rho, lbm_array.ux, lbm_array.uy);
    Collision(popMain_arr, lbm_array.rho, lbm_array.ux, lbm_array.uy);
    Streaming(popMain_arr, popAux_arr);
}

void RunSimulation() {
    LBMArrays lbm_arrays;
    const std::uint32_t max_steps = 1000;

    for(std::uint32_t step = 0; step < max_steps; ++step) {
        std::cout << "Running time step " << step << " ..." << std::endl; 

        run_time_step(lbm_arrays, step);
    }
}

using ClockT = std::chrono::high_resolution_clock;

std::string TimeConverter(const ClockT::duration& elapsedTime)
{
    std::chrono::hours hr = std::chrono::duration_cast<std::chrono::hours>(elapsedTime);
    std::chrono::minutes min = std::chrono::duration_cast<std::chrono::minutes>(elapsedTime - hr);
    std::chrono::seconds sec = std::chrono::duration_cast<std::chrono::seconds>(elapsedTime - hr - min);
    std::chrono::milliseconds ms = std::chrono::duration_cast<std::chrono::milliseconds>(elapsedTime - hr - min - sec);

    std::string time =  (hr.count() < 1 ? "" : std::to_string(hr.count()) + "hr ")      // Hour
                        + (min.count() < 1 ? "" : std::to_string(min.count()) + "min ") // Minutes
                        + (sec.count() < 1 ? "" : std::to_string(sec.count()) + "sec ") // Seconds
                        + std::to_string(ms.count()) + "ms";                            // Milliseconds
    
    return time;
}


int32_t main() {
    
    std::cout << "Start simulation!" << std::endl;
    ClockT::time_point starTime = ClockT::now();
    
    RunSimulation(); 
    ClockT::time_point endTime = ClockT::now();

    ClockT::duration elapsedTime = endTime - starTime;
    const std::string time = TimeConverter(elapsedTime);
    std::cout << "Finished simulation in " << time  << "!"<< std::endl;

    // std::cout << "Dim is:" << dim << std::endl;
    // std::cout << "pop is:" << n_pop << std::endl;
    // std::cout << "dim pop_dir: " << pop_dir.size() << std::endl;
    // std::cout << "dim pop_dir 0:"  << pop_dir[0][0] << std::endl;

    // std::cout << "dim pop_weights " << pop_weights.size() << std::endl;



    return 0;
}

