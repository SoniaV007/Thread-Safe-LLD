#include <iostream>
#include <unordered_map>
#include<thread>
#include<mutex>
using namespace std;

class physicalBlock{
   string physicalBlockID;
   string logicalBlockID;
   string data;

   public:
   physicalBlock* nextBlock;
   physicalBlock(string physicalBlockID, string logicalBlockID ,string data){
        this->data = data;
        this->physicalBlockID = physicalBlockID;
        this->logicalBlockID = logicalBlockID;
        this->nextBlock = nullptr;
   }

   string getPhysicalBlockId() {return this->physicalBlockID;}
   string getLogicalBlockId() {return this->logicalBlockID;}
   string getData() {return this->data;}
};

class logicalBlocksHistory{
    physicalBlock* latestVersion;
    mutex m;
    public:

    string generateBlockID() {
        static mutex m_counter;
        static int counter = 0;
        unique_lock<mutex> lk(m_counter);
        auto now = std::chrono::system_clock::now().time_since_epoch().count();
        return "block_" + std::to_string(now) + "_" + std::to_string(++counter);
    }

    void addBlock(string logicalBlockId, string data){
        string id = this->generateBlockID();

        physicalBlock* newBlock = new physicalBlock(id,logicalBlockId,data);

        unique_lock<mutex> lk(m);
        if(!latestVersion) {
            latestVersion = newBlock;
        }
        else{
            newBlock->nextBlock = latestVersion;
            latestVersion = newBlock;
        }
    }

    void displayAllBlocks(){
        unique_lock<mutex> lk(m);
        physicalBlock* currentBlock = latestVersion;
        while(currentBlock){
            cout<<"Block ID->"<<currentBlock->getLogicalBlockId()<<" data->"<<currentBlock->getData()<<endl;;
            currentBlock = currentBlock->nextBlock;
        }
    }
    

    void deleteBlock(){
        unique_lock<mutex> lk(m);
        deleteHelper(latestVersion);
        latestVersion = nullptr;
    }

    void deleteHelper(physicalBlock* currentBlock){
        if(!currentBlock) return;
        deleteHelper(currentBlock->nextBlock);
        delete currentBlock;
        return;
    }

    ~logicalBlocksHistory(){
        deleteBlock();

    }
    
};

class blockStorageManager{
    unordered_map<string, logicalBlocksHistory*> storageSystemMap;
    mutex storageGuard;
    
    public:
    blockStorageManager(){
 
    }

    void addStorage(string logicalBlockId,string data){
        logicalBlocksHistory* logicalBlocklist = nullptr;

        unique_lock<mutex> lk(storageGuard);
        if(storageSystemMap.find(logicalBlockId) == storageSystemMap.end()){
            logicalBlocklist = new logicalBlocksHistory();    
            storageSystemMap[logicalBlockId] = logicalBlocklist;
        }
        else{
            logicalBlocklist = storageSystemMap[logicalBlockId];
        }

        lk.unlock();
        if(logicalBlocklist)logicalBlocklist->addBlock(logicalBlockId,data);

    }

    void displayStorage(string logicalBlockId){
        logicalBlocksHistory* logicalBlocklist = nullptr;
        unique_lock<mutex> lk(storageGuard);
        if(storageSystemMap.find(logicalBlockId) != storageSystemMap.end()){
            logicalBlocklist = storageSystemMap[logicalBlockId];
        }
        else cout<<" Not a valid block ID"<<endl;
        lk.unlock();

        if(logicalBlocklist)logicalBlocklist->displayAllBlocks();

    }

    void delStorage(string logicalBlockId){
        logicalBlocksHistory* logicalBlocklist = nullptr;
        unique_lock<mutex> lk(storageGuard);
        if(storageSystemMap.find(logicalBlockId) != storageSystemMap.end()){
            logicalBlocklist = storageSystemMap[logicalBlockId];
            storageSystemMap.erase(logicalBlockId);
        }
        else cout<<" Not a valid block ID"<<endl;
        lk.unlock();

        if(logicalBlocklist) delete logicalBlocklist;
    }




};

int main(){
    blockStorageManager storageSystem1;
    storageSystem1.addStorage("1id","1");
    storageSystem1.addStorage("2id","2");
    storageSystem1.addStorage("2id","2 of 2id");
    storageSystem1.addStorage("2id","3 of 2id");
    storageSystem1.addStorage("3id","3");
    storageSystem1.delStorage("1id");
    storageSystem1.delStorage("4id");
    storageSystem1.displayStorage("1id");
    storageSystem1.displayStorage("2id");
    return 0;
}



// Potential Improvements/Further Considerations:
// Read-Write Locks (std::shared_mutex): For logicalBlocksHistory, addBlock is a write operation, but displayAllBlocks is a read operation. Using std::shared_mutex could allow multiple readers concurrently while writers are exclusive. This would further improve parallelism for read-heavy workloads.
// Error Reporting: Instead of cout for errors, consider throwing exceptions or returning status codes for more robust applications.
// Thread Pools: For a real-world application, operations might be submitted to a thread pool rather than creating new threads for every task.
// Testing: How would you test this for concurrency? (e.g., using multiple threads, stress testing, valgrind for memory errors, ASan for data races).