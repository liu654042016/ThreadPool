#! https://zhuanlan.zhihu.com/p/510405891
# 线程池技术
## 模板
参考网站:https://www.cnblogs.com/chenleideblog/p/12915534.html
参考网站：https://www.cnblogs.com/qicosmos/p/4325949.html
### 可变模板参数的展开
展开可变模版参数函数的方法一般有两种：一种是通过递归函数来展开参数包，另外一种是通过逗号表达式来展开参数包
`template <class... T>`
`void f(T... args);`
上面的可变模版参数的定义当中，省略号的作用有两个：
1.声明一个参数包 T... args，这个参数包中可以包含 0 到任意个模板参数；
2.在模板定义的右边，可以将参数包展开成一个一个独立的参数。
#### 可变模版参数函数
```
template <class... T>
void f(T... args)
{    
    cout << sizeof...(args) << endl; //打印变参的个数
}

f();        //0
f(1, 2);    //2
f(1, 2.5, "");    //3
```
##### 递归函数方式展开参数包
通过递归函数展开参数包，需要提供一个参数包展开的函数和一个递归终止函数，递归终止函数正是用来终止递归的
```
#include <iostream>
using namespace std;
//递归终止函数
void print()
{
   cout << "empty" << endl;
}
//展开函数
template <class T, class ...Args>
void print(T head, Args... rest)
{
   cout << "parameter " << head << endl;
   print(rest...);
}


int main(void)
{
   print(1,2,3,4);
   return 0;
}
```
上例会输出每一个参数，直到为空时输出 empty。展开参数包的函数有两个，一个是递归函数，另外一个是递归终止函数，参数包 Args...在展开的过程中递归调用自己，每调用一次参数包中的参数就会少一个，直到所有的参数都展开为止，当没有参数时，则调用非模板函数 print 终止递归过程。
```
print(1,2,3,4);
print(2,3,4);
print(3,4);
print(4);
print()
```
##### 逗号表达式展开参数包
```
template <class T>
void printarg(T t)
{
   cout << t << endl;
}

template <class ...Args>
void expand(Args... args)
{
   int arr[] = {(printarg(args), 0)...};
}

expand(1,2,3,4);
```
这个例子将分别打印出 1,2,3,4 四个数字。这种展开参数包的方式，不需要通过递归终止函数，是直接在 expand 函数体中展开的，printarg 不是一个递归终止函数，只是一个处理参数包中每一个参数的函数;expand 函数中的逗号表达式：(printarg(args), 0)，也是按照这个执行顺序，先执行 printarg(args)，再得到逗号表达式的结果 0。同时还用到了 C++11 的另外一个特性——初始化列表，通过初始化列表来初始化一个变长数组，{(printarg(args), 0)...}将会展开成 ((printarg(arg1),0), (printarg(arg2),0), (printarg(arg3),0),  etc... )，最终会创建一个元素值都为 0 的数组 int arr[sizeof...(Args)]。由于是逗号表达式，在创建数组的过程中会先执行逗号表达式前面的部分 printarg(args) 打印出参数，也就是说在构造 int 数组的过程中就将参数包展开了
`d = (a = b, c); `
这个表达式会按顺序执行：b 会先赋值给 a，接着括号中的逗号表达式返回 c 的值，因此 d 将等于 c

```
template<class F, class... Args>void expand(const F& f, Args&&...args) 
{
  //这里用到了完美转发，关于完美转发，读者可以参考笔者在上一期程序员中的文章《通过 4 行代码看右值引用》
  initializer_list<int>{(f(std::forward< Args>(args)),0)...};
}
expand([](int i){cout<<i<<endl;}, 1,2,3);
```
### 条件变量
参考网站：https://blog.csdn.net/GW569453350game/article/details/51782327
参考网站 2：https://www.cnblogs.com/xiaohaigegede/p/14008121.html
参考网站 3：https://www.jianshu.com/p/c1dfa1d40f53
条件变量（condition_variable）实现多个线程间的同步操作；当条件不满足时，相关线程被一直阻塞，直到某种条件出现，这些线程才会被唤醒
#### wait() 函数
（1）wait（unique_lock <mutex>＆lck）
当前线程的执行会被阻塞，直到收到 notify 为止。
（2）wait（unique_lock <mutex>＆lck，Predicate pred）
当前线程仅在 pred=false 时阻塞；如果 pred=true 时，不阻塞。
wait（）可依次拆分为三个操作：释放互斥锁、等待在条件变量上、再次获取互斥锁

#### notify_one()
notify_one()：没有参数、没有返回值。
解除阻塞当前正在等待此条件的线程之一。如果没有线程在等待，则还函数不执行任何操作。如果超过一个，不会指定具体哪一线程。

