#include "selection.h"
#include "debug.h"
#include <QtAlgorithms>

void selection::set_start(int s)
{
	start = s;
}

void selection::set_end(int e)
{
	end = e;
	qDebug() << "set" << end;
}

int selection::get_start()
{
	return start > end ? end : start;
}

int selection::get_end()
{
	return start < end ? end : start;
}

void selection::move_end(int amount)
{
	end += amount;
	qDebug() << "move" << end;
}

int selection::drag_direction()
{
	return end - start;
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
