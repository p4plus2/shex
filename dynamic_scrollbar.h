#ifndef DYNAMIC_SCROLLBAR_H
#define DYNAMIC_SCROLLBAR_H

#include <QScrollBar>
#include <QProxyStyle>
#include "qdebug.h"

class scrollbar_style;

class dynamic_scrollbar : public QScrollBar
{
		Q_OBJECT
	public:
		explicit dynamic_scrollbar(QWidget *parent = 0);
		
	signals:
		
	public slots:
		void set_range(int value);
		void toggle_mode(bool m);
		
	private:
		bool mode;
		int original_pagestep;
		
};

class scrollbar_style : public QProxyStyle
{
	public:
		virtual int pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
		{
			if(metric == PM_ScrollBarSliderMin && mode){
				return 100;
			}
			return QCommonStyle::pixelMetric(metric, option, widget);
		}
		void toggle(bool m){ mode = m; }
	private:
		bool mode;
};

#endif // DYNAMIC_SCROLLBAR_H
