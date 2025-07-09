#include<iostream>
#include<unordered_map>
#include<mutex>
#include<thread>
using namespace std;

mutex g_cout_mutex;

class userProperties{
    string userId;
    int requestRate;
    int window_size;
    int currentReqCnt;
    int prevReqCnt;
    int64_t startCurrentWindow;
    int64_t startPrevWindow;
    mutex rateEntityGuard;

    double calculateOverlap(int64_t currentRequestTime){
        int elapsed_time_in_currentWindow, overlap_time_in_previous_window;
        double overlap_Ratio;
        elapsed_time_in_currentWindow = currentRequestTime - startCurrentWindow;
        overlap_time_in_previous_window = window_size - elapsed_time_in_currentWindow;
        overlap_Ratio = (double)overlap_time_in_previous_window/window_size;
        return overlap_Ratio;
    }

    double calculateTotalRequests(double overlap_Ratio){
        double total = currentReqCnt+prevReqCnt*overlap_Ratio;
        return total;
    }

    bool checkLimit(double total_requests){
      return total_requests+1<=requestRate;
    }

    void resetForNextWindow(int64_t currentRequestTime){
        int64_t newStartCurrentWindow = startCurrentWindow + ((currentRequestTime - startCurrentWindow) / window_size) * window_size;
        double skippedWindows = (currentRequestTime - startCurrentWindow)/window_size;
        if(skippedWindows >= 2) {
            prevReqCnt = 0;
            startPrevWindow = 0;
        }
        else if(skippedWindows >= 1) {
            prevReqCnt = currentReqCnt;
            startPrevWindow = startCurrentWindow;
        }

        currentReqCnt = 0;
        startCurrentWindow = newStartCurrentWindow;
    }

    public:
    userProperties(string userId, int64_t currentTimestamp, int requestLimit, int window_size){
        this->userId = userId;
        this->window_size = window_size;
        this->requestRate = requestLimit;
        this->currentReqCnt = 0;
        this->prevReqCnt = 0;
        this->startCurrentWindow = currentTimestamp;
        this->startPrevWindow = 0;
    }

    bool processUserReqAndAcquire(int64_t currentRequestTime){
        lock_guard<mutex> lk(rateEntityGuard);
        if(currentRequestTime > startCurrentWindow+window_size){
            resetForNextWindow(currentRequestTime);
        }
        double overlap_Ratio = calculateOverlap(currentRequestTime);
        double total_requests = calculateTotalRequests(overlap_Ratio);
        bool isAvailable =  checkLimit(total_requests);
        if(isAvailable) currentReqCnt++;
        return isAvailable;
    }

    ~userProperties(){

    }
};

class rateLimitManager{
    int requestRate;
    int window_size;
    mutex limitManagerGuard;
    unordered_map<string, userProperties *> rateLimitUsers;

    void addUser(string userId, int64_t currentTimestamp){
        userProperties *user = new userProperties(userId, currentTimestamp, this->window_size, this->requestRate);
        rateLimitUsers[userId] = user;
    }

    public:

    rateLimitManager(int window_size, int requestLimit){
        this->requestRate = requestLimit;
        this->window_size = window_size;
    }

    void processRequest(string userId, int64_t currentTimestamp){
        userProperties *user = nullptr;
        
        {   lock_guard<mutex> lk(limitManagerGuard);
            if(rateLimitUsers.find(userId) == rateLimitUsers.end()){
                addUser(userId, currentTimestamp);
            }

            user = rateLimitUsers[userId];
        }
            

        bool isRequestAllowed = user->processUserReqAndAcquire(currentTimestamp);

        lock_guard<mutex> cout_lk(g_cout_mutex);
        if(isRequestAllowed) cout<<" Request Allowed! "<<endl;
        else cout<<"Sorry! Request not allowed at this moment. Please try later!"<<endl;

    }

};

int main(){
    rateLimitManager manager(60, 10);
    std::int64_t currentTimestamp = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    string s1 = "sonia", s2 = "shivam", s3 = "astha";
    thread t1(&rateLimitManager::processRequest, &manager,s1,currentTimestamp);
    thread t2(&rateLimitManager::processRequest, &manager,s1,currentTimestamp);
    currentTimestamp = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    thread t3(&rateLimitManager::processRequest, &manager,s1,currentTimestamp);
    thread t4(&rateLimitManager::processRequest, &manager,s2,currentTimestamp);
    currentTimestamp = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
    thread t5(&rateLimitManager::processRequest, &manager,s3,currentTimestamp);
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    t5.join();
    return 0;
}




// #include <iostream>
// #include <unordered_map>
// #include <mutex>
// #include <thread>
// #include <chrono>   // Required for std::chrono
// #include <memory>   // Required for std::unique_ptr
// #include <algorithm> // Required for std::max

// using namespace std;

// // Global mutex for cout, as discussed
// mutex g_cout_mutex;

