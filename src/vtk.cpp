#include <array>
#include <format>
#include <bit>
#include <iostream>
#include <iomanip>
#include <bitset>


#include "vtk.hpp"
#include "utils.hpp"




std::vector<std::uint8_t> export_array(
	std::vector<std::float32_t>& arr,
	std::vector<std::uint32_t>& dims
) {
	const std::array<std::uint32_t, 3> dims_use = { dims[0], dims[1], dims.size() < 3 ? 1 : dims[2] };
	std::vector<std::uint8_t> vtk_string;
	vtk_string.reserve(1024 * dims.size());
	const std::string header = "# vtk DataFile Version 3.0\nData\nBINARY\nDATASET STRUCTURED_POINTS\n";
	EmplaceSlice(vtk_string, header);

	EmplaceSlice(vtk_string, "DIMENSIONS ");
	for(std::uint32_t d : dims_use) {
		EmplaceSlice(vtk_string, std::format("{} ", d));
	}
	EmplaceSlice(vtk_string, "\n");


	EmplaceSlice(vtk_string, "ORIGIN ");
	for(std::uint32_t _ : dims_use) {
		EmplaceSlice(vtk_string, std::format("{} ", 0));
	}
	EmplaceSlice(vtk_string, "\n");



	EmplaceSlice(vtk_string, "SPACING ");
	for(std::uint32_t _ : dims_use) {
		EmplaceSlice(vtk_string, std::format("{} ", 1));
	}
	EmplaceSlice(vtk_string, "\n");

	EmplaceSlice(vtk_string, std::format("POINT_DATA {}\n", dims_use[0] * dims_use[1] * dims_use[2]));

	EmplaceSlice(vtk_string, std::string("SCALARS data float 1\n"));

	EmplaceSlice(vtk_string, std::string("LOOKUP_TABLE default\n"));



	std::array<std::uint8_t, 4> be_scalar;
	for(std::float32_t value : arr) {
		std::uint32_t be_value = std::bit_cast<std::int32_t>(value);

		if constexpr(IsBigEndian()) {
			be_scalar = std::bit_cast<std::array<std::uint8_t, 4>>(be_value);
		} else {

			std::uint32_t be_rev = std::byteswap<std::uint32_t>(be_value);;
			be_scalar = std::bit_cast<std::array<std::uint8_t, 4>>(be_rev);
		}

		std::array<std::uint8_t, 4> be_use = be_scalar;

		for(std::uint8_t e : be_use) {
			vtk_string.emplace_back(e);
		}
	}
	return std::move(vtk_string);
}



