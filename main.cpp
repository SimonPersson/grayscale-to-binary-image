#include <cstdint>
#include <vector>
#include <algorithm>
#include <iterator>
#include <iostream>
#include <fstream>
#include <ctime>

void naive_iter(std::vector<uint8_t> &input, uint8_t thresh) {
	for(auto &it : input) {
		it = (it<thresh) ? 0 : 0xFF;
	}
}

void asm_algo(std::vector<uint8_t> &input, uint8_t thresh) {
	uint64_t iterations = input.size()/16;
	uint8_t mask[16];
	std::fill_n(mask, 16, thresh);
	uint8_t *mask_ptr = mask, *ptr = &input[0];
	asm(
			"mov %[iters], %%rax;"
			"mov %[ptr], %%rbx;"
			"mov %[mask], %%rcx;"
			"movapd (%%rcx), %%xmm0;"
			"loop:"
			"movapd (%%rbx), %%xmm1;"
			"pminub %%xmm0, %%xmm1;"
			"pcmpeqb %%xmm0, %%xmm1;"
			"movapd %%xmm1, (%%rbx);"
			"add $16, %%rbx;"
			"sub $1, %%eax;"
			"jnz loop;"
			:
			:
			[iters] "r"(iterations),
			[ptr] "r"(ptr),
			[mask] "r"(mask_ptr)
			:
			"%rax", "%rbx", "%rcx", "%xmm0", "%xmm1");
	for(auto it = input.begin() + iterations*16; it!=input.cend(); ++it) {
		*it = (*it<thresh) ? 0 : 0xFF;
	}
}

int main(int argc, char *argv[]) {
	const uint8_t thresh = 100;
	double duration;
	std::clock_t start;
	std::vector<uint8_t> inputData;

	if(argc == 2) {
		std::ifstream sourceFile(argv[1], std::ios::binary);
		inputData = std::vector<uint8_t>(std::istreambuf_iterator<char>(sourceFile), {});
	} else {
		std::cout << "Usage:\n\t" << argv[0] << " FILE" << std::endl;
		return 1;
	}


	std::vector<uint8_t> naiveInput(inputData);
	start = std::clock();
	naive_iter(naiveInput, thresh);
	duration = (std::clock() - start)/(double) CLOCKS_PER_SEC;
	std::cout << "Simple loop: " << duration << std::endl;
	std::ofstream naiveFile("out/naive_conversion.raw", std::ios::binary);
	std::copy(naiveInput.cbegin(),
		  naiveInput.cend(),
		  std::ostream_iterator<uint8_t>(naiveFile));

	std::vector<uint8_t> functionalInput(inputData);
	start = std::clock();
	std::for_each(functionalInput.begin(), functionalInput.end(),
			[](uint8_t &x) { x = (x<thresh) ? 0 : 0xFF; });
	duration = (std::clock() - start)/(double) CLOCKS_PER_SEC;
	std::cout << "std::for_each: " << duration << std::endl;
	std::ofstream functionalFile("out/functional_conversion.raw", std::ios::binary);
	std::copy(functionalInput.cbegin(),
		  functionalInput.cend(),
		  std::ostream_iterator<uint8_t>(functionalFile));

	std::vector<uint8_t> asmInput(inputData);
	start = std::clock();
	asm_algo(asmInput, thresh);
	duration = (std::clock() - start)/(double) CLOCKS_PER_SEC;
	std::cout << "asm_algo: " << duration << std::endl;
	std::ofstream asmFile("out/asm_conversion.raw", std::ios::binary);
	std::copy(asmInput.cbegin(),
		  asmInput.cend(),
		  std::ostream_iterator<uint8_t>(asmFile));

	return 0;
}
