#ifndef EDITOR_FONT_H
#define EDITOR_FONT_H

#include <QObject>
#include <QFont>

class editor_font : public QObject
{
		Q_OBJECT
	public:
		explicit editor_font(QObject *parent = 0);
		
		static int get_width(){ return font_width; }
		static int get_height(){ return font_height; }
		static QFont get_font(){ return font; }
		static editor_font *instance(){ return self; }
		
	signals:
		void font_changed();
		
	protected:
		virtual bool event(QEvent *event);
		
	private:
		static QFont font;
		static int font_width;
		static int font_height;
		static int default_font_size;
		
		static editor_font *self;
		
		void font_setup();
};

#endif // EDITOR_FONT_H
