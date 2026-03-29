#ifndef _CACHE_H
#define _CACHE_H

#include <cstring>
#include <cstdarg>
#include <vector>
#include <string>
#include <unordered_map>

#include "Globals.h"

template<class T>
struct CacheEntry
{
    T data;
    u32 requestIdx;
    string str;
};

template<class T>
class Cache
{
public:
    Cache(u32 capacity) : capacity(capacity), currRequest(0)
    {
        memset(&nullEntry.data, 0, sizeof(T)); 
    }
    virtual ~Cache() {} 

    T& Get(string keyStr, u64 keyVar, ...)
    {
        va_list argsGen, argsLoad;
        va_start(argsGen, keyVar);
        va_copy(argsLoad, argsGen);

        string key = GenKey(keyStr, keyVar, argsGen);
        va_end(argsGen);

        typename unordered_map<string, CacheEntry<T>>::iterator itr = cache.find(key);
        if (itr == cache.end())
        {
            if (cache.size() >= capacity)
                PopOldest();
            CacheEntry<T> newEntry;

            if (!LoadEntry(newEntry.data, newEntry.str, keyStr, keyVar, argsLoad))
                return nullEntry.data;

            log.push_back("Load: " + key);
            cache[key] = newEntry;
            itr = cache.find(key);
        }
        va_end(argsLoad);

        CacheEntry<T>& entry = (*itr).second;
        entry.requestIdx = currRequest++;
        return entry.data;
    }

    T& Update(string keyStr, u64 keyVar, ...)
    {
        va_list argsGen, argsLoad;
        va_start(argsGen, keyVar);
        va_copy(argsLoad, argsGen);

        string key = GenKey(keyStr, keyVar, argsGen);
        va_end(argsGen);

        typename unordered_map<string, CacheEntry<T>>::iterator itr = cache.find(key);
        if (itr != cache.end())
        {
            UnloadEntry((*itr).second.data, (*itr).second.str);
            log.push_back("Unload (Update): " + key);
        }

        if (cache.size() >= capacity)
            PopOldest();
        CacheEntry<T> newEntry;

        if (!LoadEntryUpdate(newEntry.data, newEntry.str, keyStr, keyVar, argsLoad))
            return nullEntry.data;

        log.push_back("Load (Update): " + key);
        cache[key] = newEntry;
        itr = cache.find(key);
        va_end(argsLoad);

        CacheEntry<T>& entry = (*itr).second;
        entry.requestIdx = currRequest++;
        return entry.data;
    }

    void Release()
    {
        for (typename unordered_map<string, CacheEntry<T>>::iterator itr = cache.begin(); itr != cache.end(); ++itr)
        {
            UnloadEntry((*itr).second.data, (*itr).second.str);
            log.push_back("Unload: " + (*itr).first);
        }

    }

    static string GenerateDataStr(string keyStr, u32 keyVar)
    {
        for (u32 i = 0; i < keyStr.size(); ++i)
        {
            if (keyStr.at(i) == PATH_SEPARATOR)
                keyStr.at(i) = '-';
        }
        return keyStr + "-" + to_string(keyVar);
    }

protected:

    virtual string GenKey(string keyStr, u64 keyVar, va_list args)
    {
        UNUSED(args);
        return keyStr + "-" + to_string(keyVar);
    }
    virtual bool LoadEntry(T& data, string& dataStr, string keyStr, u64 keyVar, va_list args) = 0;
    virtual void UnloadEntry(T& data, const string& dataStr) = 0;

    virtual bool LoadEntryUpdate(T& data, string& dataStr, string keyStr, u64 keyVar, va_list args) { return LoadEntry(data, dataStr, keyStr, keyVar, args); }

private:
    u32 capacity;
    u32 currRequest;

    unordered_map<string, CacheEntry<T>> cache;
    CacheEntry<T> nullEntry;

    vector<string> log;

    void PopOldest()
    {
        typename unordered_map<string, CacheEntry<T>>::iterator oldest = cache.end();
        for (typename unordered_map<string, CacheEntry<T>>::iterator itr = cache.begin(); itr != cache.end(); ++itr)
        {
            if (oldest == cache.end() || (*itr).second.requestIdx < (*oldest).second.requestIdx)
                oldest = itr;
        }
        UnloadEntry((*oldest).second.data, (*oldest).second.str);
        log.push_back("Unload: " + (*oldest).first);
        cache.erase(oldest);
    }

    // friend class Engine;
};

#endif // _CACHE_H
