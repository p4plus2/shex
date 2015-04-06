#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QSettings>
#include "object_group.h"
#include "events/settings_event.h"

class settings_manager : public QSettings
{
	public:
		using QSettings::QSettings;
		~settings_manager();
		
		void set(const QString &key, const QVariant &value);
		QVariant get(const QString &key);
		
		static void add_listener(QObject *object, const QString &key = "");
		static void add_listener(QObject *object, const QStringList &keys);
		static void remove_listener(QObject *object, const QString &key = "");
		static void remove_listener(QObject *object, const QStringList &keys);
		
		static void add_persistent_listener(QObject *object, const QString &key = "");
		static void add_persistent_listener(QObject *object, const QStringList &keys);
		static void remove_persistent_listener(QObject *object, const QString &key = "");
		static void remove_persistent_listener(QObject *object, const QStringList &keys);
	private:
		using listener_map = QMap<QString, object_group *>;
		static listener_map listeners;
		static listener_map persistent_listeners;
		static object_group global_listeners;
		static int references;
		
		static void add_listener_implementation(QObject *object, const QString &key, listener_map &map);
		static void remove_listener_implementation(QObject *object, const QString &key, listener_map &map);
};

#endif // SETTINGS_MANAGER_H
