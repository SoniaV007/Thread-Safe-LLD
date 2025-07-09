#include<iostream>
#include<mutex>
#include<thread>
using namespace std;

mutex g_cout_mutex;
class CacheNode{
    public:
    int val;
    int key;
    CacheNode* next;
    CacheNode* prev;
    CacheNode(int key, int val){
         this->key = key;
         this->val = val;
         next = nullptr;
         prev = nullptr;
    }

    void changeValue(int value){
        this->val = value;
    }
};

class LRUCache {
    unordered_map<int, CacheNode*> nodeAddress;
    CacheNode* leastRecentNode;
    CacheNode* mostRecentNode;
    int capacity;

    void makeNodeLatest(CacheNode* node){
        if(leastRecentNode==mostRecentNode) return;

        if(node == leastRecentNode) leastRecentNode = leastRecentNode->next;

        if(node == mostRecentNode) return;

        CacheNode* nextNode = node->next;
        CacheNode* prevNode = node->prev;
        if(prevNode)prevNode->next = nextNode;
        nextNode->prev = prevNode;

        mostRecentNode->next = node;
        node->prev = mostRecentNode;
        node->next = nullptr;
        mostRecentNode=mostRecentNode->next;

        return;

    }

    void deleteHeadAsCapacityExceeded(){
        CacheNode* currNode = leastRecentNode;
        leastRecentNode = leastRecentNode->next;
        leastRecentNode->prev = nullptr;
        nodeAddress.erase(currNode->key);
        delete currNode;
    }

public:
    LRUCache(int capacity) {
        this->capacity = capacity;
        leastRecentNode = nullptr;
        mostRecentNode = nullptr;
    }
    
    int get(int key) {
        if(nodeAddress.size()==0) return -1;
        if(nodeAddress.find(key)!=nodeAddress.end()){
            makeNodeLatest(nodeAddress[key]);
            return nodeAddress[key]->val;
        }
        
        return -1;
    }
    
    void put(int key, int value) {

         if(nodeAddress.find(key)!=nodeAddress.end()){
            nodeAddress[key]->changeValue(value);
            makeNodeLatest(nodeAddress[key]);
            return;
        }                                       //update

        CacheNode* newNode = new CacheNode(key,value);    //add
        nodeAddress[key] = newNode;       
        
        if(!leastRecentNode) {       //no node till now
            leastRecentNode = newNode;
            mostRecentNode = newNode;
            return;
        }

        mostRecentNode->next = newNode;
        newNode->prev = mostRecentNode;
        mostRecentNode = mostRecentNode->next;

        if(nodeAddress.size()>capacity) deleteHeadAsCapacityExceeded();
        
        return;
    }
};

int main(){
    
}