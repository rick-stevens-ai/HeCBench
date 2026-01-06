#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <sycl/sycl.hpp>
#include <vector>
#include <numeric>
#include <iomanip>

#define TBSIZE 256
#define DOT_NUM_BLOCKS 256
#define SCALAR (0.4f)

// Default size of 2^26
int ARRAY_SIZE = 67108864;
unsigned int num_times = 10;

void matrix_mul(sycl::nd_item<2> &item, float *a, float *b, float *c, int M, int K, int N, float alpha, float beta) {
    int row = item.get_global_id(0);
    int col = item.get_global_id(1);
    if (row < M && col < N) {
        float s = 0;
        for (int k = 0; k < K; k++)
            s += a[row * K + k] * b[k * N + col];
        c[row * N + col] = alpha * s + beta * c[row * N + col];
    }
}

void run_simple_gemm(sycl::queue &q, float *a, float *b, float *c, int M, int K, int N, float alpha, float beta) {
    sycl::range<2> gws ((M + TBSIZE - 1) / TBSIZE * TBSIZE,
                        (N + TBSIZE - 1) / TBSIZE * TBSIZE);
    sycl::range<2> lws (TBSIZE, TBSIZE);

    q.submit([&] (sycl::handler &cgh) {
        cgh.parallel_for(sycl::nd_range<2>(gws, lws), [=] (sycl::nd_item<2> item) {
            matrix_mul(item, a, b, c, M, K, N, alpha, beta);
        });
    });
}

void run() {
    std::cout << "Running kernels " << num_times << " times" << std::endl;

    // The array size must be divisible by TBSIZE for kernel launches
    if (ARRAY_SIZE % TBSIZE != 0) {
        std::stringstream ss;
        ss << "Array size must be a multiple of " << TBSIZE;
        throw std::runtime_error(ss.str());
    }

#ifdef USE_GPU
    sycl::queue q(sycl::gpu_selector_v, sycl::property::queue::in_order());
#else
    sycl::queue q(sycl::cpu_selector_v, sycl::property::queue::in_order());
#endif

    size_t array_bytes = ARRAY_SIZE * sizeof(float);

    float *da = sycl::malloc_device<float>(ARRAY_SIZE, q);
    float *db = sycl::malloc_device<float>(ARRAY_SIZE, q);
    float *dc = sycl::malloc_device<float>(ARRAY_SIZE, q);
    float *dr = sycl::malloc_device<float>(ARRAY_SIZE, q);

    std::cout << "Array size: " << ARRAY_SIZE*sizeof(float)*1.0E-6 << " MB"
            << " (=" << ARRAY_SIZE*sizeof(float)*1.0E-9 << " GB)" << std::endl;
    std::cout << "Total size: " << 3.0*ARRAY_SIZE*sizeof(float)*1.0E-6 << " MB"
            << " (=" << 3.0*ARRAY_SIZE*sizeof(float)*1.0E-9 << " GB)" << std::endl;

    // Initialize arrays with a simple pattern
    q.submit([&](sycl::handler &cgh) {
        cgh.parallel_for(sycl::range<1>(ARRAY_SIZE), [=](sycl::id<1> idx) {
            int i = idx[0];
            da[i] = 1.0f;
            db[i] = 2.0f;
            dc[i] = 0.0f;
            dr[i] = 0.0f;
        });
    }).wait();

    // List of times
    std::vector<std::vector<double>> timings(3);
    std::chrono::high_resolution_clock::time_point t1, t2;

    // Main loop
    for (unsigned int k = 0; k < num_times; k++) {
        // Copy
        t1 = std::chrono::high_resolution_clock::now();
        q.memcpy(dc, da, array_bytes).wait();
        t2 = std::chrono::high_resolution_clock::now();
        timings[0].push_back(std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count());

        // Stream
        t1 = std::chrono::high_resolution_clock::now();
        q.submit([&](sycl::handler &cgh) {
            cgh.parallel_for(sycl::range<1>(ARRAY_SIZE), [=](sycl::id<1> idx) {
                int i = idx[0];
                dc[i] = da[i] + db[i];
            });
        }).wait();
        t2 = std::chrono::high_resolution_clock::now();
        timings[1].push_back(std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count());

        // Triad
        t1 = std::chrono::high_resolution_clock::now();
        q.submit([&](sycl::handler &cgh) {
            cgh.parallel_for(sycl::range<1>(ARRAY_SIZE), [=](sycl::id<1> idx) {
                int i = idx[0];
                da[i] = db[i] + SCALAR * dc[i];
            });
        }).wait();
        t2 = std::chrono::high_resolution_clock::now();
        timings[2].push_back(std::chrono::duration_cast<std::chrono::duration<double>>(t2 - t1).count());
    }

    // Display timing results
    std::cout << std::left << std::setw(12) << "Function"
            << std::left << std::setw(12) << "MBytes/sec"
            << std::left << std::setw(12) << "Min (sec)"
            << std::left << std::setw(12) << "Max"
            << std::left << std::setw(12) << "Average"
            << std::endl;

    std::vector<std::string> labels = {"Copy", "Stream", "Triad"};
    std::vector<size_t> sizes = {
        2 * sizeof(float) * ARRAY_SIZE,
        2 * sizeof(float) * ARRAY_SIZE,
        3 * sizeof(float) * ARRAY_SIZE
    };

    for (size_t i = 0; i < timings.size(); ++i) {
        auto minmax = std::minmax_element(timings[i].begin()+1, timings[i].end());
        double average = std::accumulate(timings[i].begin()+1, timings[i].end(), 0.0) / (double)(num_times - 1);
        double bandwidth = 1.0E-6 * sizes[i] / (*minmax.first);

        std::cout << std::left << std::setw(12) << labels[i]
                << std::left << std::setw(12) << std::setprecision(3) << bandwidth
                << std::left << std::setw(12) << std::setprecision(5) << *minmax.first
                << std::left << std::setw(12) << std::setprecision(5) << *minmax.second
                << std::left << std::setw(12) << std::setprecision(5) << average
                << std::endl;
    }

    sycl::free(da, q);
    sycl::free(db, q);
    sycl::free(dc, q);
    sycl::free(dr, q);
}

void parseArguments(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (!std::string("--arraysize").compare(argv[i]) ||
            !std::string("-s").compare(argv[i])) {
            if (++i >= argc || !(ARRAY_SIZE = atoi(argv[i])) || ARRAY_SIZE <= 0) {
                std::cerr << "Invalid array size." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (!std::string("--numtimes").compare(argv[i]) ||
                !std::string("-n").compare(argv[i])) {
            if (++i >= argc || !(num_times = atoi(argv[i])) || num_times < 2) {
                std::cerr << "Invalid number of times." << std::endl;
                exit(EXIT_FAILURE);
            }
        }
        else if (!std::string("--help").compare(argv[i]) ||
                !std::string("-h").compare(argv[i])) {
            std::cout << "\nUsage: " << argv[0] << " [OPTIONS]\n\n"
                    << "Options:\n"
                    << "  -h  --help               Print this message\n"
                    << "  -s  --arraysize  SIZE    Use SIZE elements in the array\n"
                    << "  -n  --numtimes   NUM     Run the test NUM times (NUM >= 2)\n\n";
            exit(EXIT_SUCCESS);
        }
        else {
            std::cerr << "Unrecognized argument '" << argv[i] << "' (try '--help')\n";
            exit(EXIT_FAILURE);
        }
    }
}

int main(int argc, char *argv[]) {
    parseArguments(argc, argv);
    run();
    return 0;
}
