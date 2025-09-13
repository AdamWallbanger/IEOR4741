#include <bits/stdc++.h>
using namespace std;

#if defined(_MSC_VER)
  #include <malloc.h>
#endif

static inline void* aligned_malloc(size_t bytes, size_t alignment = 64) {
#if defined(_MSC_VER)
    return _aligned_malloc(bytes, alignment);
#elif defined(__APPLE__) || defined(__ANDROID__)
    void* p = nullptr;
    if (posix_memalign(&p, alignment, bytes) != 0) return nullptr;
    return p;
#else
    if (bytes % alignment) bytes += alignment - (bytes % alignment);
    return std::aligned_alloc(alignment, bytes);
#endif
}
static inline void aligned_free(void* p) {
#if defined(_MSC_VER)
    _aligned_free(p);
#else
    std::free(p);
#endif
}
static inline double* aligned_alloc_double(size_t count, size_t alignment = 64) {
    void* p = aligned_malloc(count * sizeof(double), alignment);
    if (!p) throw std::bad_alloc();
    return static_cast<double*>(p);
}

using clock_t_ = std::chrono::high_resolution_clock;
struct Stats { double mean_ms, std_ms; };

template<typename Fn>
Stats time_it(Fn&& f, int runs=5) {
    vector<double> v; v.reserve(runs);
    for (int i=0;i<runs;++i) {
        auto t0 = clock_t_::now();
        f();
        auto t1 = clock_t_::now();
        v.push_back(std::chrono::duration<double, std::milli>(t1-t0).count());
    }
    double mean = accumulate(v.begin(), v.end(), 0.0) / v.size();
    double var = 0.0;
    for (double x: v) var += (x-mean)*(x-mean);
    var /= v.size();
    return {mean, std::sqrt(var)};
}

static inline bool nearly_equal(double a, double b, double eps=1e-9) {
    double diff = std::abs(a-b);
    double scale = std::max({1.0, std::abs(a), std::abs(b)});
    return diff <= eps*scale;
}
static inline bool allclose(const double* A, const double* B, size_t n, double eps=1e-9) {
    for (size_t i=0;i<n;++i) if (!nearly_equal(A[i], B[i], eps)) return false;
    return true;
}
static inline void fill_random(double* p, size_t n, uint64_t seed=42) {
    std::mt19937_64 rng(seed);
    std::uniform_real_distribution<double> dist(-1.0, 1.0);
    for (size_t i=0;i<n;++i) p[i] = dist(rng);
}

// Matrix-Vector (row-major): y = A(rows x cols) * x
void multiply_mv_row_major(const double* A, int rows, int cols,
                           const double* x, double* y) {
    if (!A || !x || !y || rows<=0 || cols<=0) return;
    // Optional OpenMP
    #ifdef _OPENMP
    #pragma omp parallel for
    #endif
    for (int i = 0; i < rows; ++i) {
        const double* Ai = A + (size_t)i * cols;
        double acc = 0.0;
        int j=0;
        for (; j+4<=cols; j+=4) {
            acc += Ai[j+0]*x[j+0] + Ai[j+1]*x[j+1] + Ai[j+2]*x[j+2] + Ai[j+3]*x[j+3];
        }
        for (; j<cols; ++j) acc += Ai[j]*x[j];
        y[i] = acc;
    }
}

// Matrix-Vector (column-major A): y = A(rows x cols) * x
void multiply_mv_col_major(const double* A, int rows, int cols,
                           const double* x, double* y) {
    if (!A || !x || !y || rows<=0 || cols<=0) return;
    std::memset(y, 0, sizeof(double) * (size_t)rows);
    for (int j=0; j<cols; ++j) {
        const double xj = x[j];
        const double* Aj = A + (size_t)j*rows; // column base
        int i=0;
        for (; i+4<=rows; i+=4) {
            y[i+0] += Aj[i+0]*xj;
            y[i+1] += Aj[i+1]*xj;
            y[i+2] += Aj[i+2]*xj;
            y[i+3] += Aj[i+3]*xj;
        }
        for (; i<rows; ++i) y[i] += Aj[i]*xj;
    }
}

// Matrix-Matrix naive (row-major): C = A(MxK) * B(KxN)
void multiply_mm_naive(const double* A, int M, int K,
                       const double* B, int K2, int N,
                       double* C) {
    if (!A || !B || !C || M<=0 || K<=0 || N<=0 || K!=K2) return;
    std::memset(C, 0, sizeof(double) * (size_t)M*N);
    // ikj ordering: stream C row, use B row contiguous
    for (int i=0; i<M; ++i) {
        const double* Ai = A + (size_t)i*K;
        double* Ci = C + (size_t)i*N;
        for (int k=0; k<K; ++k) {
            const double aik = Ai[k];
            const double* Bk = B + (size_t)k*N;
            int j=0;
            for (; j+4<=N; j+=4) {
                Ci[j+0] += aik * Bk[j+0];
                Ci[j+1] += aik * Bk[j+1];
                Ci[j+2] += aik * Bk[j+2];
                Ci[j+3] += aik * Bk[j+3];
            }
            for (; j<N; ++j) Ci[j] += aik * Bk[j];
        }
    }
}

