#pragma once

#include <vector>
#include <string>
#include <stdfloat>
#include <cinttypes>

std::vector<std::uint8_t> export_array(
	std::vector<std::float32_t>& arr,
	std::vector<std::uint32_t>& dims
);


