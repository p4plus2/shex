#ifndef UTILITY_H
#define UTILITY_H

#include <QWidget>

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

template <typename T1, typename T2> void conditional_variant_copy(T1 &a, T2 &b){ Q_UNUSED(a); Q_UNUSED(b); }
template <typename T = QVariant> void conditional_variant_copy(T &a, T &b)
{
	int type = a.type();
	a = b;
	a.convert(type);
}

inline void propagate_resize(QWidget *child)
{
	QWidget *parent = child->parentWidget();
	if(parent->layout()){
		parent->layout()->invalidate();
	}
	do{
		parent->adjustSize();
		parent = parent->parentWidget();
	}while(parent);
}

#endif // UTILITY_H
