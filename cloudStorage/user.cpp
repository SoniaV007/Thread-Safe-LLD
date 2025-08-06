#include <iostream>
#include <vector>
#include <unordered_map>

using namespace std;

class User
{
    int capacity, currentCapacity;
    vector<string> ownedFiles;

public:
    User(int capacity)
    {
        this->capacity = capacity;
    }

    int getCapacity()
    {
        return this->capacity;
    }

    int getCurrentCapacity()
    {
        return this->currentCapacity;
    }

    void setCapacity(int capacity)
    {
        this->capacity = capacity;
    }

    int updateCurrentCapacity(int capacity)
    {
        this->currentCapacity = capacity;
    }

    void addFileForUser(string fileName)
    {
        this->ownedFiles.push_back(fileName);
    }

    int updateFilesBasedOnSize(int newCapacity, unordered_map<string, int> &storage)
    {
        sort(this->ownedFiles.begin(), this->ownedFiles.end(), [&](const string &a, const string &b)
             {
        int sizeA = storage[a];
        int sizeB = storage[b];

        if (sizeA != sizeB) {
            return sizeA > sizeB;
        } else {
            return a < b;
        } });

        int currentCapacity = this->getCurrentCapacity();
        int removedFiles = 0;

        while (currentCapacity > newCapacity)
        {
            removedFiles++;
            int n = this->ownedFiles.size();
            currentCapacity = currentCapacity - storage[this->ownedFiles[n - 1]];
            this->ownedFiles.pop_back();
        }

        this->updateCurrentCapacity(currentCapacity);
        return removedFiles;
    }

    bool userOwnsFile(string fileName)
    {
        for (auto &file : this->ownedFiles)
        {
            if (file == fileName)
                return true;
        }

        return false;
    }
};