#include "bookmarks.h"
#include "hex_editor.h"
#include <QHeaderView>
#include <QStringList>
#include <QColorDialog>
#include <QFontMetrics>
#include "debug.h"

bookmarks::bookmarks(hex_editor *parent) :
        QTableView(parent)
{
	editor = parent;
	QFontMetrics metrics(QApplication::font(address_input));
	QStringList labels;
	labels << "Address" << "Color" << "Description";
	model->setHorizontalHeaderLabels(labels);
	setSortingEnabled(true);
	
	if(!display){
		hide();
		input_area->hide();
	}
	
	setModel(model);
	verticalHeader()->hide();
	setSelectionBehavior(QAbstractItemView::SelectRows);
	
	horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	int address_width = metrics.width("$AA:AAAA") + input_padding;
	int color_width = horizontalHeader()->sectionSize(1);
	
	horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	horizontalHeader()->resizeSection(0, address_width);
	horizontalHeader()->resizeSection(1, color_width);
	horizontalHeader()->setStretchLastSection(true);

	color_button->setAutoFillBackground(true);
	color_button->setFlat(true);
	
	connect(color_button, &QPushButton::clicked, this, &bookmarks::color_clicked);
	connect(address_input, &QLineEdit::textChanged, this, &bookmarks::address_updated);
	
	connect(add_button, &QPushButton::clicked, this, &bookmarks::add_clicked);
	connect(update_button, &QPushButton::clicked, this, &bookmarks::update_clicked);
	connect(reload_button, &QPushButton::clicked, this, &bookmarks::reload_clicked);
	
	connect(this, &bookmarks::clicked, this, &bookmarks::row_clicked);
	
	update_button->hide();
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	code_button->setChecked(true);
	address_input->setInputMask("$HH:HHHH");
	size_input->setInputMask("999999");
	
	address_input->setMinimumWidth(address_width);
	address_input->setMaximumWidth(address_width);
	
	size_input->setMinimumWidth(metrics.width("2222222") + input_padding);
	size_input->setMaximumWidth(metrics.width("2222222") + input_padding);
	
	init_grid_layout();
}

void bookmarks::color_clicked()
{
	set_color_button(QColorDialog::getColor());
}

void bookmarks::address_updated(QString address)
{
	if(bookmark_data_map.contains(address)){
		add_button->hide();
		update_button->show();
	}else{
		update_button->hide();
		add_button->show();
	}
}

void bookmarks::add_clicked()
{
	QString description = description_input->toPlainText();
	int clean_address = check_address(address_input->text());
	if(clean_address == -1){
		return;
	}
	
	bookmark_data bookmark;
	bookmark.address = clean_address;
	bookmark.color = color_button->palette().button().color();
	bookmark.size = size_input->text().toInt();
	bookmark.description = description;
	bookmark.code = code_button->isChecked();
	
	add_bookmark(address_input->text(), bookmark);
	bookmark_data_map.insert(address_input->text(), bookmark);
	
	add_button->hide();
	update_button->show();
	selectRow(row-1);
	
	editor->update_window();
}

void bookmarks::update_clicked()
{
	int clean_address = check_address(address_input->text());
	if(clean_address == -1){
		return;
	}
	for(int i = 0; i < row; i++){
		QModelIndex address_index = model->index(i, 0, QModelIndex());
		if(address_index.data().toString() == address_input->text()){
			model->removeRow(i);
			break;
		}
	}
	row--;
	add_clicked();
}

void bookmarks::reload_clicked()
{
	row_clicked(model->index(active_row, 0, QModelIndex()));
}

void bookmarks::row_clicked(QModelIndex index)
{	
	active_row = index.row();
	QModelIndex address_index = model->index(active_row, 0, QModelIndex());
	
	bookmark_data bookmark = bookmark_data_map[address_index.data().toString()];
	
	size_input->setText(QString::number(bookmark.size));
	description_input->setPlainText(bookmark.description);
	set_color_button(bookmark.color);
	address_input->setText(address_index.data().toString());
	code_button->setChecked(bookmark.code);
	data_button->setChecked(!bookmark.code);
}

void bookmarks::create_bookmark(int start, int end, const ROM_buffer *buffer)
{
	size_input->setText(QString::number(end - start));
	description_input->setPlainText("");
	address_input->setText(buffer->get_formatted_address(start));
	code_button->setChecked(true);
	
	show();
	toggle_display(true);
}

void bookmarks::toggle_display(bool state) { 
	if(state == display){
		return;
	}
	setVisible(state);
	
	input_area->setVisible(state);
	
	display = state;
	layout_adjust();
}

QVBoxLayout *bookmarks::get_layout()
{
	box->addWidget(this);
	box->addWidget(input_area);
	return box;
}

void bookmarks::add_bookmark(QString address, bookmark_data bookmark)
{
	model->setItem(row, 0, new QStandardItem(address));
	QStandardItem *color = new QStandardItem();
	color->setBackground(QBrush(bookmark.color));
	color->setForeground(QBrush(bookmark.color));
	color->setText(bookmark.color.name());
	model->setItem(row, 1, color);
	model->setItem(row, 2, new QStandardItem(bookmark.description));
	row++;
}

bool bookmarks::event(QEvent *event)
{
	if(event->type() != (QEvent::Type)BOOK_PANEL_EVENT){
		return QWidget::event(event);
	}
	switch(((book_panel_event *)event)->sub_type()){
		case BOOK_TOGGLE_DISPLAY:
			toggle_display(!display);
			return true;
		default:
			qDebug() << "Bad event" << ((editor_event *)event)->sub_type();
			return false;
	}
}

void bookmarks::init_grid_layout()
{
	grid->addWidget(address_label, 0, 0, 1, 1, Qt::AlignRight);
	grid->addWidget(address_input, 0, 1);
	grid->addWidget(size_label, 0, 2, 1, 1, Qt::AlignRight);
	grid->addWidget(size_input, 0, 3);
	grid->addWidget(description_label, 1, 0, 1, 2);
	grid->addWidget(code_button, 1, 2, 1, 1, Qt::AlignRight);
	grid->addWidget(data_button, 1, 3, 1, 1);
	grid->addWidget(description_input, 2, 0, 1, 4);
	grid->addWidget(color_label, 3, 0, 1, 1, Qt::AlignRight);
	grid->addWidget(color_button, 3, 1);
	grid->addWidget(add_button, 3, 2);
	grid->addWidget(update_button, 3, 2);
	grid->addWidget(reload_button, 3, 3);
	
	
	input_area->setLayout(grid);
}

void bookmarks::set_color_button(QColor color)
{
	if(color.isValid()){
		QPalette palette;
		palette.setColor(QPalette::Button, color);
		
		color_button->setPalette(palette);
	}
}

void bookmarks::layout_adjust()
{
        QWidget *parent = parentWidget();
        while(parent){
		int height = parent->height();
		parent->setUpdatesEnabled(false);
		parent->adjustSize();
		parent->resize(parent->width(), height);
		parent->setUpdatesEnabled(true);
		parent->repaint();
		parent = parent->parentWidget();
	}
}

int bookmarks::check_address(QString address)
{
	bool status;
	int clean = address.remove(QRegExp("[^0-9A-Fa-f]")).toInt(&status, 16);
	if(editor->get_buffer()->validate_address(clean)){
		return clean;
	}
	return -1;
}

bool bookmarks::display = false;
