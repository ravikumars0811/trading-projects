/*
 * SIMD Basics - SSE and AVX Examples
 *
 * Demonstrates:
 * - Vector operations with SSE/AVX
 * - Performance comparison vs scalar code
 * - Alignment requirements
 * - Common SIMD patterns
 *
 * Build: gcc -O3 -march=native -mavx2 -o simd_basics simd_basics.c -lm
 * Run: ./simd_basics
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <immintrin.h>  /* SSE, AVX intrinsics */

#define ARRAY_SIZE (1024 * 1024)
#define ALIGNMENT 32
#define NUM_ITERATIONS 100

/* Get time in microseconds */
static uint64_t get_time_us(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000ULL + tv.tv_usec;
}

/* Example 1: Vector addition - Scalar version */
static void vec_add_scalar(float *c, const float *a, const float *b, size_t n)
{
    for (size_t i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Example 1: Vector addition - SSE version (4 floats at once) */
static void vec_add_sse(float *c, const float *a, const float *b, size_t n)
{
    size_t i = 0;

    /* Process 4 floats at a time */
    for (; i + 3 < n; i += 4) {
        __m128 va = _mm_load_ps(&a[i]);
        __m128 vb = _mm_load_ps(&b[i]);
        __m128 vc = _mm_add_ps(va, vb);
        _mm_store_ps(&c[i], vc);
    }

    /* Handle remaining elements */
    for (; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Example 1: Vector addition - AVX version (8 floats at once) */
static void vec_add_avx(float *c, const float *a, const float *b, size_t n)
{
    size_t i = 0;

    /* Process 8 floats at a time */
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_load_ps(&a[i]);
        __m256 vb = _mm256_load_ps(&b[i]);
        __m256 vc = _mm256_add_ps(va, vb);
        _mm256_store_ps(&c[i], vc);
    }

    /* Handle remaining elements */
    for (; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

/* Benchmark vector addition */
static void benchmark_vec_add(void)
{
    float *a, *b, *c;
    uint64_t start, end;
    double scalar_time, sse_time, avx_time;

    printf("\n=== Vector Addition Benchmark ===\n");
    printf("Array size: %d floats (%.2f MB)\n",
           ARRAY_SIZE, (ARRAY_SIZE * sizeof(float)) / (1024.0 * 1024.0));
    printf("Iterations: %d\n", NUM_ITERATIONS);

    /* Allocate aligned memory */
    a = aligned_alloc(ALIGNMENT, ARRAY_SIZE * sizeof(float));
    b = aligned_alloc(ALIGNMENT, ARRAY_SIZE * sizeof(float));
    c = aligned_alloc(ALIGNMENT, ARRAY_SIZE * sizeof(float));

    /* Initialize */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        a[i] = (float)i;
        b[i] = (float)(i * 2);
    }

    /* Benchmark scalar version */
    start = get_time_us();
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        vec_add_scalar(c, a, b, ARRAY_SIZE);
    }
    end = get_time_us();
    scalar_time = (end - start) / 1000.0;

    printf("\nScalar version:\n");
    printf("  Time: %.2f ms\n", scalar_time);
    printf("  Throughput: %.2f GB/s\n",
           (ARRAY_SIZE * sizeof(float) * 3 * NUM_ITERATIONS) /
           (scalar_time * 1000.0) / (1024.0 * 1024.0));

    /* Benchmark SSE version */
    start = get_time_us();
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        vec_add_sse(c, a, b, ARRAY_SIZE);
    }
    end = get_time_us();
    sse_time = (end - start) / 1000.0;

    printf("\nSSE version (4 floats):\n");
    printf("  Time: %.2f ms\n", sse_time);
    printf("  Throughput: %.2f GB/s\n",
           (ARRAY_SIZE * sizeof(float) * 3 * NUM_ITERATIONS) /
           (sse_time * 1000.0) / (1024.0 * 1024.0));
    printf("  Speedup: %.2fx\n", scalar_time / sse_time);

    /* Benchmark AVX version */
    start = get_time_us();
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        vec_add_avx(c, a, b, ARRAY_SIZE);
    }
    end = get_time_us();
    avx_time = (end - start) / 1000.0;

    printf("\nAVX version (8 floats):\n");
    printf("  Time: %.2f ms\n", avx_time);
    printf("  Throughput: %.2f GB/s\n",
           (ARRAY_SIZE * sizeof(float) * 3 * NUM_ITERATIONS) /
           (avx_time * 1000.0) / (1024.0 * 1024.0));
    printf("  Speedup: %.2fx\n", scalar_time / avx_time);

    free(a);
    free(b);
    free(c);
}

/* Example 2: Dot product */
static float dot_product_scalar(const float *a, const float *b, size_t n)
{
    float sum = 0.0f;
    for (size_t i = 0; i < n; i++) {
        sum += a[i] * b[i];
    }
    return sum;
}

static float dot_product_avx(const float *a, const float *b, size_t n)
{
    __m256 vsum = _mm256_setzero_ps();
    size_t i = 0;

    /* Process 8 floats at a time */
    for (; i + 7 < n; i += 8) {
        __m256 va = _mm256_load_ps(&a[i]);
        __m256 vb = _mm256_load_ps(&b[i]);
        __m256 vmul = _mm256_mul_ps(va, vb);
        vsum = _mm256_add_ps(vsum, vmul);
    }

    /* Horizontal add to get final sum */
    float result[8];
    _mm256_store_ps(result, vsum);
    float sum = result[0] + result[1] + result[2] + result[3] +
                result[4] + result[5] + result[6] + result[7];

    /* Handle remaining elements */
    for (; i < n; i++) {
        sum += a[i] * b[i];
    }

    return sum;
}

/* Benchmark dot product */
static void benchmark_dot_product(void)
{
    float *a, *b;
    uint64_t start, end;
    double scalar_time, avx_time;
    float scalar_result, avx_result;

    printf("\n=== Dot Product Benchmark ===\n");
    printf("Array size: %d floats\n", ARRAY_SIZE);

    /* Allocate aligned memory */
    a = aligned_alloc(ALIGNMENT, ARRAY_SIZE * sizeof(float));
    b = aligned_alloc(ALIGNMENT, ARRAY_SIZE * sizeof(float));

    /* Initialize */
    for (size_t i = 0; i < ARRAY_SIZE; i++) {
        a[i] = 1.0f;
        b[i] = 2.0f;
    }

    /* Benchmark scalar version */
    start = get_time_us();
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        scalar_result = dot_product_scalar(a, b, ARRAY_SIZE);
    }
    end = get_time_us();
    scalar_time = (end - start) / 1000.0;

    printf("\nScalar version:\n");
    printf("  Time: %.2f ms\n", scalar_time);
    printf("  Result: %.2f\n", scalar_result);

    /* Benchmark AVX version */
    start = get_time_us();
    for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
        avx_result = dot_product_avx(a, b, ARRAY_SIZE);
    }
    end = get_time_us();
    avx_time = (end - start) / 1000.0;

    printf("\nAVX version:\n");
    printf("  Time: %.2f ms\n", avx_time);
    printf("  Result: %.2f\n", avx_result);
    printf("  Speedup: %.2fx\n", scalar_time / avx_time);

    free(a);
    free(b);
}

