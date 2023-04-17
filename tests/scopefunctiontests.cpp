#include "testentities.hpp"
#include <algorithm>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <gtest/internal/gtest-port.h>
#include <string>
#include <scopefnmacros.hpp>


using namespace scopefn;

TEST(ScopeFunctionTests, LetFunctionTest)
{
    Person alice{ .name="Alice", .location="London", .age=20 };
    Person().let([&alice](Person& it)
    {
        it.name = "Alice";
        it.age = 20;
        it.location = "Amsterdam";
        it.moveTo("London");
        ASSERT_EQ(it == alice, true);
    });

    std::string maxValue = std::vector<int>{0, 1, 2} 
    | let([](std::vector<int>& it)
        {
            it.push_back(3);
            return *std::max_element(it.begin(), it.end()).base();
        }) 
    | let([](int& it) { return std::to_string(it); });

    ASSERT_EQ(maxValue, "3");
}

TEST(ScopeFunctionTests, RunFunctionTest)
{
    Person person{.name = "Alice", .location = "London", .age = 20};
    bool success = person.run([self = &person]
    {
        self->name = "Peter";
        self->age = 55;
        self->location = "Madrid";
        return true;
    });
    ASSERT_EQ(person.name, "Peter");
    ASSERT_EQ(person.age, 55);
    ASSERT_EQ(person.location, "Madrid");
    ASSERT_EQ(success, true);

    std::string hello("hello world");
    hello | run([self = &hello]
        {
            self->replace(0,1,"y");
        });
    ASSERT_EQ(hello, "yello world");
}

TEST(ScopeFunctionTests, WithFunctionTest)
{
    Person person{.name = "Alice", .location = "London", .age = 20};
    with([self = &person]
    {
        self->name.clear();
    });

    ASSERT_EQ(person.name, "");
    ASSERT_EQ(person.age, 20);
    ASSERT_EQ(person.location, "London");
}

TEST(ScopeFunctionTests, AlsoFunctionTest)
{
    Person person{.name = "Alice", .location = "London", .age = 20};
    person.also([](Person& it) { it.name.clear(); })
        .also([](Person& it) { it.location.clear(); })
        .also([](Person& it) { it.age = 0; });
    ASSERT_EQ(person.name, "");
    ASSERT_EQ(person.age, 0);
    ASSERT_EQ(person.location, "");
}

TEST(ScopeFunctionTests, ApplyFunctionTest)
{
    Person person{.name = "Alice", .location = "London", .age = 20};
    person.apply([self = &person] { self->name.clear(); })
        .apply([self = &person]{ self->location.clear(); })
        .apply([self = &person] { self->age = 0; });
    ASSERT_EQ(person.name, "");
    ASSERT_EQ(person.age, 0);
    ASSERT_EQ(person.location, "");
}

TEST(ScopeFunctionTests, MixingFunctionsTest)
{
    unsigned num =
        std::vector<int>{1, 2, 3} |
        let(
            [](std::vector<int>& it)
            {
                it.push_back(4);
                return it;
            }) |
        let([](std::vector<int>& it) -> unsigned
            { return *std::max_element(it.begin(), it.end()).base(); }) |
        also([](unsigned& it) { it = it * 2; });

    Person person{.name = "Alice", .location = "London", .age = 20};
    person.apply([self = &person, num] { self->age = num; })
        .apply([self = &person] { self->name.clear(); })
        .also([](Person& it) { it.location.clear(); });

    ASSERT_EQ(person.name, "");
    ASSERT_EQ(person.age, 8);
    ASSERT_EQ(person.location, "");
}

TEST(ScopeFunctionTests, CrazyMacrosTest)
{
    unsigned num = 0;
    auto vec = std::vector<int>{1, 2, 3};
    num = 
    vec | LET(vec,{ it.push_back(4); return it; })
        | LET(vec,{ return *std::max_element(it.begin(), it.end()).base(); }) 
        | ALSO(int(), { it = it * 2; });

    Person person{.name = "Alice", .location = "London", .age = 20};
    person.APPLY(person, { self->age = 8; })
        .APPLY(person, { self->name.clear(); })
        .ALSO(person, { it.location.clear(); });

    ASSERT_EQ(num, 8);
    ASSERT_EQ(person.name, "");
    ASSERT_EQ(person.age, 8);
    ASSERT_EQ(person.location, "");
}