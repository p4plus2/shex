#include "settings_manager.h"
#include "debug.h"

void settings_manager::set(const QString &key, const QVariant &value)
{
	QVariant old_value = get(key);
	setValue(key, value);
	settings_event event(key, value, old_value);
	listeners.distribute_event(&event);
	qDebug() << key << value;
}

QVariant settings_manager::get(const QString &key)
{
	return value(key);
}

void settings_manager::add_listener(QObject *object)
{
	listeners.add_to_group(object);
}

void settings_manager::remove_listener(QObject *object)
{
	listeners.remove_from_group(object);
}

object_group settings_manager::listeners;
