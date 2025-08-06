#include <iostream>
#include "InMemoryDBImpl.hpp"

using namespace std;

void InMemoryDBImpl::set(string &key, const string &field, const string &value)
{
    this->inMemoryDB[key][field] = value;
    return;
}

optional<string> InMemoryDBImpl::get(const string &key, const string &field)
{
    if (this->inMemoryDB.find(key) != this->inMemoryDB.end())
    {
        if (this->inMemoryDB[key].find(field) != this->inMemoryDB[key].end())
        {
            return this->inMemoryDB[key][field];
        }
    }

    return nullopt;
}

bool InMemoryDBImpl::deleteField(const string &key, const string &field)
{
    if (this->inMemoryDB.find(key) != this->inMemoryDB.end())
    {
        if (this->inMemoryDB[key].find(field) != this->inMemoryDB[key].end())
        {
            this->inMemoryDB[key].erase(field);
        }
    }

    return false;
}

vector<string> InMemoryDBImpl::scan(const string &key)
{
    vector<string> a;
    return a;
}

vector<string> InMemoryDBImpl::scanByPrefix(const string &key, const string &prefix)
{
    vector<string> a;
    return a;
}