/* Check CPU features */
static void check_cpu_features(void)
{
    printf("\n=== CPU Features ===\n");

#ifdef __SSE__
    printf("SSE: Supported\n");
#endif

#ifdef __SSE2__
    printf("SSE2: Supported\n");
#endif

#ifdef __SSE3__
    printf("SSE3: Supported\n");
#endif

#ifdef __SSE4_1__
    printf("SSE4.1: Supported\n");
#endif

#ifdef __AVX__
    printf("AVX: Supported\n");
#endif

#ifdef __AVX2__
    printf("AVX2: Supported\n");
#endif

#ifdef __AVX512F__
    printf("AVX-512: Supported\n");
#endif
}

int main(void)
{
    printf("SIMD Basics - SSE and AVX Examples\n");
    printf("===================================\n");

    check_cpu_features();
    benchmark_vec_add();
    benchmark_dot_product();

    printf("\n=== Key Takeaways ===\n");
    printf("1. SIMD can provide 4-8x speedup for appropriate workloads\n");
    printf("2. Data must be aligned (16 bytes for SSE, 32 for AVX)\n");
    printf("3. Works best with: arithmetic, data parallel operations\n");
    printf("4. Compiler can auto-vectorize with -O3 -ftree-vectorize\n");
    printf("5. Intrinsics give more control than auto-vectorization\n");
    printf("6. Watch for horizontal operations (they're slow)\n");

    printf("\n=== Common SIMD Use Cases ===\n");
    printf("- Vector/matrix math (graphics, ML)\n");
    printf("- Audio/video processing\n");
    printf("- Image processing\n");
    printf("- Cryptography\n");
    printf("- Packet processing\n");
    printf("- Scientific computing\n");

    return 0;
}
