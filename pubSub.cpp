#include<iostream>
#include<unordered_map>
#include<mutex>
#include<thread>
using namespace std;

mutex g_cout_mutex;

class pubSub{
    unordered_map<string,unordered_map<string,function<void(string)>>> subscriptions;
    mutex pub_sub_gaurd;

    public:
    void subscribe(string topicId, string subscriber_id, function<void(string)>  handler_function){
        lock_guard<mutex> lk(pub_sub_gaurd);
        auto &subscribers_and_handler_functions = subscriptions[topicId];
        if(subscribers_and_handler_functions.find(subscriber_id)!= subscribers_and_handler_functions.end()) {
            cout<<" User already subscribed to this topic "<<endl;
            return;
        }

        subscribers_and_handler_functions[subscriber_id] = handler_function;
    }

    void unsubscribe(string topicId, string subscriber_id){
        lock_guard<mutex> lk(pub_sub_gaurd);
        if(subscriptions.find(topicId)!= subscriptions.end()) {
            auto &subscribers_and_handler_functions = subscriptions[topicId];
            if(subscribers_and_handler_functions.find(subscriber_id) == subscribers_and_handler_functions.end()) {
                cout<<" Subscriber -> "<<subscriber_id<<" is not subscribed to topic -> "<<topicId<<endl;
                return;
            }

            subscribers_and_handler_functions.erase(subscriber_id);
            cout<<" Subscriber -> "<<subscriber_id<<" is successfully unsubscribed to topic -> "<<topicId<<endl;
            return;
        }
        else{
            cout<<"sorry no such topic exists"<<endl;
        }
    }

    void publish(string topicId, string message) {
    unordered_map<string, function<void(string)>> localCopy;

    {  // scoped locking block
        lock_guard<mutex> lk(pub_sub_gaurd);
        if (subscriptions.find(topicId) == subscriptions.end()) return;
        localCopy = subscriptions[topicId];  // 💡 Copy the entire map
    }

    // 🔓 Lock is released here

    for (auto& subscriber : localCopy) {
        subscriber.second(message);  // ✅ Safe to call handlers now
    }
}

};

void myCallback(const string& message) {
    lock_guard<mutex> lk(g_cout_mutex);
    cout << "Received: " << message <<endl;
}

int main(){
    pubSub p_s_system;
    string topic1 = "weather", topic2="food";
    string name1 = "sonia", name2 = "shivam" , name3 = "astha";
    string message1 = "i have eaten", message2 = " its raining";
    thread t1(pubSub::subscribe,&p_s_system,topic1,name1,myCallback);
    thread t6(pubSub::unsubscribe,&p_s_system,topic2,name1);
    thread t2(pubSub::subscribe,&p_s_system,topic1,name2,myCallback);
    thread t5(pubSub::publish,&p_s_system,topic1,message1);
    thread t3(pubSub::subscribe,&p_s_system,topic2,name1,myCallback);
    thread t4(pubSub::subscribe,&p_s_system,topic2,name2,myCallback);
    thread t7(pubSub::unsubscribe,&p_s_system,topic2,name1);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    t6.join();
    t7.join();
    return 0;
}






#include <iostream>
#include <unordered_map>
#include <queue>
#include <string>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
using namespace std;

class Subscriber {
    string id;
    function<void(string)> handler;
    queue<string> messageQueue;
    mutex mtx;
    condition_variable cv;
    thread worker;
    bool active;

public:
    Subscriber(string id, function<void(string)> handler) : id(id), handler(handler), active(true) {
        // Start worker thread
        worker = thread(&Subscriber::processMessages, this);
    }

    void receiveMessage(string msg) {
        unique_lock<mutex> lk(mtx);
        messageQueue.push(msg);
        cv.notify_one();
    }

    void processMessages() {
        while (active) {
            unique_lock<mutex> lk(mtx);
            cv.wait(lk, [&]() { return !messageQueue.empty() || !active; });

            while (!messageQueue.empty()) {
                string msg = messageQueue.front();
                messageQueue.pop();
                lk.unlock();

                // Process outside lock
                handler(msg);

                lk.lock();
            }
        }
    }

    void shutdown() {
        {
            unique_lock<mutex> lk(mtx);
            active = false;
            cv.notify_all();
        }
        if (worker.joinable())
            worker.join();
    }

    ~Subscriber() {
        shutdown();
    }
};

class PubSub {
    unordered_map<string, unordered_map<string, Subscriber*>> topicSubscribers;
    mutex mtx;

public:
    void subscribe(string topic, string subscriberId, function<void(string)> handler) {
        lock_guard<mutex> lk(mtx);
        if (topicSubscribers[topic].count(subscriberId)) {
            cout << subscriberId << " is already subscribed to " << topic << endl;
            return;
        }
        topicSubscribers[topic][subscriberId] = new Subscriber(subscriberId, handler);
    }

    void unsubscribe(string topic, string subscriberId) {
        lock_guard<mutex> lk(mtx);
        if (topicSubscribers[topic].count(subscriberId)) {
            topicSubscribers[topic][subscriberId]->shutdown();
            delete topicSubscribers[topic][subscriberId];
            topicSubscribers[topic].erase(subscriberId);
            cout << subscriberId << " unsubscribed from " << topic << endl;
        }
    }

    void publish(string topic, string message) {
        lock_guard<mutex> lk(mtx);
        if (!topicSubscribers.count(topic)) return;

        for (auto& [id, sub] : topicSubscribers[topic]) {
            sub->receiveMessage(message);
        }
    }

    ~PubSub() {
        // Cleanup all subscribers
        for (auto& [topic, subs] : topicSubscribers) {
            for (auto& [id, sub] : subs) {
                sub->shutdown();
                delete sub;
            }
        }
    }
};

void myCallback(const string& message) {
    cout << "[Callback] Got message: " << message << " on thread " << this_thread::get_id() << endl;
}

int main() {
    PubSub ps;

    ps.subscribe("sports", "sonia", myCallback);
    ps.subscribe("sports", "shivam", myCallback);
    ps.subscribe("weather", "astha", myCallback);

    ps.publish("sports", "India won the match!");
    ps.publish("weather", "Rain expected tomorrow.");
    ps.publish("sports", "Kohli scored a century!");

    this_thread::sleep_for(chrono::seconds(1));

    ps.unsubscribe("sports", "shivam");

    ps.publish("sports", "Final score update.");
    ps.publish("weather", "Thunderstorm alert.");

    this_thread::sleep_for(chrono::seconds(2));

    return 0;
}
