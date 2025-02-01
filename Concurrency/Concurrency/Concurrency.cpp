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
class MasterControl {
public:
    MasterControl(int workerCount):lk{mtx},workerCount{workerCount}{}
    void SignalDone() {
        {
            std::lock_guard lk{ mtx };
            ++doneCount;
        }
        if (doneCount == workerCount)
        {
            cv.notify_one();
        }
    }
    void WaitForAllDone()
    {
        cv.wait(lk, [this] {return doneCount == workerCount;});
        doneCount = 0;
    }
private:
    std::condition_variable cv;
    std::mutex mtx;
    std::unique_lock<std::mutex>lk;
    int workerCount;
    //shared memory 
    int doneCount = 0;
};
class Worker
{
public:
    Worker(MasterControl* pMaster) :pMaster{ pMaster }, thread{&Worker::Run_, this
//when we want to run a thread on the member function so we have to pass
// first pointer to the memeber function 
//second enstance of the member function 
}{}
    void SetJob(std::span<int>data, int* pOut) {
        {
            std::lock_guard lk{ mtx };
            input = data;
            pOutput = pOut;
        }
        cv.notify_one();
}
    void Kill()
    {
        {
            std::lock_guard lk{ mtx };
            dying = true;
        }
        cv.notify_one();
    }

private:
    void Run_()
    {
        std::unique_lock lk{ mtx };// we are locking the mutex;
        //to prevent the multiple excess
        while (true)// we are running to ensure worker thread is always ready for new work 
        {
            cv.wait(lk, [this] {return pOutput != nullptr || dying;});
            // we have to wait for newwork until we get it 
            //pOutput= true means we get new work 
            // dying is true means now we donot have any work just die the thread 

            if (dying)
            {
                break;
            }
            ProcessDataset(input, *pOutput,mtx);//process the curring data which we get it
            pOutput = nullptr;// make the output is nullptr that we donot have new work
            input = {};//make input empty
            pMaster->SignalDone();//mark the main thread that we are done 
        }
    }
    MasterControl* pMaster;
    std::jthread thread;
    std::condition_variable cv;
    std::mutex mtx;
    //shared memory 
    std::span<int> input;
    int* pOutput = nullptr;
    bool dying = false;

};
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
    constexpr size_t workerCount = 4;
    MasterControl mctrl{ workerCount };
    std::vector<std::unique_ptr<Worker>> workerPtrs;
    for (size_t i = 0;i < workerCount;i++)
    {
        workerPtrs.push_back(std::make_unique<Worker>(&mctrl));
    }
    for(size_t i = 0;i < DATASET_SIZE;i+=subsetSize)
    {
        for (int j = 0;j < 4;j++)
        {
            workerPtrs[j]->SetJob(std::span{ &datasets[j][i],subsetSize }, &sum[j].v);
          /*  workers.push_back(std::jthread{ ProcessDataset, std::span{&datasets[j][i],subsetSize},std::ref(sum[j].v),std::ref(mtx)});*/
        }
        mctrl.WaitForAllDone();
        //when we clear then threads get distroyed and jthread distructor will be called 
        // we do not have to join
        

    }
    grandTotal = sum[0].v + sum[1].v + sum[2].v + sum[3].v;
    for (auto& w : workerPtrs)
    {
        w->Kill();
    }
    //workerPtrs.clear();
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