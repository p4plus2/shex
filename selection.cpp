#include "selection.h"
#include "debug.h"
#include <QtAlgorithms>

void selection::set_start(int s)
{
	start = s;
}

void selection::set_end(int e)
{
	end = e + 1;
	active = true;
}

int selection::get_start()
{
	return start > end ? end : start;
}

int selection::get_end()
{
	return start < end ? end : start;
}

int selection::get_start_aligned()
{
	return (start > end ? end : start) & ~1;
}


int selection::get_end_aligned()
{
	return (start < end ? end : start) & ~1;
}

void selection::move_end(int amount)
{
	end += amount;
}

int selection::range()
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
	if(!active){
		dragging = false;
	}
}

bool selection::is_dragging()
{
	return dragging;
}

void selection::set_dragging(bool d)
{
	dragging = d;
}
