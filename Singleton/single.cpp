#include <iostream>
#include <mutex>
#include <atomic>

class Singleton1{
    public:
        static Singleton1* GetInstance(){
            if(_instance==nullptr){
                _instance=new Singleton1(); //P1:_instance指向内存在堆上分配，需要手动delete。
            }
            return _instance;
        } 
    private:
        Singleton1(){std::cout<<"1.\n";}
        ~Singleton1() {
            std::cout<<"delete Singleton1.\n";
        }
        Singleton1(const Singleton1&)=delete;
        Singleton1& operator=(const Singleton1&)=delete;
        Singleton1(Singleton1 &&)=delete;
        Singleton1& operator=(Singleton1 &&)=delete;

        static Singleton1* _instance;
        //P2:_instance本身在全局静态区，自动释放，导致P1无法调用析构函数。

        //P3.单例模式的场景下是否需要释放P1的资源，是否可以在执行过程中一直占用直到程序被关闭。
};
Singleton1* Singleton1::_instance=nullptr;

class Singleton2{
    public:
        static Singleton2* GetInstance(){
            if(_instance==nullptr){
                _instance=new Singleton2(); //P1:_instance指向内存在堆上分配，需要手动delete。
                atexit(Destructor);  //P4:通过exit的时候出发回调函数，调用析构函数释放new内存。
            }
            return _instance;
        } 
    private:
        static void Destructor(){
            if(nullptr!=_instance){
                delete _instance;
                _instance=nullptr;
            }
        }
        Singleton2(){std::cout<<"2.\n";}
        ~Singleton2() {
            std::cout<<"delete Singleton2.\n";
        }
        Singleton2(const Singleton2&)=delete;
        Singleton2& operator=(const Singleton2&)=delete;
        Singleton2(Singleton2 &&)=delete;
        Singleton2& operator=(Singleton2 &&)=delete;

        static Singleton2* _instance;
};
Singleton2* Singleton2::_instance=nullptr;

//1、2版本多线程程序有问题

class Singleton3{
    public:
        static Singleton3* GetInstance(){
            // std::lock_guard<std::mutex> lock(mtx); //P1.3.1版本，锁粒度粗
            if(_instance==nullptr){
                std::lock_guard<std::mutex> lock(mtx); //3.2版本，双重检验机制，粒度细，性能优于3.1
                if(_instance==nullptr){
                    _instance=new Singleton3(); 
                    //1.申请内存
                    //2.调用构造函数
                    //3.返回对象指针
                    //ps：多核心机器上面，cpu指令重排，可能顺序为1，3，2；
                    //此时其余线程直接返回指针可能导致程序出现问题。
                    //拿到非空指针，但是还没有调用构造函数。
                }
                atexit(Destructor);  
            }
            return _instance;
        } 
    private:
        static void Destructor(){
            if(nullptr!=_instance){
                delete _instance;
                _instance=nullptr;
            }
        }
        Singleton3(){std::cout<<"3.\n";}
        ~Singleton3() {
            std::cout<<"delete Singleton3.\n";
        }
        Singleton3(const Singleton3&)=delete;
        Singleton3& operator=(const Singleton3&)=delete;
        Singleton3(Singleton3 &&)=delete;
        Singleton3& operator=(Singleton3 &&)=delete;

        static Singleton3* _instance;
        static std::mutex mtx;
};
Singleton3* Singleton3::_instance=nullptr;
std::mutex Singleton3::mtx;
//多cpu下指令重排

class Singleton4 {
public:
    static Singleton4 * GetInstance() {
        // Singleton4* tmp = _instance.load(std::memory_order_relaxed);
        // std::atomic_thread_fence(std::memory_order_acquire);
        Singleton4* tmp=_instance.load(std::memory_order_acquire);
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(_mutex);
            tmp = _instance.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new Singleton4;
                // std::atomic_thread_fence(std::memory_order_release);
                // _instance.store(tmp, std::memory_order_relaxed);
                _instance.store(tmp,std::memory_order_release);
            }
            atexit(Destructor);
        }
        return tmp;
    }

private:
    static void Destructor() {
        Singleton4* tmp = _instance.load(std::memory_order_relaxed);
        if (nullptr != tmp) {
            delete tmp;
        }
    }
    Singleton4(){std::cout<<"4.\n";}
    ~Singleton4() {
        std::cout << "~Singleton4()\n";
    }

    Singleton4(const Singleton4 &) = delete;
    Singleton4& operator=(const Singleton4&) = delete;
    Singleton4(Singleton4 &&) = delete;
    Singleton4& operator=(Singleton4 &&) = delete;

    static std::atomic<Singleton4*> _instance;
    static std::mutex _mutex;
};
std::atomic<Singleton4*> Singleton4::_instance=nullptr;
std::mutex Singleton4::_mutex;

//天然线程安全版本，C++11及以上
class Singleton5{
public:
    static Singleton5* GetInstance(){
        static Singleton5 _instance;
        return &_instance;
    }
private:
    Singleton5(){std::cout<<"5.\n";}
    ~Singleton5(){
        std::cout<<"~Singleton5().\n";
    }

    Singleton5(const Singleton5&)=delete;
    Singleton5(Singleton5&&)=delete;
    Singleton5& operator= (const Singleton5&)=delete;
    Singleton5& operator= (Singleton5&&)=delete;

    // static Singleton5 _instance;
};

//封装单例模板
template<typename T>
class Singleton{
public:
    static T* GetInstance(){
        static T instance;
        return &instance;
    }
protected:
    Singleton(){
        std::cout<<"template father generate.\n";
    };
    virtual ~Singleton(){
        std::cout<<"~Singleton().\n";
    }
private:
    Singleton(const Singleton&)=delete;
    Singleton& operator= (const Singleton&)=delete;
    Singleton(Singleton&&)=delete;
    Singleton operator= (Singleton&&)=delete;
};

class DesignPatten :public Singleton<DesignPatten>{
    friend class Singleton<DesignPatten>;
public:
//...
private:
    DesignPatten(){std::cout<<"DesignPatten.\n";}
    ~DesignPatten(){
        std::cout<<"~DesignPatten().\n";
    }
};

int main(){
    //版本1.堆上资源不能正常析构
    Singleton1 *s1=Singleton1::GetInstance();
    //版本2.堆上资源正常析构，多线程版本不适用
    Singleton2 *s2=Singleton2::GetInstance();
    //版本3.双检查锁，多cpu下指令重排可能造成内存泄漏
    Singleton3 *s3=Singleton3::GetInstance();
    //版本4.线程安全 原子操作+内存屏障+互斥锁
    Singleton4 *s4=Singleton4::GetInstance();
    //版本5. c++11，天然线程安全
    Singleton5 *s5=Singleton5::GetInstance();
    //版本6. 封装模板类
    DesignPatten *d1=DesignPatten::GetInstance();

    return 0;
}