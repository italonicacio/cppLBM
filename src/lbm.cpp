#include <cmath>
#include <algorithm>

#include "lbm.hpp"
#include "vtk.hpp"
#include "utils.hpp"


void Macroscopics(
	std::vector<std::float32_t>& pop_arr,
	std::vector<std::float32_t>& rho_arr,
	std::vector<std::float32_t>& ux_arr,
	std::vector<std::float32_t>& uy_arr
) {
	for(std::size_t idx = 0; idx < n_nodes; ++idx) {

		const auto pos = idx2pos(idx);
		std::array<std::float32_t, n_pop> pop;
		for(std::size_t j = 0; j < n_pop; ++j) {
			pop[j] = pop_arr[idxPop(pos, j)];
		}

		std::float32_t rho = 0;
		for(auto& p : pop) {
			rho += p;
		}

		std::array<std::float32_t, dim> u{};
		for(std::uint32_t j = 0; j < n_pop; ++j) {
			for(std::uint32_t d = 0; d < dim; ++d) {
				const std::float32_t fdir = static_cast<std::float32_t>(pop_dir[j][d]);
				u[d] += pop[j] * fdir / rho;
			}

		}

		rho_arr[idx] = rho;
		ux_arr[idx] = u[0];
		uy_arr[idx] = u[1];


	}

}

void Collision(
	std::vector<std::float32_t>& pop_arr,
	std::vector<std::float32_t>& rho_arr,
	std::vector<std::float32_t>& ux_arr,
	std::vector<std::float32_t>& uy_arr

) {
	for(std::uint32_t idx = 0; idx < n_nodes; ++idx) {

		const auto pos = idx2pos(idx);
		const std::float32_t rho = rho_arr[idx];
		const std::float32_t ux = ux_arr[idx];
		const std::float32_t uy = uy_arr[idx];
		const std::array<std::float32_t, dim> u = { ux, uy };
		std::array<std::float32_t, n_pop> pop;
		for(std::size_t j = 0; j < n_pop; ++j) {
			pop[j] = pop_arr[idxPop(pos, j)];
		}

		for(std::uint32_t i = 0; i < n_pop; ++i) {

			const std::float32_t feq = FuncFeq(rho, u, i);
			const std::float32_t f_coll = pop[i] - (pop[i] - feq) / tau;
			pop_arr[idxPop(pos, i)] = f_coll;
		}
	}

}

void Streaming(
	std::vector<std::float32_t>& popA_arr,
	std::vector<std::float32_t>& popB_arr
) {
	for(std::uint32_t idx = 0; idx < n_nodes; ++idx) {
		auto pos = idx2pos(idx);
		for(std::uint32_t i = 0; i < n_pop; ++i) {
			const std::array<std::int32_t, dim> local_pop_dir{ pop_dir[i][0], pop_dir[i][1] };
			std::array<std::int32_t, dim> pos_to{ pos[0], pos[1] };


			for(std::uint32_t d = 0; d < dim; ++d) {
				pos_to[d] += local_pop_dir[d];
				if(pos_to[d] < 0) {
					pos_to[d] += domain_size[d];
				} else if(pos_to[d] >= domain_size[d]) {
					pos_to[d] -= domain_size[d];
				}
			}

			const std::array<std::uint32_t, dim> pos_to_u{ pos_to[0], pos_to[1] };

			popB_arr[idxPop(pos_to_u, i)] = popA_arr[idxPop(pos, i)];

		}
	}
}



