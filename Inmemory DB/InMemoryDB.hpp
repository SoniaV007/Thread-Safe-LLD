#ifndef INMEMORYDB_HPP
#define INMEMORYDB_HPP

#include <string>
#include <optional>

using namespace std;

class InMemoryDB
{
public:
    virtual void set(string &key, const string &field, const string &value) = 0;

    virtual optional<string> get(const string &key, const string &field) = 0;

    virtual bool deleteField(const string &key, const string &field) = 0;

    virtual vector<string> scan(const std::string &key) = 0;

    virtual vector<string> scanByPrefix(const string &key, const string &prefix) = 0;

    virtual ~InMemoryDB() = default;
};

#endif
