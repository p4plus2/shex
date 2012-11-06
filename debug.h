#ifndef DEBUG_OPTIONS_H
#define DEBUG_OPTIONS_H

#include <QDebug>

#if 0
	#define LOG_TO_FILE
#endif

#if 1
	#define USE_DEFAULT_ROM
#endif

#if 1
#include <QMetaEnum>
#define ENUM_STRING(ENUM, VALUE) \
	staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator(#ENUM)).valueToKey(VALUE);
#endif

#endif // DEBUG_OPTIONS_H
