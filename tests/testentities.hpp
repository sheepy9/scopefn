#ifndef _TESTENTITIES_H
#define _TESTENTITIES_H

#include <algorithm>
#include <string>
#include <vector>
#include "scopefn.hpp"

struct Person : scopefn::ScopeFunctions<Person>
{
    std::string name;
    std::string location;
    std::vector<Person> children;
    unsigned age;
    void moveTo(std::string newLocation) { location = newLocation; }

    bool operator==(Person& other)
    {
        if(name != other.name || location != other.location || age != other.age || children.size() != other.children.size())
            return false;
        
        return true;
        // return std::equal(children.begin(), children.end(), other.children.begin());
    }
};

#endif