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
    unsigned age;
    void moveTo(std::string newLocation) { location = newLocation; }

    bool operator==(Person& other)
    {
        if(name != other.name || location != other.location || age != other.age)
            return false;
        return true;
    }
};

#endif