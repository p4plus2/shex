#include "selection.h"
#include <QtAlgorithms>

selection &selection::operator= (const selection &s)
{
	_start = s._start;
	_end = s._end;
	return *this;
}

void selection::set(int s, int e)
{
	if(s > e){
		qSwap(s, e);
	}
	_start = s;
	_end = e;
}

int selection::range() const
{
	return end - start + 1; //selections are inclusive of the last byte
}

bool selection::is_active()
{
	return active;
}

void selection::set_active(bool a)
{
	active = a;
}

bool selection::is_dragging()
{
	return dragging;
}

void selection::set_dragging(bool d)
{
	dragging = d;
}