void LBMArrays::Initialize() {
	for(std::uint32_t idx = 0; idx < n_nodes; ++idx) {
		const auto pos = idx2pos(idx);

		rho[idx] = 1;

		std::array<std::float32_t, dim> posF{ static_cast<std::float32_t>(pos[0]), static_cast<std::float32_t>(pos[1]) };
		std::array<std::float32_t, dim> posNorm{ posF[0] / domain_size[0], posF[1] / domain_size[1] };


		const std::float32_t vel_norm = 0.01;
		const std::float32_t two_pi = 2.0f32 * std::numbers::pi;
		const std::float32_t _ux = vel_norm * std::sin(posNorm[0] * two_pi) * std::cos(posNorm[1] * two_pi);
		const std::float32_t _uy = -vel_norm * std::cos(posNorm[0] * two_pi) * std::sin(posNorm[1] * two_pi);

		ux[idx] = _ux;
		uy[idx] = _uy;
		// ux[idx] = 0.01 * ((((domain_size[1] - 1) - posF[1]) * posF[1]) / (domain_size[1] - 1));
		// uy[idx] = 0;

		const std::array<std::float32_t, dim> u = { ux[idx], uy[idx] };
		for(std::uint32_t j = 0; j < n_pop; ++j) {
			popA[idxPop(pos, j)] = FuncFeq(rho[idx], u, j);
			popB[idxPop(pos, j)] = FuncFeq(rho[idx], u, j);

		}
	}

}

void LBMArrays::ExportArrays(std::uint32_t time_step) {

	std::string output_dir = "../output";

	std::vector<std::uint32_t> domain(domain_size.begin(), domain_size.end());

#pragma GCC unroll 3
	for(const auto& [name, member_ptr] : this->member_map) {
		const std::string filename_use = std::format("{}/{}{:0>5}.vtk", output_dir, name, time_step);
		auto data_write = export_array(member_ptr, domain);
		WriteArrayListToFile(filename_use, data_write);
	}

	// std::ranges::for_each(this->member_map.cbegin(), this->member_map.cend(),
	// 	[&](const auto& pair) {
	// 		const auto& [name, member_ptr] = pair;
	// 		const std::string filename_use = std::format("{}/{}{:0>5}.vtk", output_dir, name, time_step);
	// 		auto data_write = export_array(member_ptr, domain);
	// 		WriteArrayListToFile(filename_use, data_write);
	// 	}
	// );


	// std::string rho_filename = std::format("{}/rho{:0>5}.vtk", output_dir, time_step);
	// std::string ux_filename = std::format("{}/ux{:0>5}.vtk", output_dir, time_step);
	// std::string uy_filename = std::format("{}/uy{:0>5}.vtk", output_dir, time_step);


	// auto rho_string_vtk = export_array(this->rho, domain);
	// WriteArrayListToFile(rho_filename, rho_string_vtk);

	// auto ux_string_vtk = export_array(this->ux, domain);
	// WriteArrayListToFile(ux_filename, ux_string_vtk);

	// auto uy_string_vtk = export_array(this->uy, domain);
	// WriteArrayListToFile(uy_filename, uy_string_vtk);
}


void RunTimeStep(
	LBMArrays& lbm_array,
	std::uint32_t time_step
) {
	std::vector<std::float32_t>& popMain_arr = (time_step % 2 == 0) ? lbm_array.popA : lbm_array.popB;
	std::vector<std::float32_t>& popAux_arr = (time_step % 2 == 1) ? lbm_array.popA : lbm_array.popB;


	Macroscopics(popMain_arr, lbm_array.rho, lbm_array.ux, lbm_array.uy);
	Collision(popMain_arr, lbm_array.rho, lbm_array.ux, lbm_array.uy);
	Streaming(popMain_arr, popAux_arr);
}

LBMArrays RunSimulation(const std::uint32_t max_steps, const std::uint32_t export_interval) {
	LBMArrays lbm_arrays;
	lbm_arrays.Initialize();

	lbm_arrays.ExportArrays(0);

	for(std::uint32_t step = 1; step < max_steps; ++step) {
		std::cout << "Running time step " << step << " ...\n";
		std::cout << std::format("rho {:.6f} ux {:.6f} uy {:.6f} ...", lbm_arrays.rho[0], lbm_arrays.ux[0], lbm_arrays.uy[0]) << std::endl;

		RunTimeStep(lbm_arrays, step);

		if(step % export_interval == 0) {
			lbm_arrays.ExportArrays(step);
			std::cout << std::format("Exporting Arrays in step {}\n", step);
		}
	}

	return lbm_arrays;
}
