#include "object_group.h"
#include "debug.h"
#include <QApplication>

void object_group::add_to_group(QObject *object)
{
	group.append(object);
	connect(object, &QObject::destroyed, this, &object_group::remove_from_group);
} 

void object_group::remove_from_group(QObject *object)
{
	group.removeOne(object);
}

bool object_group::event(QEvent *e)
{
	foreach(QObject *object, group){
		QApplication::sendEvent(object, e);
	}
	return true;
}

