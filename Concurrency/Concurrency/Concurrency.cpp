// Concurrency.cpp : This file contains the 'main' function. Program execution begins and ends there.


#include <iostream>
#include<vector>
#include<array>
#include<random>
#include<ranges>
#include<thread>
#include<cmath>
#include<limits>
#include<span>
#include<mutex>
constexpr size_t DATASET_SIZE = 50000000; 
void ProcessDataset(std::span<int, DATASET_SIZE> set , int &sum,std::mutex&mtx)
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
        
        sum+= int(std::sin(std::cos(y)) * limit);
       // mtx.unlock();
    }
}
std::vector<std::array<int, DATASET_SIZE>> GenerateDatasets() {
    std::minstd_rand rne;
    std::vector<std::array<int, DATASET_SIZE>> datasets{ 4 };
    

    for (auto& arr : datasets) {
        std::ranges::generate(arr, rne);
    }
    return datasets;
}
int DoBiggie()
{
    auto datasets = GenerateDatasets();
  std::vector<std::thread> workers;
    struct Value
    {
        int v = 0;
        char padding[64];
    };
    Value sum[4];

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
        workers.push_back(std::thread{ ProcessDataset, std::span(datasets[i]),std::ref(sum[i].v),std::ref(mtx)});
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
    std::cout << "Hello World!\n"<<sum[0].v+ sum[1].v+ sum[2].v+ sum[3].v;
}
int DoSmallies() {
    auto datasets = GenerateDatasets();
    std::vector<std::jthread> workers;
    struct Value
    {
        int v = 0;
        char padding[64];
    };
    Value sum[4];
    int grandTotal = 0;
    std::mutex mtx;
    constexpr const auto subsetSize = DATASET_SIZE / 10000;
    for(size_t i = 0;i < DATASET_SIZE;i+=subsetSize)
    {
        for (int j = 0;j < 4;j++)
        {
            workers.push_back(std::jthread{ ProcessDataset, std::span{&datasets[j][i],subsetSize},std::ref(sum[j].v),std::ref(mtx)});
        }
        workers.clear();
        //when we clear then threads get distroyed and jthread distructor will be called 
        // we do not have to join
        grandTotal = sum[0].v + sum[1].v + sum[2].v + sum[3].v;
    }
    return 0;
}
int main(int argc, char** argv) {
    if (argc > 1 && std::string{ argv[1] } == "--smol")
    {
        return DoSmallies();
        //when we are breaking into parts 
        // and we are increasing the no of parts they are breaking 
        // the time of excution get increases as we increases the parts we are dividing
        //reaason for this is we are creating a lots of thread and creating thread is expensive operation
        //it call the operating system and it hast to create the context of the thread
        // that include the allocating the memory for that thread stack every every thread
         
    }
    return DoBiggie();
}