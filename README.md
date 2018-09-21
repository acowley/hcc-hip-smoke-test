These are **very** simple test programs for the [hcc](https://github.com/RadeonOpenCompute/hcc) and [hip](https://github.com/ROCm-Developer-Tools/HIP) compilers.

A pleasantly interesting one is the `diffusion` example by @smithakihide originally posted as [an `hcc` issue](https://github.com/RadeonOpenCompute/hcc/issues/548). As C++ AMP has been deprecated with ROCm 1.9.0, I have ported this example to hcc C++. What is satisfying about this example is that it offers a simple comparison between the performance of a single CPU core and a GPU. This makes it a pretty good smoke test that parts of the compiler targeting the GPU are working (e.g. an RX 580 is ~75x faster than one core of a Ryzen 5 1400).

Consult the `Makefile`s for other sample programs.