// class userProperties {
// private:
//     string userId;
//     int requestRate;   // Max requests allowed within window_size
//     int window_size;   // Time window in milliseconds

//     int currentReqCnt;    // Count of requests in the current fixed window
//     int prevReqCnt;       // Count of requests in the previous fixed window

//     int64_t startCurrentWindow; // Start timestamp of the current fixed window
//     int64_t startPrevWindow;    // Start timestamp of the previous fixed window

//     mutex rateEntityGuard; // Mutex to protect this user's properties

//     // Calculates the overlap ratio for the sliding window counter
//     double calculateOverlapRatio(int64_t currentRequestTime) {
//         // The sliding window is [currentRequestTime - window_size, currentRequestTime]
//         // The previous fixed window is [startPrevWindow, startPrevWindow + window_size)

//         // Calculate the end time of the previous fixed window
//         int64_t prevWindowEndTime = startPrevWindow + window_size;

//         // Calculate the start time of the current sliding window
//         int64_t slidingWindowStartTime = currentRequestTime - window_size;

//         // The overlap is the intersection of [slidingWindowStartTime, currentRequestTime]
//         // and [startPrevWindow, prevWindowEndTime]
        
//         // Overlap duration is max(0, prevWindowEndTime - slidingWindowStartTime)
//         // If slidingWindowStartTime is >= prevWindowEndTime, there's no overlap or negative overlap.
//         int64_t overlapDuration = prevWindowEndTime - slidingWindowStartTime;
        
//         // Ensure overlap is not negative (if previous window has completely passed the current sliding window start)
//         overlapDuration = std::max(0LL, overlapDuration); // Use 0LL for int64_t literal

//         return (double)overlapDuration / window_size;
//     }

//     // Calculates the total estimated requests in the current sliding window
//     double calculateTotalRequests(double overlap_Ratio) {
//         // This is the core formula for the sliding window counter:
//         // current_requests + (previous_requests * overlap_ratio_with_current_sliding_window)
//         double total = currentReqCnt + prevReqCnt * overlap_Ratio;
//         return total;
//     }

//     // Checks if allowing one more request would exceed the limit
//     bool checkLimit(double total_requests) {
//         return total_requests + 1 <= requestRate;
//     }

//     // Resets counters and window start times when a new fixed window is entered
//     void resetForNextWindow(int64_t currentRequestTime) {
//         // Calculate the start of the fixed window that currentRequestTime falls into
//         // Using integer division to snap to the start of the window
//         int64_t targetCurrentWindowStart = (currentRequestTime / window_size) * window_size;

//         // How many full fixed windows have passed since the last startCurrentWindow
//         int64_t windowsPassed = (targetCurrentWindowStart - startCurrentWindow) / window_size;

//         // This function is only called when currentRequestTime > startCurrentWindow + window_size,
//         // so windowsPassed will always be >= 1.
        
//         if (windowsPassed >= 2) {
//             // If two or more full windows have been skipped, the previous window's count is irrelevant.
//             prevReqCnt = 0;
//             startPrevWindow = targetCurrentWindowStart - window_size; // Set to the window immediately preceding the new current
//         } else { // Exactly one full window has passed (windowsPassed == 1)
//             prevReqCnt = currentReqCnt; // The current window's count becomes the previous window's count
//             startPrevWindow = startCurrentWindow; // The current window's start becomes the previous window's start
//         }

//         currentReqCnt = 0; // Reset current window count for the new fixed window
//         startCurrentWindow = targetCurrentWindowStart; // Move current window start to the new target
//     }

// public:
//     // Constructor
//     userProperties(string userId, int64_t initialTimestamp, int requestLimit, int window_size) {
//         this->userId = userId;
//         this->window_size = window_size;
//         this->requestRate = requestLimit;
//         this->currentReqCnt = 0;
//         this->prevReqCnt = 0;
//         // Initialize startCurrentWindow to the start of the fixed window containing initialTimestamp
//         this->startCurrentWindow = (initialTimestamp / window_size) * window_size;
//         this->startPrevWindow = this->startCurrentWindow - window_size; // Previous window starts just before
//     }

//     // Process a user request and determine if it's allowed
//     bool processUserReqAndAcquire(int64_t currentRequestTime) {
//         lock_guard<mutex> lk(rateEntityGuard); // Lock this user's data

//         // If the current request falls into a new fixed window (or multiple new windows)
//         if (currentRequestTime >= startCurrentWindow + window_size) {
//             resetForNextWindow(currentRequestTime);
//         }
        
//         // Calculate overlap based on the current state (after potential reset)
//         double overlap_Ratio = calculateOverlapRatio(currentRequestTime);
//         double total_requests = calculateTotalRequests(overlap_Ratio);
        
//         bool isAvailable = checkLimit(total_requests);
        
//         if (isAvailable) {
//             currentReqCnt++; // Increment count for the current fixed window
//         }
//         return isAvailable;
//     }

