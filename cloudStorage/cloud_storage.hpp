#ifndef CLOUD_STORAGE_HPP_
#define CLOUD_STORAGE_HPP_

#include <optional>
#include <string>

struct CloudStorage
{
    virtual bool AddFile(const std::string &name, int size)
    {
        return false;
    }

    virtual bool CopyFile(const std::string &name_from, const std::string &name_to)
    {
        return false;
    }

    virtual optional<int> GetFileSize(const std::string &name)
    {
        return std::nullopt;
    }

    virtual vector<string> findFile(string &prefix, string &suffix)
    {
        vector<string> ans;
        return ans;
    }

    virtual bool addUser(const std::string &user_id, int capacity)
    {
        return false;
    }

    virtual optional<int> addFileBy(const std::string &user_id, const std::string &name, int size)
    {
        return std::nullopt;
    }

    virtual optional<int> updateCapacity(const std::string &user_id, int capacity)
    {
        return std::nullopt;
    }

    virtual optional<int> compressFile(const string &user_id, const string &name)
    {
        return std::nullopt;
    }

    virtual optional<int> decompressFile(const string &user_id, string &name)
    {
        return std::nullopt;
    }

    virtual inline ~CloudStorage() = 0;
};

inline CloudStorage::~CloudStorage() = default;

#endif
