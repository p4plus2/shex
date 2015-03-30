#ifndef SETTINGS_EVENT_H
#define SETTINGS_EVENT_H

#include <QEvent>
#include <QVariant>
#include "event_types.h"

class settings_event : public QEvent
{
	public:
		settings_event(QString k, QVariant v, QVariant o) : 
		        QEvent((Type)SETTINGS_EVENT), value(v), old_value(o), key(k) {}
		QPair<QString, QVariant> data(){ return {key, value}; }
		QPair<QString, QVariant> old_data(){ return {key, old_value}; }
		
	private:
		QVariant value;
		QVariant old_value;
		QString key;
};

#endif // SETTINGS_EVENT_H
