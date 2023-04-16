#ifndef _SCOPEFN_H
#define _SCOPEFN_H

#include <functional>

namespace scopefn {

namespace scopefn_internal {

template<typename T>
using base_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<typename RT, typename AT>
struct FuncType
{
    FuncType(std::function<RT(AT)>) {};
    using return_type = RT;
    using argument_type = AT;
};

template<typename RT>
struct FuncTypeNoArg
{
    FuncTypeNoArg(std::function<RT(void)>) {};
    using return_type = RT;
    using argument_type = void;
};

/**
 * @brief Type to enable lambda function reflection.
 * An instantiated LambdaReflection<LambdaType> provides aliases for the return 
 * type and argument type of a lambda. This feature is implemented using 
 * std::function and suffers from the same limitations as std::function 
 * reflection. This template struct supports only lambdas with one argument.
 * 
 * @tparam T - type of lambda
 */
template<typename T>
struct LambdaReflection
{
    using return_type = base_type<typename decltype(FuncType(std::function(std::declval<T>())))::return_type>;
    using argument_type = base_type<typename decltype(FuncType(std::function(std::declval<T>())))::argument_type>;
};

template<typename T>
struct LambdaReflectionNoArg
{
    using return_type = base_type<typename decltype(FuncTypeNoArg(std::function(std::declval<T>())))::return_type>;
};

} // namespace scopefn_internal

/**
 * @brief Template struct used for inheriting via the CRTP pattern to add the
 * scope functions as methods to the derived class. This contains no members and
 * it doesn't change the size and memory layout of the derived class objects.
 * Calling the scope methods is done via static polymorphism via the CRTP
 * pattern. There are no virtual (indirect) function calls and no vtables.
 *
 * @tparam Base
 */
template<typename Base>
struct ScopeFunctions
{
    template<typename L>
    auto let(L lambda) -> typename scopefn_internal::LambdaReflection<L>::return_type
    {
        static_assert(
            std::is_same_v<typename scopefn_internal::LambdaReflection<L>::argument_type, Base>,
            "Scope function `let` argument type must match context object type");
        return lambda(*static_cast<Base *>(this));
    }

    template<typename L>
    auto run(L lambda) -> typename scopefn_internal::LambdaReflectionNoArg<L>::return_type
    {
        return lambda();
    }

    template<typename L>
    auto apply(L lambda) -> Base&
    {
        
        lambda();
        return *static_cast<Base *>(this);
    }

    template<typename L>
    auto also(L lambda) -> Base&
    {
        static_assert(
            std::is_same_v<Base, typename scopefn_internal::LambdaReflection<L>::argument_type>,
            "Scope function `also` argument type must match context object "
            "type");

        lambda(*static_cast<Base*>(this));
        return *static_cast<Base *>(this);
    }
};


/**
 * @brief Freestanding let function.
 * 
 * @tparam T 
 */
template <typename L>
struct let
{
    using ARG = typename scopefn_internal::LambdaReflection<L>::argument_type;
    using RT = typename scopefn_internal::LambdaReflection<L>::return_type;
    let(L lambda) : fun(lambda) {}
    RT operator()(ARG& contextObject) { return fun(contextObject); }

    std::function<RT(ARG &)> fun;
};

template<typename T, typename L>
auto operator|(T& contextObject, let<L>&& lambda) -> typename let<L>::RT
{
    static_assert(
        std::is_same_v<T, typename let<L>::ARG>,
        "Scope function `let` argument type must match context object type");
    return lambda(contextObject);
}

template<typename T, typename L>
auto operator|(T&& contextObject, let<L>&& lambda) -> typename let<L>::RT
{
    static_assert(
        std::is_same_v<T, typename let<L>::ARG>,
        "Scope function `let` argument type must match context object type");
    return lambda(contextObject);
}

template <typename L>
struct run
{
    using RT = typename scopefn_internal::LambdaReflectionNoArg<L>::return_type;
    run(L lambda) : fun(lambda) {}
    RT operator()() 
    {
        return fun();
    }

    std::function<RT(void)> fun;
};

template<typename T, typename L>
auto operator|(T& contextObject, run<L>&& lambda) -> typename run<L>::RT
{
    return lambda();
}

template<typename T, typename L>
auto operator|(T&& contextObject, run<L>&& lambda) -> typename run<L>::RT
{
    return lambda();
}

template <typename L>
struct with
{
    using RT = typename scopefn_internal::LambdaReflectionNoArg<L>::return_type;
    with(L lambda) : fun(lambda) {std::invoke(*this);}
    RT operator()() 
    {
        return fun();
    }

    std::function<RT(void)> fun;
};

template <typename L>
struct also
{
    using ARG = typename scopefn_internal::LambdaReflection<L>::argument_type;
    using RT = void;
    also(L lambda) : fun(lambda) {}
    ARG& operator()(ARG& contextObject) 
    {
        fun(contextObject);
        return contextObject;
    }

    std::function<RT(ARG &)> fun;
};

template<typename T, typename L>
auto operator|(T& contextObject, also<L>&& lambda) -> typename also<L>::ARG&
{
    static_assert(std::is_same_v<T, typename also<L>::ARG>,
                  "Scope function `also` argument type must match return type "
                  "which is the context object type");
    return lambda(contextObject);
}

template<typename T, typename L>
auto operator|(T&& contextObject, also<L>&& lambda) -> typename also<L>::ARG&
{
    static_assert(std::is_same_v<T, typename also<L>::ARG>,
                  "Scope function `also` argument type must match return type "
                  "which is the context object type");
    return lambda(contextObject);
}

} // namespace scopefn

#endif