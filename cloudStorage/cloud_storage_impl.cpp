#include "cloud_storage_impl.hpp"
using namespace std;

bool CloudStorageIMPL::AddFile(const string &name, int size)
{
    if (this->storage.find(name) == this->storage.end())
    {
        this->storage[name] = size;
        return true;
    }
    return false;
}

bool CloudStorageIMPL::CopyFile(const string &name_from, const string &name_to)
{
    if (this->storage.find(name_from) != this->storage.end() || this->storage.find(name_to) == this->storage.end())
    {
        this->storage[name_to] = this->storage[name_from];
        return true;
    }
    return false;
}

optional<int> CloudStorageIMPL::GetFileSize(const string &name)
{
    if (this->storage.find(name) != this->storage.end())
        return this->storage[name];
    return nullopt;
}

vector<string> CloudStorageIMPL::findFile(string &prefix, string &suffix)
{
    vector<pair<int, string>> ans;
    vector<string> ansFiles;

    for (auto &it : this->storage)
    {
        string fileName = it.first;
        int sizeOfFileName = fileName.size();
        int sizeOfFile = it.second;
        int prefixSize = prefix.size();
        int suffixSize = suffix.size();

        bool isPrefix = (sizeOfFileName >= prefixSize && fileName.compare(0, prefixSize, prefix) == 0);
        bool isSuffix = (sizeOfFileName >= suffixSize && fileName.compare(sizeOfFileName - suffixSize, suffixSize, suffix) == 0);

        if (isPrefix && isSuffix)
        {
            ans.push_back(make_pair(sizeOfFile, fileName));
        }
    }

    sort(ans.begin(), ans.end(), [](const pair<int, string> &a, const pair<int, string> &b)
         {
    if (a.first != b.first)
        return a.first > b.first;
    return a.second < b.second; });

    for (auto &file : ans)
    {
        ansFiles.push_back(file.second + to_string(file.first));
    }
    return ansFiles;
}

bool CloudStorageIMPL::addUser(const string &user_id, int capacity)
{
    if (this->users.find(user_id) == this->users.end())
    {
        this->users[user_id] = new User(capacity);
        return true;
    }
    return false;
}

optional<int> CloudStorageIMPL::addFileBy(const string &user_id, const string &name, int size)
{
    User *user = this->users[user_id];
    if (user->getCurrentCapacity() + size <= user->getCapacity())
    {
        this->AddFile(name, size);
        user->addFileForUser(name);
        int currCapacity = user->getCurrentCapacity();
        user->updateCurrentCapacity(currCapacity + size);

        return user->getCapacity() -
               user->getCurrentCapacity();
    }
    return nullopt;
}

optional<int> CloudStorageIMPL::updateCapacity(const string &user_id, int capacity)
{
    User *user = this->users[user_id];
    user->setCapacity(capacity);
    if (user->getCurrentCapacity() > capacity)
    {
        int removedfiles = user->updateFilesBasedOnSize(capacity, this->storage);
        return removedfiles;
    }
    return nullopt;
}

optional<int> CloudStorageIMPL::compressFile(const string &user_id, const string &fileName)
{
    User *user = this->users[user_id];

    if (user->userOwnsFile(fileName))
    {
        int sizeOfFile = this->storage[fileName];
        sizeOfFile = sizeOfFile / 2;

        this->storage.erase(fileName);
        this->storage[fileName + ".compressed"] = sizeOfFile;

        int currentCapacity = user->getCurrentCapacity();
        user->updateCurrentCapacity(currentCapacity - sizeOfFile);

        return user->getCurrentCapacity();
    }

    return nullopt;
}

optional<int> CloudStorageIMPL::decompressFile(const string &user_id, string &fileName)
{
    User *user = this->users[user_id];

    if (this->storage.find(fileName) == this->storage.end() && user->userOwnsFile(fileName))
    {
        int sizeOfFile = this->storage[fileName];

        int currentCapacity = user->getCurrentCapacity();
        if (currentCapacity + sizeOfFile > user->getCapacity())
            return false;

        user->updateCurrentCapacity(currentCapacity + sizeOfFile);

        this->storage.erase(fileName);

        fileName.erase(fileName.size() - string(".compressed").size());

        this->storage[fileName] = sizeOfFile * 2;

        return user->getCurrentCapacity();
    }

    return nullopt;
}