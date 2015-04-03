#ifndef SETTINGS_DIALOG_H
#define SETTINGS_DIALOG_H

#include <functional>
#include <QCheckBox>

#include "abstract_dialog.h"
#include "settings_manager.h"

using QPushable = QPushButton;

class settings_dialog : public abstract_dialog
{
		Q_OBJECT
	public:
		explicit settings_dialog(QWidget *parent);
		
	public slots:
		void refresh();
		void apply();
		
	private:
		settings_manager settings;
		QGridLayout *layout = new QGridLayout(this);
		
		QPushButton *refresh_button = new QPushButton("Refresh", this);
		QPushButton *apply_button = new QPushButton("Apply", this);
		QVector<std::function<void()>> setting_functions;
		
		template <typename T, typename V, typename D, typename I = std::function<void(QWidget *)>>
		void setting(QString name, QString key, V validator, D default_data, I initializer = [](QWidget *){});
		
		void set_default(QComboBox *widget, QVariant d){ widget->setCurrentIndex(widget->findData(d)); }
		void set_default(QCheckBox *widget, bool d){ widget->setCheckState(d ? Qt::Checked : Qt::Unchecked); }
		void set_default(QLineEdit *widget, QString d){ widget->setText(d); }
		void set_default(QLineEdit *widget, int d){ widget->setText(QString::number(d)); }
		void set_default(QPushable *widget, QColor d){ set_color(widget, d); }
		
		QVariant get_data(QComboBox *widget){ return widget->currentData(); }
		QVariant get_data(QCheckBox *widget){ return widget->checkState(); }
		QVariant get_data(QLineEdit *widget){ return widget->text(); }
		QVariant get_data(QPushable *widget){ return widget->text(); }
		
		void set_color(QPushable *widget, QColor color);
};

#endif // SETTINGS_DIALOG_H