// Transpose helper: B (KxN, row-major) -> B_T (N x K, row-major)
void transpose_rowmajor(const double* B, int rowsB, int colsB, double* B_T) {
    for (int r=0; r<rowsB; ++r)
        for (int c=0; c<colsB; ++c)
            B_T[(size_t)c*rowsB + r] = B[(size_t)r*colsB + c];
}

// Matrix-Matrix with transposed B: C = A(MxK) * B_T(NxK)^T
void multiply_mm_transposed_b(const double* A, int M, int K,
                              const double* B_T, int K2, int N,
                              double* C) {
    if (!A || !B_T || !C || M<=0 || K<=0 || N<=0 || K!=K2) return;
    for (int i=0; i<M; ++i) {
        const double* Ai = A + (size_t)i*K;
        double* Ci = C + (size_t)i*N;
        for (int j=0; j<N; ++j) {
            const double* Bj = B_T + (size_t)j*K;
            double acc = 0.0;
            int k=0;
            for (; k+4<=K; k+=4) {
                acc += Ai[k+0]*Bj[k+0] + Ai[k+1]*Bj[k+1]
                     + Ai[k+2]*Bj[k+2] + Ai[k+3]*Bj[k+3];
            }
            for (; k<K; ++k) acc += Ai[k]*Bj[k];
            Ci[j] = acc;
        }
    }
}

void multiply_mm_blocked(const double* A, int M, int K,
                         const double* B, int K2, int N,
                         double* C, int blockM=128, int blockN=128, int blockK=256) {
    if (!A || !B || !C || M<=0 || K<=0 || N<=0 || K!=K2) return;
    std::memset(C, 0, sizeof(double) * (size_t)M*N);

    unique_ptr<double[]> BT(new double[(size_t)N*K]);
    transpose_rowmajor(B, K, N, BT.get());

    for (int i0=0; i0<M; i0+=blockM) {
        int iMax = std::min(i0 + blockM, M);
        for (int j0=0; j0<N; j0+=blockN) {
            int jMax = std::min(j0 + blockN, N);
            for (int k0=0; k0<K; k0+=blockK) {
                int kMax = std::min(k0 + blockK, K);
                for (int i=i0; i<iMax; ++i) {
                    const double* Ai_block = A + (size_t)i*K + k0;
                    double* Ci_block = C + (size_t)i*N + j0;
                    for (int j=j0; j<jMax; ++j) {
                        const double* Bj_block = BT.get() + (size_t)j*K + k0;
                        double acc = Ci_block[j - j0];
                        int k = k0;
                        for (; k+4<=kMax; k+=4) {
                            int kk = k - k0;
                            acc += Ai_block[kk+0] * Bj_block[0];
                            acc += Ai_block[kk+1] * Bj_block[1];
                            acc += Ai_block[kk+2] * Bj_block[2];
                            acc += Ai_block[kk+3] * Bj_block[3];
                            Bj_block += 4;
                        }
                        for (; k<kMax; ++k) {
                            acc += Ai_block[k - k0] * *Bj_block++;
                        }
                        Ci_block[j - j0] = acc;
                    }
                }
            }
        }
    }
}

static void simple_tests() {
    // MM: naive vs B^T
    {
        const int M=2,K=3,N=2;
        double A[M*K] = {1,2,3,4,5,6};
        double B[K*N] = {7,8, 9,10, 11,12};
        double C1[M*N], C2[M*N];
        multiply_mm_naive(A,M,K,B,K,N,C1);
        vector<double> BT((size_t)N*K);
        transpose_rowmajor(B,K,N, BT.data());
        multiply_mm_transposed_b(A,M,K, BT.data(), K,N, C2);
        if (!allclose(C1,C2,(size_t)M*N, 1e-9)) {
            cerr << "MM small test FAILED\n"; exit(1);
        }
    }
    // MV: row-major vs column-major
    {
        const int R=3,C=4;
        vector<double> A(R*C), x(C), y1(R), y2(R);
        fill_random(A.data(), A.size(), 1);
        fill_random(x.data(), x.size(), 2);
        multiply_mv_row_major(A.data(), R,C, x.data(), y1.data());
        vector<double> Acm(R*C);
        for (int i=0;i<R;++i) for (int j=0;j<C;++j) Acm[(size_t)j*R + i] = A[(size_t)i*C + j];
        multiply_mv_col_major(Acm.data(), R,C, x.data(), y2.data());
        if (!allclose(y1.data(), y2.data(), (size_t)R, 1e-9)) {
            cerr << "MV small test FAILED\n"; exit(1);
        }
    }
    cout << "Basic correctness tests passed.\n";
}

