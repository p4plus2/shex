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

template<typename T>
T clamp(T value, T low, T high)
{
	return (value < low) ? low :
	       (value > high) ? high :
	       value;
}

#endif // UTILITY_H
