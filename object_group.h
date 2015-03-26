#ifndef WIDGET_GROUP_H
#define WIDGET_GROUP_H

#include <QObject>

class object_group : public QObject
{
		Q_OBJECT
	public:
		using QObject::QObject;
		void add_to_group(QObject *object);
		void remove_from_group(QObject *object);
		
	public slots:
		void distribute_event(QEvent *e){ event(e); }
		
	protected:
		virtual bool event(QEvent *e);
	private:
		QList<QObject *> group;
};

#endif // WIDGET_GROUP_H