int main(int argc, char** argv) {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    simple_tests();

    vector<int> sizes = {256, 512, 1024, 1536, 2048};
    if (argc > 1) {
        sizes.clear();
        for (int i=1;i<argc;++i) sizes.push_back(atoi(argv[i]));
    }

    // MV: row-major vs column-major
    printf("\n=== Matrix-Vector (row-major vs column-major) ===\n");
    printf("%6s  | %15s  %15s\n", "N", "row-major (ms)", "col-major (ms)");
    printf("--------+-------------------------------------------\n");
    for (int n : sizes) {
        // A_row (row-major), A_col (column-major)
        double* A_row = aligned_alloc_double((size_t)n*n);
        double* A_col = aligned_alloc_double((size_t)n*n);
        double* x     = aligned_alloc_double(n);
        double* y1    = aligned_alloc_double(n);
        double* y2    = aligned_alloc_double(n);

        fill_random(A_row, (size_t)n*n, 3);
        for (int i=0;i<n;++i) for (int j=0;j<n;++j)
            A_col[(size_t)j*n + i] = A_row[(size_t)i*n + j];
        fill_random(x, n, 4);

        auto s1 = time_it([&]{ multiply_mv_row_major(A_row, n,n, x, y1); }, 5);
        auto s2 = time_it([&]{ multiply_mv_col_major(A_col, n,n, x, y2); }, 5);

        if (!allclose(y1,y2,(size_t)n,1e-9)) cerr << "MV mismatch at n="<<n<<"\n";
        printf("%6d  | %10.3f \xC2\xB1 %-7.3f  %10.3f \xC2\xB1 %-7.3f\n",
               n, s1.mean_ms, s1.std_ms, s2.mean_ms, s2.std_ms);

        aligned_free(A_row); aligned_free(A_col);
        aligned_free(x); aligned_free(y1); aligned_free(y2);
    }

    // MM: naive vs B^T vs blocked
    printf("\n=== Matrix-Matrix (naive vs B^T vs blocked) ===\n");
    printf("%6s  | %12s  %12s  %12s\n", "N", "naive (ms)", "B^T (ms)", "blocked (ms)");
    printf("--------+-----------------------------------------------\n");
    for (int n : sizes) {
        int M=n,K=n,N=n;
        double* A  = aligned_alloc_double((size_t)M*K);
        double* B  = aligned_alloc_double((size_t)K*N);
        double* C1 = aligned_alloc_double((size_t)M*N);
        double* C2 = aligned_alloc_double((size_t)M*N);
        double* C3 = aligned_alloc_double((size_t)M*N);
        vector<double> BT((size_t)N*K);

        fill_random(A, (size_t)M*K, 5);
        fill_random(B, (size_t)K*N, 6);
        memset(C1,0,sizeof(double)*(size_t)M*N);
        memset(C2,0,sizeof(double)*(size_t)M*N);
        memset(C3,0,sizeof(double)*(size_t)M*N);

        auto t_naive = time_it([&]{ multiply_mm_naive(A,M,K,B,K,N,C1); }, 3);

        transpose_rowmajor(B,K,N, BT.data());
        auto t_bt = time_it([&]{ multiply_mm_transposed_b(A,M,K, BT.data(), K,N, C2); }, 3);

        auto t_blk = time_it([&]{ multiply_mm_blocked(A,M,K,B,K,N,C3, 128,128,256); }, 3);

        if (!allclose(C1,C2,(size_t)M*N,1e-8) || !allclose(C1,C3,(size_t)M*N,1e-8)) {
            cerr << "MM mismatch at n="<<n<<"\n";
        }

        printf("%6d  | %10.1f \xC2\xB1 %-5.1f  %10.1f \xC2\xB1 %-5.1f  %10.1f \xC2\xB1 %-5.1f\n",
               n, t_naive.mean_ms, t_naive.std_ms,
               t_bt.mean_ms, t_bt.std_ms,
               t_blk.mean_ms, t_blk.std_ms);

        aligned_free(A); aligned_free(B);
        aligned_free(C1); aligned_free(C2); aligned_free(C3);
    }

    puts("\nDone.");
    return 0;
}
