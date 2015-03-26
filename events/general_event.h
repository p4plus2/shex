#ifndef GENERAL_EVENTS_H
#define GENERAL_EVENTS_H

#include <QEvent>

enum event_types : int;

template <typename S, event_types B>
class general_event : public QEvent
{
	public:
		template <typename T>general_event(T t) : QEvent((QEvent::Type)B)
		{
			type = t;
		}
		S sub_type(){ return type; }
	private:
		S type;
};
#endif // GENERAL_EVENTS_H
