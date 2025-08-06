#ifndef CLOUD_STORAGE_IMPL_HPP_
#define CLOUD_STORAGE_IMPL_HPP_
#include "cloud_storage.hpp"
#include "user.cpp"

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class CloudStorageIMPL : public CloudStorage
{
private:
    unordered_map<string, int> storage;
    unordered_map<string, User *> users;

public:
    bool AddFile(const std::string &name, int size)
    {
        return false;
    }

    bool CopyFile(const std::string &name_from, const std::string &name_to)
    {
        return false;
    }

    optional<int> GetFileSize(const std::string &name)
    {
        return NULL;
    }

    vector<string> findFile(string &prefix, string &suffix)
    {
        vector<string> ans;
        return ans;
    }

    bool addUser(const std::string &user_id, int capacity)
    {
        return false;
    }

    optional<int> addFileBy(const string &user_id, const std::string &name, int size)
    {
        return nullopt;
    }

    optional<int> updateCapacity(const string &user_id, int capacity)
    {
        return nullopt;
    }

    optional<int> compressFile(const string &user_id, const string &name)
    {
        return nullopt;
    }

    optional<int> decompressFile(const string &user_id, string &name)
    {
        return nullopt;
    }
};

#endif
