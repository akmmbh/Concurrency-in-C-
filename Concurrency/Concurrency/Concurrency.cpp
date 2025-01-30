// Concurrency.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <iostream>
#include<vector>
#include<array>
#include<random>
#include<ranges>
#include<thread>
#include<cmath>
#include<limits>
#include<mutex>
constexpr size_t DATASET_SIZE = 5000000; 
void ProcessDataset(std::array<int, DATASET_SIZE>& set , int &sum,std::mutex&mtx)
{
    for (int x : set)
    {
           //sometimes you forget to unlock the mutex then you
        // should use the 
        // std::lock_guard g{mtx} 
        // so you not have to unlock it when it goes out of scope it automatically unlock it

      //  mtx.lock();
        std::lock_guard g{ mtx };
        constexpr auto limit = (double)std::numeric_limits<int>::max();
        const auto y = (double)x / limit;
        
        sum= int(std::sin(std::cos(y)) * limit);
       // mtx.unlock();
    }
}
int main()
{
    std::minstd_rand rne;
    std::vector<std::array<int, DATASET_SIZE>> datasets{ 4 };
    std::vector<std::thread> workers;

    for (auto& arr : datasets) {
        std::ranges::generate(arr, rne);
    }
    int sum[] = { 0,0,0,0, };

    std::mutex mtx;
    for (size_t i =0;i<4 ;i++)
    {/*  lamda function 
        auto w = [&set] {
            for (int x : set)
            {
                constexpr auto limit = (double)std::numeric_limits<int>::max();
                const auto y = (double)x / limit;
                set[0] += int(std::sin(std::cos(y)) * limit);
            }
            };*/
        workers.push_back(std::thread{ ProcessDataset, std::ref(datasets[i]),std::ref(sum[i]),std::ref(mtx)});
        // we cannot pass by reference directy if we have to pass by reference in thread
        // we have to create a wrapper of reference around it;

        
    }
    // agar hum ye make sure nhi karte ki apna kam khatm karenge tho tho ye gadbad hai 
    // fir tho humara program unko kam dene ke baad hi close ho jayega 
    // kyunki destructor tho chalu hua hi nhi hua hai
    for (auto& w : workers) {
        w.join();
    }
    //join ak tarekhe se wait karta ahi threaed ko compelte hone ka
    // for multithreading we have some work to multithread
    std::cout << "Hello World!\n"<<sum[0]+ sum[1]+ sum[2]+ sum[3];
}