//     // Destructor (unique_ptr handles deletion, so no manual delete calls needed here)
//     ~userProperties() {
//         // You could add a print statement here to confirm object destruction
//         // lock_guard<mutex> cout_lk(g_cout_mutex);
//         // cout << "User " << userId << " properties destroyed." << endl;
//     }
// };

// class rateLimitManager {
// private:
//     int requestRate;
//     int window_size;
//     mutex limitManagerGuard; // Mutex to protect the unordered_map

//     // Use unique_ptr to manage userProperties objects and prevent memory leaks
//     unordered_map<string, unique_ptr<userProperties>> rateLimitUsers;

//     // Helper function to add a new user
//     void addUser(string userId, int64_t currentTimestamp) {
//         // Pass the currentTimestamp to userProperties constructor to correctly set initial window boundaries
//         rateLimitUsers[userId] = make_unique<userProperties>(userId, currentTimestamp, this->requestRate, this->window_size);
//     }

// public:
//     // Constructor
//     rateLimitManager(int window_size, int requestLimit) {
//         this->requestRate = requestLimit;
//         this->window_size = window_size;
//     }

//     // Destructor to ensure all unique_ptr objects are correctly deallocated
//     // The unique_ptr in the map will handle the 'delete' automatically when the map clears.
//     ~rateLimitManager() {
//         lock_guard<mutex> lk(limitManagerGuard); // Lock map during cleanup
//         rateLimitUsers.clear(); // This will destroy all unique_ptr objects, calling userProperties destructors
//         // lock_guard<mutex> cout_lk(g_cout_mutex);
//         // cout << "RateLimitManager destroyed. All user properties deallocated." << endl;
//     }

//     // Main function to process an incoming request
//     void processRequest(string userId, int64_t currentTimestamp) {
//         userProperties *user = nullptr;

//         { // Critical section for map access (short-lived lock)
//             lock_guard<mutex> lk(limitManagerGuard); 
//             if (rateLimitUsers.find(userId) == rateLimitUsers.end()) {
//                 addUser(userId, currentTimestamp);
//             }
//             // Get the raw pointer from unique_ptr to call its method
//             user = rateLimitUsers[userId].get(); 
//         } // Lock for map access released here

//         // Process the request using the user's properties.
//         // The user's own mutex (rateEntityGuard) handles concurrency for that user.
//         bool isRequestAllowed = user->processUserReqAndAcquire(currentTimestamp);

//         // Protect cout statements to prevent interleaving
//         lock_guard<mutex> cout_lk(g_cout_mutex);
//         if (isRequestAllowed) {
//             cout << "User '" << userId << "': Request Allowed! (" << user->currentReqCnt << " in current window)" << endl;
//         } else {
//             cout << "User '" << userId << "': Sorry! Request not allowed at this moment. Please try later!" << endl;
//         }
//     }
// };

// int main() {
//     // Example: 10 requests allowed within a 60-millisecond window
//     rateLimitManager manager(60, 10); 

//     string s1 = "sonia", s2 = "shivam", s3 = "astha";

//     // Simulate requests from different users or the same user over time
//     // Using lambdas to capture currentTimestamp accurately for each thread call.
//     // This is more robust than capturing it once outside the loop.

//     // Rapid requests for sonia to hit limit
//     for (int i = 0; i < 12; ++i) { // Try to exceed limit of 10
//         thread t([&]() {
//             std::int64_t ts = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
//             manager.processRequest(s1, ts);
//         });
//         t.detach(); // Detach to prevent main from waiting on too many threads
//         std::this_thread::sleep_for(std::chrono::milliseconds(2)); // Small delay
//     }

//     // Requests for other users (should generally be allowed initially)
//     thread t_s2_1([&]() {
//         std::int64_t ts = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
//         manager.processRequest(s2, ts);
//     });

//     thread t_s3_1([&]() {
//         std::int64_t ts = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
//         manager.processRequest(s3, ts);
//     });

//     std::this_thread::sleep_for(std::chrono::milliseconds(30)); // Half window

//     // More requests for sonia after half window
//     for (int i = 0; i < 8; ++i) { // Try to hit limit again, factoring in overlap
//         thread t([&]() {
//             std::int64_t ts = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
//             manager.processRequest(s1, ts);
//         });
//         t.detach();
//         std::this_thread::sleep_for(std::chrono::milliseconds(3)); // Small delay
//     }

//     std::this_thread::sleep_for(std::chrono::milliseconds(65)); // Wait for a full window + some
    
//     // Requests after a full window passed
//     for (int i = 0; i < 5; ++i) {
//         thread t([&]() {
//             std::int64_t ts = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);
//             manager.processRequest(s1, ts);
//         });
//         t.detach();
//         std::this_thread::sleep_for(std::chrono::milliseconds(5));
//     }


//     // Ensure main thread stays alive long enough for detached threads to complete
//     std::this_thread::sleep_for(std::chrono::milliseconds(200)); 

//     t_s2_1.join();
//     t_s3_1.join();
    
//     return 0;
// }