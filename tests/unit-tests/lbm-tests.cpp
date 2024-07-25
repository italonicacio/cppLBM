#include <gtest/gtest.h>



TEST(SumTest, SumTest) {
	int a = 1;
	int b = 2;
	int expected = 3;
	EXPECT_EQ((a + b), expected);
}


// void test_fun_feq() {
// 	std::float32_t rho = 1;
// 	std::array<std::float32_t, dim> u{ 0,0 };

// 	for(std::size_t i = 0; i < n_pop; ++i) {
// 		std::float32_t feq = FuncFeq(rho, u, i);
// 		if(!(feq == pop_weights[i])) {
// 			throw std::runtime_error("error");
// 		}
// 	}

// }

// void test_idx_pop() {
// 	std::uint32_t count = 0;
// 	for(std::size_t idx = 0; idx < array_size; ++idx) {
// 		auto pos = idx2pos(idx);
// 		auto retIdx = pos2idx(pos);
// 		if(!(idx == retIdx)) {
// 			throw std::runtime_error("error idx_pop 1");
// 		}

// 		for(std::uint32_t d = 0; d < dim; ++d) {
// 			if(!(pos[d] >= 0)) {
// 				throw std::runtime_error("error idx_pop 3");
// 			}

// 			if(!(pos[d] < domain_size[d])) {
// 				throw std::runtime_error("error idx_pop 3");
// 			}
// 		}


// 		for(std::size_t i = 0; i < n_pop; ++i) {
// 			auto popIdx = idxPop(pos, i);
// 			if(!(count == popIdx)) {
// 				throw std::runtime_error("error idx_pop 2");
// 			}
// 			count += 1;
// 		}
// 	}
// }

// void test() {
// 	test_fun_feq();
// 	test_idx_pop();

// }
