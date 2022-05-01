#include "../Mutex.h"
#include <iostream>

int main(int argc, char* argv[]) {
    muduo::MutexLock mt;
    {
        muduo::MutexLockGuard lock(mt);
        std::cout << "test MutexLockGuard" << std::endl;
    }
    return 0;
}