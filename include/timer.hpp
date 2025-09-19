
#include <chrono>
#include <iostream>
class Timer
{
    public:
        Timer()
        {
            start = std::chrono::high_resolution_clock::now();
        };
        ~Timer(){
            elapsed = std::chrono::high_resolution_clock::now() - start;
            std::cout << "Test time: " << elapsed.count() << " s\n";
        }
    private:
        std::chrono::time_point<std::chrono::high_resolution_clock> start;
        std::chrono::duration<double> elapsed;
};