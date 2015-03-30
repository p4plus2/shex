#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#include <QSettings>
#include "object_group.h"
#include "events/settings_event.h"

class settings_manager : public QSettings
{
	public:
		using QSettings::QSettings;
		
		void set(const QString &key, const QVariant &value);
		QVariant get(const QString &key);
		
		static void add_listener(QObject *object);
		static void remove_listener(QObject *object);
		
	private:
		static object_group listeners;
};

#endif // SETTINGS_MANAGER_H
