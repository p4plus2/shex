#include <QApplication>

#include "object_group.h"
#include "debug.h"

void object_group::add_to_group(QObject *object)
{
	group.append(object);
	connect(object, &QObject::destroyed, this, &object_group::remove_from_group);
	if(object->metaObject()->indexOfSignal(QMetaObject::normalizedSignature("send_event(QEvent *)")) != -1){
		connect(object, SIGNAL(send_event(QEvent *)), this, SLOT(distribute_event(QEvent *)));
	}
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

