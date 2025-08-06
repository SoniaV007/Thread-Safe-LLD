#ifndef INMEMORYDBIMPL_HPP
#define INMEMORYDBIMPL_HPP

#include "InMemoryDB.hpp"
#include <string>
#include <optional>
#include <unordered_map>
#include <vector>

using namespace std;

class InMemoryDBImpl : public InMemoryDB
{

    unordered_map<string, unordered_map<string, string>> inMemoryDB;

public:
    void set(string &key, const string &field, const string &value)
    {

        return;
    }

    optional<string> get(const string &key, const string &field)
    {
        return nullopt;
    }

    bool deleteField(const string &key, const string &field)
    {
        return false;
    }

    vector<string> scan(const string &key)
    {
        vector<string> a;
        return a;
    }

    vector<string> scanByPrefix(const string &key, const string &prefix)
    {
        vector<string> a;
        return a;
    }
};

#endif