```
#include <iostream>
#include <deque>
#include <thread>
#include <mutex>
#include <condition_variable>

std::deque<int> q;
std::mutex mu;
std::condition_variable cond;

void function_1() {
    int count = 10;
    while (count > 0) {
        std::unique_lock<std::mutex> locker(mu);
        q.push_front(count);
        locker.unlock();
        cond.notify_one();  // Notify one waiting thread, if there is one.
        std::this_thread::sleep_for(std::chrono::seconds(1));
        count--;
    }
}

void function_2() {
    int data = 0;
    while ( data != 1) {
        std::unique_lock<std::mutex> locker(mu);
        while(q.empty())
            cond.wait(locker); // Unlock mu and wait to be notified
        data = q.back();
        q.pop_back();
        locker.unlock();
        std::cout << "t2 got a value from t1: " << data << std::endl;
    }
}
int main() {
    std::thread t1(function_1);
    std::thread t2(function_2);
    t1.join();
    t2.join();
    return 0;
}
```
### std::future()
参考网站：https://murphypei.github.io/blog/2019/04/cpp-concurrent-4
参考网站：https://blog.csdn.net/c_base_jin/article/details/89761718
我们想要从线程中返回异步任务结果，一般需要依靠全局变量；从安全角度看，有些不妥；为此 C++11 提供了 std::future 类模板，future 对象提供访问异步操作结果的机制，很轻松解决从异步任务中返回结果
### std::promise()
 promise 可以通过调用 get_future() 返回一个 future 对象，让你去了解这个承诺是否完成了。因此，promise 是存放异步操作产出值的坑，而 future 是从其中获取异步操作结果，二者都是模板类型。
 ```
 struct MyData
{
	int value;
	float conf;
};
 
MyData data{0, 0.0f};
 
int main()
{
	std::promise<MyData> dataPromise;
	std::future<MyData> dataFuture = dataPromise.get_future();
 
	std::thread producer(
    [&] (std::promise<MyData> &data) -> void {
      std::this_thread::sleep_for(std::chrono::seconds(1));
      data.set_value({2, 1.0f});
    }, 
    std::ref(dataPromise)
  );
 
	std::thread consumer(
    [&] (std::future<MyData> &data) -> void {
      auto a = data.valid();
      std::cout << a << std::endl;
      auto res = data.get();
      std::cout << res.value << "\t" << res.conf << std::endl;
      auto b = data.valid();
      std::cout << b << std::endl;
    }, 
    std::ref(dataFuture)
  );
 
  producer.join();
  consumer.join();
 
  return 0;
}
 ```
 我们不用互斥量和条件变量也可以写出生产者消费者模型，生产者将结果放到 promise 中，而消费者通过 promise 这个异步操作相关联的 future 对象获取结果。另外可以看出，对于一个 future 对象，可以通过执行 valid() 检查其结果是否是共享的（但是不一定准备好了），然后调用 get() 获取结果，并且 get() 会改变其共享状态。这里需要注意的一点是，future 的 get() 方法是阻塞的，所以在与其成对的 promise 还未产出值，也就是未调用 set_value() 方法之前，调用 get() 的线程将会一直阻塞在 get() 处直到其他任何人调用了 set_value() 方法（虽然 valid() 一直是 true）。
 ### std::packaged_task
 packaged_task 是对一个任务的抽象，我们可以给其传递一个函数来完成其构造。相较于 promise，它应该算是更高层次的一个抽象了吧，同样地，我们可以将任务投递给任何线程去完成，然后通过 packaged_task::get_future() 方法获取的 future 对象来获取任务完成后的产出值。总结来说，packaged_task 是连数据操作创建都封装进去了的 promise。packaged_task 也是一个类模板，模板参数为函数签名，也就是传递函数的类型。
 ```
 struct MyData
{
	int value;
	float conf;
};
 
MyData data{0, 0.0f};
 
int main()
{
	std::packaged_task<MyData()> produceTask(
        [&] () -> MyData {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return MyData{2, 1};
        }
    );

    auto dataFuture = produceTask.get_future();
 
	std::thread producer(
        [&] (std::packaged_task<MyData()> &task) -> void {
            task();
    }, 
    std::ref(produceTask)
  );
 
	std::thread consumer(
        [&] (std::future<MyData> &data) -> void {
            auto res = data.get();
            std::cout << res.value << "\t" << res.conf << std::endl;
    },
    std::ref(dataFuture)
  );
 
  producer.join();
  consumer.join();
 
  return 0;
}

 ```
 packaged_task::valid() 可以帮忙检查当前 packaged_task 是否处于一个有效的共享状态，对于由默认构造函数生成的 packaged_task 对象，该函数返回 false，除非中间进行了 move() 赋值操作或者 swap() 操作。另外我们也可以通过 reset() 来重置其共享状态。对于我们上面创建的 producerTask 其创建之后就拥有有效的共享状态
 ### std::async
 std::async 大概的工作过程：先将异步操作用 std::packaged_task 包装起来，然后将异步操作的结果放到 std::promise 中，这个过程就是创造未来的过程。外面再通过 future.get/wait 来获取这个未来的结果。可以说，std::async 帮我们将 std::future、std::promise 和 std::packaged_task 三者结合了起来。
 ```
 struct MyData
{
	int value;
	float conf;
};
 
MyData data{0, 0.0f};
 
int main()
{
    auto start = std::chrono::steady_clock::now();
    std::future<MyData> dataFuture = std::async(std::launch::async, [] () -> MyData {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return MyData{2, 1};
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    auto res = dataFuture.get();
    std::cout << res.value << "\t" << res.conf << std::endl;

    auto end = std::chrono::steady_clock::now();
    std::cout << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << std::endl;

    return 0;
}

 ```
 async 返回一个与函数返回值相对应类型的 future，通过它我们可以在其他任何地方获取异步结果。由于我们给 async 提供了 std::launch::async 策略，所以生产过程将被异步执行，具体执行的时间取决于各种因素，最终输出的时间为 2000ms < t < 3000ms ，可见生产过程和主线程是并发执行的。除了 std::launch::async，还有一个 std::launch::deferred 策略，它会延迟线程地创造，也就是说只有当我们调用 future.get() 时子线程才会被创建以执行任务，这样输出时间应该是 t > 3000 ms 的。
