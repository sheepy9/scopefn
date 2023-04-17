#ifndef _SCOPEFN_H
#define _SCOPEFN_H

#include <functional>

namespace scopefn {

namespace scopefn_internal {

/**
 * @brief Helper alias to unwrap CV qualifiers from a type T.
 * For example if T = const& Animal then base_type = Animal
 * 
 * @tparam T 
 */
template<typename T>
using base_type = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

/**
 * @brief Helper struct used for template argument deduction on
 * std::function<R(A)>.
 *
 * @tparam RT - return type
 * @tparam AT - argument type
 */
template<typename RT, typename AT>
struct FuncType
{
    FuncType(std::function<RT(AT)>) {};
    using return_type = RT;
    using argument_type = AT;
};

/**
 * @brief Helper struct used for template argument deduction on
 * std::function<R(void)>. This struct exists because void cannot be deduced
 * from the regular FuncType struct
 *
 * @tparam RT - return type
 */
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

/**
 * @brief Same as LambdaReflection, but for lambdas that accept no args, since
 * void arg type cannot be deduced by std::function.
 *
 * @tparam T
 */
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
    /**
     * @brief The `let` scope function accepts the context object via argument
     * and returns the lambda result.
     *
     * @example animal.let([](Animal& it) { it.doSomething(); });
     *
     * @tparam L
     * @param lambda
     * @return scopefn_internal::LambdaReflection<L>::return_type
     */
    template<typename L>
    auto let(L lambda) -> typename scopefn_internal::LambdaReflection<L>::return_type
    {
        static_assert(
            std::is_same_v<typename scopefn_internal::LambdaReflection<L>::argument_type, Base>,
            "Scope function `let` argument type must match context object type");
        return lambda(*static_cast<Base *>(this));
    }

    /**
     * @brief The `run` scope function accepts the context object through lambda
     * capture and returns the lambda result.
     *
     * @example animal.run([self = &animal] { self->doSomething(); });
     *
     * @tparam L
     * @param lambda
     * @return scopefn_internal::LambdaReflectionNoArg<L>::return_type
     */
    template<typename L>
    auto run(L lambda) -> typename scopefn_internal::LambdaReflectionNoArg<L>::return_type
    {
        return lambda();
    }

    /**
     * @brief The `apply` scope function accepts the context object through
     * lambda capture and returns a reference to the same context object.
     *
     * @example animal.apply([self = &animal] { self->doSomething(); });
     *
     * @tparam L
     * @param lambda
     * @return Base&
     */
    template<typename L>
    auto apply(L lambda) -> Base&
    {
        
        lambda();
        return *static_cast<Base *>(this);
    }

    /**
     * @brief The `also` scope function accepts the context object as an
     * argument and returns a reference to the same context object.
     *
     * @example animal.apply([](Animal& it) { it.doSomething(); });
     *
     * @tparam L
     * @param lambda
     * @return Base&
     */
    template<typename L>
    auto also(L lambda) -> Base&
    {
        static_assert(
            std::is_same_v<Base, typename scopefn_internal::LambdaReflection<L>::argument_type>,
            "Scope function `also` argument type must match context object "
            "type");

        lambda(*static_cast<Base*>(this));
        return *static_cast<Base*>(this);
    }
};

/**
 * @brief Freestanding let scope function. The `let` scope function accepts the
 * context object via argument and returns the lambda result.
 *
 * @example animal | let([](Animal& it){ it.doSomething(); });
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

/**
 * @brief Chaining operator for the frestanding let function
 * 
 * @tparam T 
 * @tparam L 
 * @param contextObject 
 * @param lambda 
 * @return let<L>::RT 
 */
template<typename T, typename L>
auto operator|(T& contextObject, let<L>&& lambda) -> typename let<L>::RT
{
    static_assert(
        std::is_same_v<T, typename let<L>::ARG>,
        "Scope function `let` argument type must match context object type");
    return lambda(contextObject);
}

/**
 * @brief Chaining operator for the freestanding let function
 * 
 * @tparam T 
 * @tparam L 
 * @param contextObject 
 * @param lambda 
 * @return let<L>::RT 
 */
template<typename T, typename L>
auto operator|(T&& contextObject, let<L>&& lambda) -> typename let<L>::RT
{
    static_assert(
        std::is_same_v<T, typename let<L>::ARG>,
        "Scope function `let` argument type must match context object type");
    return lambda(contextObject);
}

/**
 * @brief Freestanding run scope function. The `run` scope function accepts the
 * context object through lambda capture and returns the lambda result.
 *
 * @example animal | run([self = &animal]{ self->doSomething(); });
 *
 * @tparam L
 */
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

/**
 * @brief Chaining operator for the run scope function
 * 
 * @tparam T 
 * @tparam L 
 * @param contextObject 
 * @param lambda 
 * @return run<L>::RT 
 */
template<typename T, typename L>
auto operator|(T& contextObject, run<L>&& lambda) -> typename run<L>::RT
{
    return lambda();
}

/**
 * @brief Chaining operator for the run scope function
 * 
 * @tparam T 
 * @tparam L 
 * @param contextObject 
 * @param lambda 
 * @return run<L>::RT 
 */
template<typename T, typename L>
auto operator|(T&& contextObject, run<L>&& lambda) -> typename run<L>::RT
{
    return lambda();
}

/**
 * @brief Freestanding with scope function. The with scope function accepts the
 * context object via lambda capture and returns the lambda result. The with
 * scope function only exists as freestanging and cannot be chained.
 *
 * @example animal | with([self = &animal]{ self->doSomething(); });
 *
 * @tparam L
 */
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

/**
 * @brief Freestanding also function. The `also` scope function accepts the
 * context object as an argument and returns a reference to the same context
 * object.
 *
 * @example animal | also([](Animal& it){ it.doSomething(); });
 *
 * @tparam L
 */
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

/**
 * @brief Chaining operator for the freestanding also function.
 * 
 * @tparam T 
 * @tparam L 
 * @param contextObject 
 * @param lambda 
 * @return also<L>::ARG& 
 */
template<typename T, typename L>
auto operator|(T& contextObject, also<L>&& lambda) -> typename also<L>::ARG&
{
    static_assert(std::is_same_v<T, typename also<L>::ARG>,
                  "Scope function `also` argument type must match return type "
                  "which is the context object type");
    return lambda(contextObject);
}

/**
 * @brief Chaining operator for the freestanding also function.
 * 
 * @tparam T 
 * @tparam L 
 * @param contextObject 
 * @param lambda 
 * @return also<L>::ARG& 
 */
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