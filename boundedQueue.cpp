#include<iostream>
#include<queue>
#include<mutex>
#include<thread>
using namespace std;

mutex g_cout_mutex;

template <typename T>
class boundedQueue{
    queue<T> bounded_Queue;
    mutex queue_guard;
    condition_variable cenq;
    condition_variable cdeq;
    int capacity;
    bool shutDown;

    public:

    boundedQueue(int capacity){
        if(capacity<=0) throw invalid_argument("invalid argument";)
        this->capacity = capacity;
        shutDown = false;
    }

    void enqueue(T input){
        unique_lock<mutex> enqLock(queue_guard);
        cenq.wait(enqLock, [this]{ return this->bounded_Queue.size() < capacity || shutDown; });
        if(shutDown) return;
        bounded_Queue.push(input);
        {
            unique_lock<mutex> cout_lock(g_cout_mutex);
            cout<<" Successfully enqueued "<<endl;
        }
        cdeq.notify_one();
    }

    T dequeue(){
        unique_lock<mutex> deqLock(queue_guard);
        cdeq.wait(deqLock, [this]{ return !this->bounded_Queue.empty() || shutDown; });
        if(shutDown && this->bounded_Queue.empty()) throw runtime_error("Queue shut down");
        T frontValue = bounded_Queue.front();
        bounded_Queue.pop();
        {
            unique_lock<mutex> cout_lock(g_cout_mutex);
            cout<<" Successfully dequeued "<<endl;
        }
        cenq.notify_one();
        return frontValue;
    }

    void shutdown(){
        unique_lock<mutex> shutdownlock(queue_guard);
        shutDown = true;
        cenq.notify_all();
        cdeq.notify_all();
    }

    ~boundedQueue(){
        shutdown();
    }

};

void producer(boundedQueue<int> &q, int start){
    for(int i=0;i<10;i++){
        q.enqueue(i+start);
        this_thread::sleep_for(chrono::microseconds(100));
    }
}

void consumer(boundedQueue<int> &q){
    try{
        for(int i=0;i<10;i++){
            int val = q.dequeue();
            this_thread::sleep_for(chrono::microseconds(100));
        }
    }
    catch(exception &e){
        unique_lock<mutex> cout_lock(g_cout_mutex);
        cout<< "[consumer ] exception: "<<e.what()<<endl;
    }

}

int main(){
    boundedQueue<int> q(20);

    thread t1(producer,ref(q),100);
    thread t2(consumer,ref(q));
    t1.join();
    t2.join();
    this_thread::sleep_for(chrono::milliseconds(100));
    thread t3(&boundedQueue<int>::shutdown,&q);
    t3.join();
    return 0;
}