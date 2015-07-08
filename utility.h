#ifndef UTILITY_H
#define UTILITY_H

#include <QWidget>
#include <QLayout>
#include <QDir>

template<typename... A>
struct resolve{ 
    template<typename C, typename F> 
    static constexpr auto from(F (C::*function)(A...)) -> decltype(function)
    { 
        return function;
    } 
};

template<typename T>
T clamp(T value, T low, T high)
{
	return (value < low) ? low :
	       (value > high) ? high :
	       value;
}

template <typename T1, typename T2> void conditional_variant_copy(T1 &a, T2 &b){ Q_UNUSED(a); Q_UNUSED(b); }
template <typename T = QVariant> void conditional_variant_copy(T &a, T &b)
{
	int type = a.type();
	a = b;
	a.convert(type);
}

inline void propagate_resize(QWidget *child)
{
	QWidget *parent = child->parentWidget();
	if(parent->layout()){
		parent->layout()->invalidate();
	}
	do{
		parent->adjustSize();
		parent = parent->parentWidget();
	}while(parent);
}

inline QString absolute_path(const QString file_path)
{
	QDir directory;
	return directory.absoluteFilePath(file_path);
}

inline QString to_hex(int data, int pad = 2)
{
	QString hex = QString::number(data, 16).rightJustified(pad, '0').toUpper();
	hex.truncate(pad);
	return hex;
}

//add optional endian flip in the future maybe
inline unsigned short read_word(const QByteArray &data, int offset)
{
	return (unsigned char)data.at(offset) | ((unsigned char)data.at(offset+1) << 8);
}

inline unsigned int read_long(const QByteArray &data, int offset)
{
	return (unsigned char)data.at(offset) | ((unsigned char)data.at(offset+1) << 8) | 
						((unsigned char)data.at(offset+2) << 16);
}
#endif // UTILITY_H
