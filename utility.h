#ifndef UTILITY_H
#define UTILITY_H

template<typename... A>
struct resolve{ 
    template<typename C, typename F> 
    static constexpr auto from(F (C::*function)(A...)) -> decltype(function)
    { 
        return function;
    } 
};

#endif // UTILITY_H
