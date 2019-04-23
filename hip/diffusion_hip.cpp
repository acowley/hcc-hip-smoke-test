/// Sample parallel computation code by @smithakihide
/// from https://github.com/RadeonOpenCompute/hcc/issues/548
/// Adapated from C++ AMP to hcc by Anthony Cowley (@acowley)
#include <chrono>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include <hip/hip_runtime.h>

using std::cout;

int32_t N = 1024 * 1024;
int32_t nT = 128;

using realp = float;
void Acc();
void cpustd();

int main(int carg, char *varg[]) {
  // int sel;
  // sel = 0;

  if (carg > 1) {
    try {
      N = std::stoi(varg[1]);
    } catch (...) {
    }
  }

  if (carg > 2) {
    try {
      nT = std::stoi(varg[2]);
    } catch (...) {
    }
  }

  cout << "carg = " << carg << "\n";
  cout << "size = " << N << "\n";
  cout << "nT = " << nT << "\n";

  Acc();
  cpustd();

  return 0;
}

__global__ void diffusion(const int r, const int N, float* aA) {
  int i = hipThreadIdx_x + hipBlockIdx_x * hipBlockDim_x;
  const int stride = hipBlockDim_x * hipGridDim_x;
  const int stop = (N - 2) / 2;
  for(; i < stop; i += stride) {
    const int iC = i * 2 + r + 1;
    const int iE = iC + 1;
    const int iW = iC - 1;
    aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
    aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
    aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
    aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
    aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
    aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
    aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
    aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
  }
}

void Acc() {
  cout << "by hip\n";
  realp pvA[N];

  using clock = std::chrono::high_resolution_clock;

  // Warmup the HIP runtime
  float* aA;
  hipMalloc(&aA, N*sizeof(realp));
  hipLaunchKernelGGL(diffusion, 128, 64, 0, 0, 0, N, aA);

  // initialization.
  auto t0 = clock::now();
  std::mt19937 mt(19);
  std::uniform_real_distribution<realp> dist(-1, 1);
  // std::generate_n(pvA, N, [&]() { return dist(mt); });
  // pvA[0] = 0;
  // pvA[N - 1] = 1;

  for(int i = 0; i < N; ++i) { pvA[i] = dist(mt); }
  pvA[0] = 0;
  pvA[N - 1] = 1;
  hipMemcpy(aA, pvA, N*sizeof(realp), hipMemcpyHostToDevice);
  hipDeviceSynchronize();

  hipEvent_t hstart, hstop;
  hipEventCreate(&hstart);
  hipEventCreate(&hstop);

  auto t1 = clock::now();
  hipEventRecord(hstart, nullptr);
  for (int32_t cT = 0; cT < nT; cT++) {
    for (int32_t r = 0; r < 2; r++) {
      hipLaunchKernelGGL(diffusion, 512, 256, 0, 0, r, N, aA);
    }
  }

  hipEventRecord(hstop, nullptr);
  hipEventSynchronize(hstop);
  hipDeviceSynchronize();
  auto t2 = clock::now();
  float htime;
  hipEventElapsedTime(&htime, hstart, hstop);
  hipMemcpy(pvA, aA, (N/4+1)*sizeof(realp), hipMemcpyDeviceToHost);
  realp res = pvA[N / 4];
  hipDeviceSynchronize();
  auto t4 = clock::now();

  hipFree(aA);

  using std::chrono::duration_cast;
  using std::chrono::microseconds;

  double dt1 = (double)duration_cast<microseconds>(t1 - t0).count() / 1000000;
  double dt2 = (double)duration_cast<microseconds>(t2 - t1).count() / 1000000;
  double dt4 = (double)duration_cast<microseconds>(t4 - t1).count() / 1000000;

  cout << "Result = " << res << "\n";
  cout << "init = " << dt1 << "[s]\n";
  cout << "dt2 = " << dt2 << "[s]\n";
  cout << "hiptime = " << htime << "[ms]\n";
  cout << "calc = " << dt4 << "[s]\n";

  cout << "Acc\n";
}

void cpustd() {
  cout << "by raw cpu with vector\n";

  std::vector<realp> *pvA;

  pvA = new std::vector<realp>(N);

  using clock = std::chrono::high_resolution_clock;

  auto t0 = clock::now();
  std::vector<realp> &aA = *pvA;

  // initialization.
  std::mt19937 mt(19);
  std::uniform_real_distribution<realp> dist(-1, 1);
  for (int32_t i = 0; i < N; i++) {
    aA[i] = dist(mt);
  }
  aA[0] = 0;
  aA[N - 1] = 1;

  auto t1 = clock::now();

  for (int32_t cT = 0; cT < nT; cT++) {
    for (int32_t r = 0; r < 2; r++) {
      for (int32_t i = 0; i < (N - 2) / 2; i++) {
        int32_t iC = i * 2 + r + 1;
        int32_t iE = iC + 1;
        int32_t iW = iC - 1;
        aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
        aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
        aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
        aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
        aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
        aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
        aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
        aA[iC] = aA[iC] + 0.1 * (aA[iE] - 2 * aA[iC] + aA[iW]);
      }
    }
  }
  auto t2 = clock::now();

  realp res = aA[N / 4];
  auto t4 = clock::now();

  using std::chrono::duration_cast;
  using std::chrono::microseconds;

  double dt1 = (double)duration_cast<microseconds>(t1 - t0).count() / 1000000;
  double dt2 = (double)duration_cast<microseconds>(t2 - t1).count() / 1000000;
  double dt4 = (double)duration_cast<microseconds>(t4 - t1).count() / 1000000;

  cout << "Result = " << res << "\n";
  cout << "init = " << dt1 << "[s]\n";
  cout << "dt2 = " << dt2 << "[s]\n";
  cout << "calc = " << dt4 << "[s]\n";
}
