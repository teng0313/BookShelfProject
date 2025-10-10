
#include <chrono>
#include <iostream>
class Timer
{
    public:
        Timer()
        {
            start = std::chrono::high_resolution_clock::now();
        };
        Timer(std::string task_name) : task_name_(task_name)
        {
            start = std::chrono::high_resolution_clock::now();
        };
        ~Timer(){
            elapsed = std::chrono::high_resolution_clock::now() - start;
            std::cout << task_name_ <<"Test time: " << elapsed.count() << " s\n";
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        std::chrono::duration<double> elapsed;
        std::string task_name_ {};
};