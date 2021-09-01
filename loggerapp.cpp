#include <iostream>
#include <thread>

#include "logger.h"

void myFunc2()
{
    LOG_ERROR("Hello from myFunc2!");
    LOG_INFO("Hello from myFunc2!");

}

void myFunc1()
{
    LOG_INFO("Hello from myFunc1!");
    myFunc2();
    LOG_INFO("Hello from myFunc1! [", "OK", ']' );
}

int main()
{
    LOG_WARN("Hello from main [", 42, ']');
    std::thread t1(myFunc1);
    std::thread t2(myFunc1);

    t2.join();
    t1.join();
}
