#include <iostream>

class Singleton1{
    public:
        static Singleton1* GetInstance(){
            if(_instance==nullptr){
                _instance=new Singleton1(); //P1:_instance指向内存在堆上分配，需要手动delete。
            }
            return _instance;
        } 
    private:
        Singleton1(){}
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
        Singleton2(){}
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