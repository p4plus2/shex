#include "settings_manager.h"
#include "debug.h"

void settings_manager::set(const QString &key, const QVariant &value)
{
	QVariant old_value = get(key);
	setValue(key, value);
	settings_event event(key, value, old_value);
	global_listeners.distribute_event(&event);
	if(listeners.contains(key)){
		listeners[key]->distribute_static_result_event(&event);
	}
	if(persistent_listeners.contains(key)){
		persistent_listeners[key]->distribute_event(&event);
	}
}

QVariant settings_manager::get(const QString &key)
{
	return value(key);
}

void settings_manager::add_listener(QObject *object, const QStringList &keys)
{
	for(const auto &key : keys){
		add_listener_implementation(object, key, listeners);
	}
}

void settings_manager::add_listener(QObject *object, const QString &key)
{
	add_listener_implementation(object, key, listeners);
}

void settings_manager::remove_listener(QObject *object, const QStringList &keys)
{
	for(const auto &key : keys){
		remove_listener_implementation(object, key, listeners);
	}
}

void settings_manager::remove_listener(QObject *object, const QString &key)
{
	remove_listener_implementation(object, key, listeners);
}

void settings_manager::add_persistent_listener(QObject *object, const QStringList &keys)
{
	for(const auto &key : keys){
		add_listener_implementation(object, key, persistent_listeners);
	}
}

void settings_manager::add_persistent_listener(QObject *object, const QString &key)
{
	add_listener_implementation(object, key, persistent_listeners);
}

void settings_manager::remove_persistent_listener(QObject *object, const QStringList &keys)
{
	for(const auto &key : keys){
		remove_listener_implementation(object, key, persistent_listeners);
	}
}

void settings_manager::remove_persistent_listener(QObject *object, const QString &key)
{
	remove_listener_implementation(object, key, persistent_listeners);
}

void settings_manager::add_listener_implementation(QObject *object, const QString &key, listener_map &map)
{
	if(key.isEmpty()){
		global_listeners.add_to_group(object);
	}else{
		if(!map.contains(key)){
			map.insert(key, new object_group(&parent));
		}
		object_group *group = map[key];
		group->add_to_group(object);
		if(group->size() == 1){
			settings_manager settings;
			QVariant value = settings.get(key);
			settings_event event(key, value, value);
			group->distribute_static_result_event(&event);
		}
	}
}

void settings_manager::remove_listener_implementation(QObject *object, const QString &key, listener_map &map)
{
	if(key.isEmpty()){
		global_listeners.remove_from_group(object);
	}else{
		if(map.contains(key)){
			map[key]->remove_from_group(object);
		}
	}
}


settings_manager::listener_map settings_manager::listeners;
settings_manager::listener_map settings_manager::persistent_listeners;
object_group settings_manager::global_listeners;
QObject settings_manager::parent;
