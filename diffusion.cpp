/// Sample parallel computation code by @smithakihide
/// from https://github.com/RadeonOpenCompute/hcc/issues/548
#include <vector>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <amp.h>
#include <amp_math.h>

concurrency::accelerator ChoseAccelerator()
{
        using  std::cout;
        using  std::cin;
        using  std::wcout;
        using  std::vector;
        using  concurrency::accelerator;

        vector<accelerator> allAccl = accelerator::get_all();


        vector<accelerator> validAccl;
        int numAcs = allAccl.size();

        for (int i = 0; i<numAcs; i++)
        {
                {
                        validAccl.push_back(allAccl[i]);
                }
        }

        if (validAccl.size() == 0)
        {
                cout << "there is no GPU suitable for GPGPU\n";
                throw - 1;
        }

        cout << "select the GPU" << "\n\n";
        for (int i = 0; i<validAccl.size(); i++)
        {
                std::wstring st1;
                std::string st2;
                st1 = validAccl[i].get_description();
                for (auto it : st1)
                {
                        st2.push_back(it);
                }

                cout << i << " : " << st2 << "\n";
                cout << "    memory                           : "
                        << (double)validAccl[i].get_dedicated_memory() / 1024 / 1000 << " [GB]\n";
                cout << "    full double precision feature    : ";
                if (validAccl[i].get_supports_double_precision())
                        cout << "true\n";
                else
                        cout << "false\n";

                cout << "    whether be used to display       : ";
                if (validAccl[i].get_has_display())
                        cout << "true\n";
                else
                        cout << "false\n";

        }
        cout << "\nput a number to specify which GPU to use\n\n";

        cin.exceptions(std::ios::failbit);

        int selAcs;
        while (true)
        {
                cout << "the number = ";
                try {
                        cin >> selAcs;
                }
                catch (...) {
                        cout << "please input a number\n";
                        cin.clear();
                        cin.seekg(0);
                        continue;
                }
                cout << "\n";

                if (0 <= selAcs && selAcs < validAccl.size())
                {
                        cout << selAcs << " is selected\n";
                        return validAccl[selAcs];
                }

                cout << "plsese set a number from 0 to " << validAccl.size() - 1 << "."
                        << "there is no GPU its number is" << selAcs << "\n";
        }


        throw - 1;
}

using  std::cout;
using  concurrency::accelerator;
accelerator ChoseAccelerator();

accelerator acc;
int32_t N = 1024*1024;
int32_t nT = 128;

using realp = float;
void Acc();
void cpustd();

int main(int carg, char* varg[])
{
        acc = ChoseAccelerator();

        int sel;
        sel = 0;

        if (carg > 1)
        {
                try {
                        N = std::stoi(varg[1]);
                }
                catch (...) {
                }
        }

        if (carg > 2)
        {
                try {
                        nT = std::stoi(varg[2]);
                }
                catch (...) {
                }
        }

        cout << "carg = " << carg << "\n";
        cout << "size = " << N << "\n";
        cout << "nT = " << nT << "\n";


        Acc();
        cpustd();

    return 0;
}

void Acc()
{
        cout << "by amp with array\n";
        concurrency::array<realp> *pvA;

        pvA = new concurrency::array<realp>(N, acc.get_default_view());

        auto t0 = std::chrono::high_resolution_clock::now();
        concurrency::array_view<realp> aA = *pvA;
        //initialization.
        std::mt19937 mt(19);
        std::uniform_real_distribution<realp> dist(-1, 1);
        for (int32_t i = 0; i < N; i++)
        {
                aA[i] = dist(mt);
        }
        aA[0] = 0;
        aA[N - 1] = 1;
        aA.synchronize();

        auto t1 = std::chrono::high_resolution_clock::now();

        for (int32_t cT = 0; cT < nT; cT++)
        {
                for (int32_t r = 0; r < 2; r++)
                {
                        concurrency::parallel_for_each(
                                acc.get_default_view(),
                                concurrency::extent<1>((N - 2) / 2),
                                [=](concurrency::index<1> iR) restrict(amp) {
                                int32_t iC = iR[0] * 2 + r + 1;
                                int32_t iE = iC + 1;
                                int32_t iW = iC - 1;
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                        });
                }
        }

        auto t2 = std::chrono::high_resolution_clock::now();
        aA.synchronize();


        realp res = aA[N / 4];
        auto t4 = std::chrono::high_resolution_clock::now();

        double dt1 = (double)std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000000;
        double dt2 = (double)std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000;
        double dt4 = (double)std::chrono::duration_cast<std::chrono::microseconds>(t4 - t1).count() / 1000000;

        cout << "Result = " << res << "\n";
        cout << "init = " << dt1 << "[s]\n";
        cout << "dt2 = " << dt2 << "[s]\n";
        cout << "calc = " << dt4 << "[s]\n";

        cout << "Acc\n";
}

void cpustd()
{
        cout << "by raw cpu with vector\n";


        std::vector<realp> *pvA;

        pvA = new std::vector<realp>(N);

        auto t0 = std::chrono::high_resolution_clock::now();
        std::vector<realp>& aA = *pvA;
        //initialization.
        std::mt19937 mt(19);
        std::uniform_real_distribution<realp> dist(-1, 1);
        for (int32_t i = 0; i < N; i++)
        {
                aA[i] = dist(mt);
        }
        aA[0] = 0;
        aA[N - 1] = 1;

        auto t1 = std::chrono::high_resolution_clock::now();

        for (int32_t cT = 0; cT < nT; cT++)
        {
                for (int32_t r = 0; r < 2; r++)
                {
                        for (int32_t i = 0; i<(N - 2) / 2; i++)
                        {
                                int32_t iC = i * 2 + r + 1;
                                int32_t iE = iC + 1;
                                int32_t iW = iC - 1;
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                                aA[iC] = aA[iC] + 0.1*(aA[iE] - 2 * aA[iC] + aA[iW]);
                        }
                }
        }
        auto t2 = std::chrono::high_resolution_clock::now();


        realp res = aA[N / 4];
        auto t4 = std::chrono::high_resolution_clock::now();

        double dt1 = (double)std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count() / 1000000;
        double dt2 = (double)std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count() / 1000000;
        double dt4 = (double)std::chrono::duration_cast<std::chrono::microseconds>(t4 - t1).count() / 1000000;

        cout << "Result = " << res << "\n";
        cout << "init = " << dt1 << "[s]\n";
        cout << "dt2 = " << dt2 << "[s]\n";
        cout << "calc = " << dt4 << "[s]\n";
}
