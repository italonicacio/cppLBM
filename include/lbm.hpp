#pragma once

#include <map>
#include <memory>

#include "defines.hpp"
#include "idx.hpp"

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


// inline std::float32_t DotProd(const std::array<std::float32_t, dim>& x, const std::array<std::int8_t, dim>& y) {

// 	return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<std::float32_t>(0));
// }

inline std::float32_t DotProd(const std::array<std::float32_t, dim>& x, const std::array<std::float32_t, dim>& y) {

	return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<std::float32_t>(0));
}

inline std::float32_t FuncFeq(const std::float32_t rho, const std::array<std::float32_t, dim>& u, std::uint32_t i) {
	std::array<std::float32_t, dim> local_pop_dir = { pop_dir[i][0], pop_dir[i][1] };
	const std::float32_t uc = DotProd(u, local_pop_dir);
	const std::float32_t uu = DotProd(u, u);

	return rho * pop_weights[i] * (1 + uc / cs2 + (uc * uc) / (2 * cs2 * cs2) - (uu) / (2 * cs2));

}

// template<typename T>
// T DotProd(T& x, T& y) {

//     return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<T>(0));
// }

struct MemberMap {
	std::string name{};
	std::vector<std::float32_t>& member_ptr;
};

struct LBMArrays {
	std::vector<std::float32_t> popA;
	std::vector<std::float32_t> popB;
	std::vector<std::float32_t> ux;
	std::vector<std::float32_t> uy;
	std::vector<std::float32_t> rho;
	std::array<MemberMap, 3> member_map;

	LBMArrays() :
		popA(n_nodes* n_pop),
		popB(n_nodes* n_pop),
		ux(n_nodes),
		uy(n_nodes),
		rho(n_nodes),
		member_map{
			MemberMap{"rho", rho},
			MemberMap{"ux", ux},
			MemberMap{"uy", uy}
		} {


	}

	void Initialize();

	void ExportArrays(std::uint32_t time_step);
};


LBMArrays RunSimulation(const std::uint32_t max_steps, const std::uint32_t export_interval);

void RunTimeStep(LBMArrays& lbm_array, std::uint32_t time_step);
