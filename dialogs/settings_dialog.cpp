#include <type_traits>
#include <QColorDialog>

#include "settings_dialog.h"
#include "rom_buffer.h"
#include "utility.h"

settings_dialog::settings_dialog(QWidget *parent) : abstract_dialog(parent)
{
	connect(refresh_button, &QPushButton::clicked, this, &settings_dialog::refresh);
	connect(apply_button, &QPushButton::clicked, this, &settings_dialog::apply);
	
	auto null_validator = [](auto value){ return value; };
	auto font_validator = [](auto value){ return clamp(value, 6, 15); };
	
	auto make_color = [&](auto value){
		value->setAutoFillBackground(true);
		value->setFlat(true);
		connect(value, &QPushable::clicked, this, [=](){
			QColor color = QColorDialog::getColor();
			if(color.isValid()){
				set_default(value, color);
			}
		});
	};
	
	QColor highlight_color = QApplication::palette().color(QPalette::Active, QPalette::Highlight).lighter();
	
	setting<QLineEdit>("Editor font size", "display/font", font_validator, QApplication::font().pointSize());
	setting<QCheckBox>("Do not prompt on size change:", "editor/size_change", null_validator, false);
	setting<QCheckBox>("Move cursor with mouse wheel:", "editor/wheel_cursor", null_validator, false);
	
	setting<QComboBox>("Default copy type:", "buffer/copy", null_validator, QVariant(SPACES), [](auto copy){
				copy->addItem("No space", NO_SPACES);
				copy->addItem("Spaces", SPACES);
				copy->addItem("Hex format", HEX_FORMAT);
				copy->addItem("Word table", ASM_WORD_TABLE);
				copy->addItem("Byte table", ASM_BYTE_TABLE);
				copy->addItem("Long table", ASM_LONG_TABLE);
				copy->addItem("C source", C_SOURCE);
			});
	setting<QPushable>("Highlight color:", "display/highlight", null_validator, highlight_color, make_color);
	
	int row = layout->rowCount();
	layout->addWidget(refresh_button, row, 0);
	layout->addWidget(apply_button, row, 1);
	setLayout(layout);
	apply();
}

void settings_dialog::refresh()
{
	
}

void settings_dialog::apply()
{
	for(auto & function : setting_functions){
		function();
	}
}

template <typename T, typename V, typename D, typename I>
void settings_dialog::setting(QString name, QString key, V validator, D default_data, I initializer)
{
	QLabel *label = new QLabel(name, this);
	T *widget = new T(this);
	initializer(widget);
	QVariant stored_data = settings.get(key);
	if(stored_data.isValid()){
		if(std::is_same<D, QVariant>::value){
			conditional_variant_copy(default_data, stored_data);
		}else{
			default_data = stored_data.value<D>();	
		}
	}
	
	set_default(widget, default_data);
	setting_functions.append([this, widget, key, validator](){
		settings.set(key, validator(get_data(widget).template value<D>()));
	});
	
	int row = layout->rowCount();
	layout->addWidget(label, row, 0);
	layout->addWidget(widget, row, 1);
}

void settings_dialog::set_color(QPushable *widget, QColor color)
{
	QPalette palette;
	widget->setText(color.name()); 
	palette.setColor(QPalette::Button, color); 
	palette.setColor(QPalette::ButtonText, color);
	widget->setPalette(palette); 
}
