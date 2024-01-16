#include <immintrin.h> // For AVX2 and AVX512f intrinsics
#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include <chrono> // Include chrono for timing
#include <cstdint>

size_t calc_intersize12_avx2(const uint64_t* e1_usigs, const uint64_t* e2_usigs, size_t sketchsize64, size_t bbits) {
    size_t samebits = 0;

    for (size_t i = 0; i < sketchsize64; ++i) {
        __m256i common_bits_accumulated = _mm256_set1_epi64x(~0ULL);
        const uint64_t* ptr1 = e1_usigs + i * bbits;
        const uint64_t* ptr2 = e2_usigs + i * bbits;
        const uint64_t* end1 = ptr1 + bbits;

        while (ptr1 + 4 <= end1) {
            __m256i e1_val = _mm256_loadu_si256((__m256i*)ptr1);
            __m256i e2_val = _mm256_loadu_si256((__m256i*)ptr2);
            __m256i xored = _mm256_xor_si256(e1_val, e2_val);
            common_bits_accumulated = _mm256_andnot_si256(xored, common_bits_accumulated);

            ptr1 += 4;
            ptr2 += 4;
        }

        uint64_t aggregated_result = ~0ULL;
        aggregated_result &= _mm256_extract_epi64(common_bits_accumulated, 0);
        aggregated_result &= _mm256_extract_epi64(common_bits_accumulated, 1);
        aggregated_result &= _mm256_extract_epi64(common_bits_accumulated, 2);
        aggregated_result &= _mm256_extract_epi64(common_bits_accumulated, 3);

        // Handle remaining elements with scalar operations
        while (ptr1 < end1) {
            aggregated_result &= ~(*ptr1++ ^ *ptr2++);
        }

        samebits += __builtin_popcountll(aggregated_result);
    }

    return samebits;
}


size_t calc_intersize12_avx512(const uint64_t* e1_usigs, const uint64_t* e2_usigs, size_t sketchsize64, size_t bbits) {
    size_t samebits = 0;

    for (size_t i = 0; i < sketchsize64; ++i) {
        __m512i common_bits_accumulated = _mm512_set1_epi64(~0ULL);
        const uint64_t* ptr1 = e1_usigs + i * bbits;
        const uint64_t* ptr2 = e2_usigs + i * bbits;
        const uint64_t* end1 = ptr1 + bbits;

        while (ptr1 + 8 <= end1) {
            __m512i e1_val = _mm512_loadu_si512((__m512i*)ptr1);
            __m512i e2_val = _mm512_loadu_si512((__m512i*)ptr2);
            __m512i xored = _mm512_xor_si512(e1_val, e2_val);
            common_bits_accumulated = _mm512_andnot_si512(xored, common_bits_accumulated);
            ptr1 += 8;
            ptr2 += 8;
        }

        uint64_t aggregated_result = ~0ULL;
        __m256i half1 = _mm512_extracti64x4_epi64(common_bits_accumulated, 0);
        __m256i half2 = _mm512_extracti64x4_epi64(common_bits_accumulated, 1);
        aggregated_result &= _mm256_extract_epi64(half1, 0);
        aggregated_result &= _mm256_extract_epi64(half1, 1);
        aggregated_result &= _mm256_extract_epi64(half1, 2);
        aggregated_result &= _mm256_extract_epi64(half1, 3);
        aggregated_result &= _mm256_extract_epi64(half2, 0);
        aggregated_result &= _mm256_extract_epi64(half2, 1);
        aggregated_result &= _mm256_extract_epi64(half2, 2);
        aggregated_result &= _mm256_extract_epi64(half2, 3);
    // Handle remaining elements with scalar operations
        while (ptr1 < end1) {
            aggregated_result &= ~(*ptr1++ ^ *ptr2++);
        }

        samebits += __builtin_popcountll(aggregated_result);
    }

    return samebits;
}






size_t calc_intersize12_scalar(const uint64_t* e1_usigs, const uint64_t* e2_usigs, size_t sketchsize64, size_t bbits) {
    size_t samebits = 0;
    for (size_t i = 0; i < sketchsize64; i++) {
        uint64_t bits = ~0ULL;
        for (size_t j = 0; j < bbits; j++) {
            bits &= ~(e1_usigs[i * bbits + j] ^ e2_usigs[i * bbits + j]);
        }
        samebits += __builtin_popcountll(bits);
    }
    return samebits;
}

int main() {
    const size_t sketchsize64 = 12800;  // Adjust as needed
    const size_t bbits = 64;           // Adjust as needed

    std::vector<uint64_t> e1_usigs(sketchsize64 * bbits, 0xABCD);
    std::vector<uint64_t> e2_usigs(sketchsize64 * bbits, 0x1234);

    // Measure AVX512 performance
    auto start_avx512 = std::chrono::high_resolution_clock::now();
    size_t result_avx512 = calc_intersize12_avx512(e1_usigs.data(), e2_usigs.data(), sketchsize64, bbits);
    auto end_avx512 = std::chrono::high_resolution_clock::now();
    auto elapsed_avx512 = std::chrono::duration_cast<std::chrono::microseconds>(end_avx512 - start_avx512).count();

    // Measure AVX2 performance
    auto start_avx2 = std::chrono::high_resolution_clock::now();
    size_t result_avx2 = calc_intersize12_avx2(e1_usigs.data(), e2_usigs.data(), sketchsize64, bbits);
    auto end_avx2 = std::chrono::high_resolution_clock::now();
    auto elapsed_avx2 = std::chrono::duration_cast<std::chrono::microseconds>(end_avx2 - start_avx2).count();

    // Measure Scalar performance
    auto start_scalar = std::chrono::high_resolution_clock::now();
    size_t result_scalar = calc_intersize12_scalar(e1_usigs.data(), e2_usigs.data(), sketchsize64, bbits);
    auto end_scalar = std::chrono::high_resolution_clock::now();

    auto elapsed_scalar = std::chrono::duration_cast<std::chrono::microseconds>(end_scalar - start_scalar).count();
    std::cout << "Scalar Result: " << result_scalar << ", Elapsed time: " << elapsed_scalar << " microseconds\n";
    std::cout << "AVX2 Result: " << result_avx2 << ", Elapsed time: " << elapsed_avx2 << " microseconds\n";
    std::cout << "AVX512 Result: " << result_avx512 << ", Elapsed time: " << elapsed_avx512 << " microseconds\n";
    return 0;
}
