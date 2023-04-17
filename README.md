# scopefn - Kotlin style scope functions for C++

`scopefn` is a small header only library that implements kotlin style scope functions in C++. Kotlin defines 5 scope functions: `let`, `run`, `with`, `apply`, `also`. Scope function's sole purpose is to execute a block of code within the context of an object. Scope functions in kotlin are implicit extension functions for all objects. These functions can be used to conveniently manipulate objects in a concise and expressive way, reducing boilerplate code and improving readability.

This is what a kotlin `let` scope function looks like applied on a temporary Person object (the `it` argument of the lambda is implicit):
``` kt
Person("Alice", 20, "Amsterdam").let {
    println(it)
    it.moveTo("London")
    it.incrementAge()
    println(it)
}
```

This is what the equivalent `scopefn` implementation in C++ enables:
``` cpp
Person("Alice", 20, "Amsterdam").let([](Person& it){
    std::cout << it << std::endl;
    it.moveTo("London");
    it.incrementAge();
    std::cout << it << std::endl;
})
```

The scope functions differ depending on how they accept the context object (object on which the scope function is being applied) and what their return value is.

|Function   | Accepts                            | Return value   |
|-----------|------------------------------------|----------------|
| `let`     | contextObject& via lambda argument | lambda result  |
| `run`     | contextObject* via lambda capture  | lambda result  |
| `with`    | contextObject* via lambda capture  | lambda result  |
| `apply`   | contextObject* via lambda capture  | contextObject& |
| `also`    | contextObject& via lambda argument | contextObject& |

## Features
- Provides `let`, `run`, `apply`, and `also` scope functions via CRTP and `let`, `run`, `also` and `with` via regular freestanding functions
- Supports both member functions (via CRTP pattern) and freestanding functions
- Enables chaining of scope functions using the | operator. Chaining using operator | was added to compensate for the lack of extension functions. 
- Uses static polymorphism and does not introduce runtime overhead
- Header-only library with no external dependencies

## Usage
Include the header file in your project:

``` cpp
#include "scopefn.hpp"
```
Use the scope functions as member functions with the CRTP pattern:

``` cpp
class Animal : public scopefn::ScopeFunctions<Animal> {
    // Your class implementation
};
```

``` cpp
using namespace scopefn;

Animal animal;
std::string name = animal.apply([self = &animal]{ self->doSomething(); })
                         .also([](Animal& it){ it.doSomethingElse();})
                         .let([](Animal& it){ return it.getName();});
```
Or use them as freestanding functions with the | operator if you cannot use inheritance for the given type:

``` cpp
using namespace scopefn;
std::vector<int> vec;
int max = vec | also([](std::vector<int>& it){ it.push_back(2);}) 
              | also([](std::vector<int>& it){ it.push_back(3);})
              | let([](std::vector<int>& it) { return *std::max_element(it.begin(),it.end()).base();});
```

## Scope Functions
### let
Accepts the context object as an argument and returns the lambda result.

``` cpp
animal.let([](Animal& it) { it.doSomething(); });
animal | scopefn::let([](Animal& it){ it.doSomething(); });
```

### run
Accepts the context object through lambda capture and returns the lambda result.

``` cpp
animal.run([self = &animal] { self->doSomething(); });
animal | scopefn::run([self = &animal]{ self->doSomething(); });
```

### apply
Accepts the context object through lambda capture and returns a reference to the same context object.

``` cpp
animal.apply([self = &animal] { self->doSomething(); });
```

### also
Accepts the context object as an argument and returns a reference to the same context object.

``` cpp
animal.also([](Animal& it) { it.doSomething(); });
animal | scopefn::also([](Animal& it){ it.doSomething(); });
```

## Special Thanks
- ChatGPT for generating this readme from the code documentation
