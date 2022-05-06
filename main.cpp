/*
 * @Author: LIU KANG
 * @Date: 2022-05-05 18:21:52
 * @LastEditors: LIU KANG
 * @LastEditTime: 2022-05-06 20:53:17
 * @FilePath: \ThreadPool\main.cpp
 * @Description: thread pool demo
 * 
 * Copyright (c) 2022 by 用户/公司名，All Rights Reserved. 
 */
#include <iostream>
//#include"example"
#include "ThreadPool.h"

#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

//demo 1
int main(int, char**) {
    std::cout << "Hello, world!\n";

    ThreadPool pool(4);
    std::vector< std::future<int> > results;

    for(int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::cout << "hello " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "world " << i << std::endl;
                return i*i;
            })
        );
    }

    for(auto && result: results)
        std::cout << result.get() << ' ';
    std::cout << std::endl;
    

    std::cout<<"111111111111111111"<<std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(5));
  // system(Sleep(5000));
    return 0;

}
/////////////demo2//////////////////////

// std::deque<int> q;
// std::mutex mu;
// std::condition_variable cond;

// void function_1() {
//     int count = 10;
//     while (count > 0) {
//         std::unique_lock<std::mutex> locker(mu);
//         q.push_front(count);
//         locker.unlock();
//         cond.notify_one();  // Notify one waiting thread, if there is one.
//         std::this_thread::sleep_for(std::chrono::seconds(1));
//         count--;
//     }
// }

// void function_2() {
//     int data = 0;
//     while ( data != 1) {
//         std::unique_lock<std::mutex> locker(mu);
//         while(q.empty())
//             cond.wait(locker); // Unlock mu and wait to be notified
//         data = q.back();
//         q.pop_back();
//         locker.unlock();
//         std::cout << "t2 got a value from t1: " << data << std::endl;
//     }
// }
// int main() {
//     std::cout<<"hello"<<std::endl;
//     std::thread t1(function_1);
//     std::thread t2(function_2);
//     t1.join();
//     t2.join();
//     std::cout<<"end"<<std::endl;
//     return 0;
// }