#ifndef TEXT_DISPLAY_H
#define TEXT_DISPLAY_H

#include <QWidget>

class text_display : public QWidget
{
		Q_OBJECT
	public:
		explicit text_display(QWidget *parent = 0);
	protected:
		virtual void paintEvent(QPaintEvent *event);
		
	signals:
		
	public slots:
		
};

#endif // TEXT_DISPLAY_H
