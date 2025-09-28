#define _GNU_SOURCE
#include "sysfetch.h"
#include <immintrin.h>
#include <cpuid.h>

static int cpu_features_detected = 0;
static int has_avx2 = 0;
static int has_sse42 = 0;

void detect_cpu_features(void) {
    if (cpu_features_detected) return;

    unsigned int eax, ebx, ecx, edx;

    if (__get_cpuid(1, &eax, &ebx, &ecx, &edx)) {
        has_sse42 = (ecx & bit_SSE4_2) != 0;
    }

    if (__get_cpuid_count(7, 0, &eax, &ebx, &ecx, &edx)) {
        has_avx2 = (ebx & bit_AVX2) != 0;
    }

    cpu_features_detected = 1;
}

void fast_string_copy_simd(char *dest, const char *src, size_t len) {
    detect_cpu_features();

    if (has_avx2 && len >= 32) {
        size_t simd_len = len & ~31;
        for (size_t i = 0; i < simd_len; i += 32) {
            __m256i data = _mm256_loadu_si256((__m256i*)(src + i));
            _mm256_storeu_si256((__m256i*)(dest + i), data);
        }
        for (size_t i = simd_len; i < len; i++) {
            dest[i] = src[i];
        }
    } else if (has_sse42 && len >= 16) {
        size_t simd_len = len & ~15;
        for (size_t i = 0; i < simd_len; i += 16) {
            __m128i data = _mm_loadu_si128((__m128i*)(src + i));
            _mm_storeu_si128((__m128i*)(dest + i), data);
        }
        for (size_t i = simd_len; i < len; i++) {
            dest[i] = src[i];
        }
    } else {
        memcpy(dest, src, len);
    }
    dest[len] = '\0';
}

int fast_string_compare_simd(const char *str1, const char *str2, size_t len) {
    detect_cpu_features();

    if (has_avx2 && len >= 32) {
        size_t simd_len = len & ~31;
        for (size_t i = 0; i < simd_len; i += 32) {
            __m256i a = _mm256_loadu_si256((__m256i*)(str1 + i));
            __m256i b = _mm256_loadu_si256((__m256i*)(str2 + i));
            __m256i cmp = _mm256_cmpeq_epi8(a, b);
            int mask = _mm256_movemask_epi8(cmp);
            if (mask != -1) return 0;
        }
        for (size_t i = simd_len; i < len; i++) {
            if (str1[i] != str2[i]) return 0;
        }
        return 1;
    } else {
        return memcmp(str1, str2, len) == 0;
    }
}

void parallel_string_processing(char *buffer, size_t size) {
    detect_cpu_features();

    if (!has_sse42) return;

    __m128i newlines = _mm_set1_epi8('\n');
    __m128i spaces = _mm_set1_epi8(' ');
    __m128i tabs = _mm_set1_epi8('\t');

    size_t simd_size = size & ~15;
    for (size_t i = 0; i < simd_size; i += 16) {
        __m128i data = _mm_loadu_si128((__m128i*)(buffer + i));

        __m128i nl_mask = _mm_cmpeq_epi8(data, newlines);
        __m128i sp_mask = _mm_cmpeq_epi8(data, spaces);
        __m128i tb_mask = _mm_cmpeq_epi8(data, tabs);

        __m128i whitespace = _mm_or_si128(_mm_or_si128(nl_mask, sp_mask), tb_mask);

        int mask = _mm_movemask_epi8(whitespace);
        if (mask) {
            for (int j = 0; j < 16; j++) {
                if (mask & (1 << j)) {
                    if (buffer[i + j] == '\n') {
                        buffer[i + j] = '\0';
                        return;
                    }
                }
            }
        }
    }
}