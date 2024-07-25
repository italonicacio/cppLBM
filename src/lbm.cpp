#include "lbm.hpp"
#include "vtk.hpp"
#include "utils.hpp"


std::float32_t DotProd(const std::array<std::float32_t, dim>& x, const std::array<std::int8_t, dim>& y) {

	return std::inner_product(x.begin(), x.end(), y.begin(), static_cast<std::float32_t>(0));
}

std::float32_t FuncFeq(std::float32_t rho, std::array<std::float32_t, dim>& u, std::uint32_t i) {
	const std::float32_t uc = DotProd(u, pop_dir[i]);
	const std::float32_t uu = DotProd(u, pop_dir[i]);

	return rho * pop_weights[i] * (1 + uc / cs2 + (uc * uc) / (2 * cs2 * cs2) - (uu) / (2 * cs2));

}

void Macroscopics(
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

		std::array<std::float32_t, dim> u{ 0 };
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
		const auto pos = idx2pos(idx);
		const std::float32_t rho = rho_arr[idx];
		auto ux = ux_arr[idx];
		auto uy = uy_arr[idx];
		std::array<std::float32_t, dim> u = { ux, uy };
		std::array<std::float32_t, n_pop> pop;
		for(std::size_t j = 0; j < n_pop; ++j) {
			pop[j] = pop_arr[idxPop(pos, j)];
		}

		for(std::uint32_t i = 0; i < pop.size(); ++i) {
			const std::float32_t f = pop[i];
			const std::float32_t feq = FuncFeq(rho, u, i);
			const std::float32_t f_coll = f + (f - feq) / tau;
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



void LBMArrays::Initialize() {
	for(std::int32_t idx = 0; idx < array_size; ++idx) {
		auto pos = idx2pos(idx);
		ux[idx] = 0;
		uy[idx] = 0;
		rho[idx] = 1;

		for(std::uint32_t pop = 0; pop < n_pop; ++pop) {
			popA[idxPop(pos, pop)] = pop_weights[pop];
			popB[idxPop(pos, pop)] = pop_weights[pop];
		}
	}
}

void LBMArrays::ExportArrays(std::uint32_t time_step) {



	std::string rho_filename = std::format("rho{:0>5}.vtk", time_step);
	std::string ux_filename = std::format("ux{:0>5}.vtk", time_step);
	std::string uy_filename = std::format("uy{:0>5}.vtk", time_step);

	std::vector<std::uint32_t> domain(domain_size.begin(), domain_size.end());

	auto rho_string_vtk = export_array(this->rho, domain);
	WriteArrayListToFile(rho_filename, rho_string_vtk);

	auto ux_string_vtk = export_array(this->ux, domain);
	WriteArrayListToFile(ux_filename, ux_string_vtk);

	auto uy_string_vtk = export_array(this->uy, domain);
	WriteArrayListToFile(uy_filename, uy_string_vtk);
}


void RunTimeStep(
	LBMArrays& lbm_array,
	std::uint32_t time_step
) {
	std::vector<std::float32_t> popMain_arr = (time_step % 2 == 0) ? lbm_array.popA : lbm_array.popB;
	std::vector<std::float32_t> popAux_arr = (time_step % 2 == 0) ? lbm_array.popA : lbm_array.popB;

	Macroscopics(popMain_arr, lbm_array.rho, lbm_array.ux, lbm_array.uy);
	Collision(popMain_arr, lbm_array.rho, lbm_array.ux, lbm_array.uy);
	Streaming(popMain_arr, popAux_arr);
}

LBMArrays RunSimulation(const std::uint32_t max_steps) {
	LBMArrays lbm_arrays;
	lbm_arrays.Initialize();


	for(std::uint32_t step = 0; step < max_steps; ++step) {
		std::cout << "Running time step " << step << " ..." << std::endl;
		std::cout << std::format("rho {} ux {} uy {} ...", lbm_arrays.rho[0], lbm_arrays.ux[0], lbm_arrays.uy[0]) << std::endl;
		RunTimeStep(lbm_arrays, step);
	}

	return lbm_arrays;
}
