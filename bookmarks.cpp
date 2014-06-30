#include "bookmarks.h"
#include <QHeaderView>
#include <QStringList>
#include <QColorDialog>
#include <QFontMetrics>
#include "debug.h"

bookmarks::bookmarks(QWidget *parent) :
        QTableView(parent)
{
	QStringList labels;
	labels << "Address" << "Description";
	model->setHorizontalHeaderLabels(labels);
	
	if(!display){
		hide();
		input_area->hide();
	}
	
	setModel(model);
	verticalHeader()->hide();
	setSelectionBehavior(QAbstractItemView::SelectRows);
	
	horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
	int address_width = horizontalHeader()->sectionSize(0);
	
	horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
	horizontalHeader()->resizeSection(0, address_width);
	horizontalHeader()->setStretchLastSection(true);

	color_button->setAutoFillBackground(true);
	color_button->setFlat(true);
	
	connect(color_button, SIGNAL(clicked()), this, SLOT(color_clicked()));
	connect(address_input, SIGNAL(textChanged(QString)), this, SLOT(address_updated(QString)));
	
	connect(add_button, SIGNAL(clicked()), this, SLOT(add_clicked()));
	connect(update_button, SIGNAL(clicked()), this, SLOT(update_clicked()));
	connect(reload_button, SIGNAL(clicked()), this, SLOT(reload_clicked()));
	
	connect(this, SIGNAL(clicked(QModelIndex)), this, SLOT(row_clicked(QModelIndex)));
	
	update_button->hide();
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	code->setChecked(true);
	address_input->setInputMask("$HH:HHHH");
	size_input->setInputMask("999999");
	
	QFontMetrics metrics(QApplication::font(address_input));
	
	address_input->setMinimumWidth(metrics.width("$AA:AAAA") + 3); //pad this a bit, it looks better
	address_input->setMaximumWidth(metrics.width("$AA:AAAA") + 3);
	
	size_input->setMinimumWidth(metrics.width("2222222") + 3);
	size_input->setMaximumWidth(metrics.width("2222222") + 3);
	
	init_grid_layout();
}

void bookmarks::color_clicked()
{
	set_color_button(QColorDialog::getColor());
}

void bookmarks::address_updated(QString address)
{
	if(bookmark_map.contains(address)){
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
	add_bookmark(address_input->text(), description);
	
	bookmark_data bookmark;
	
	bookmark.color = color_button->palette().button().color();
	bookmark.size = size_input->text().toInt();
	bookmark.description = description;
	bookmark.code = code->isChecked();
	
	bookmark_map.insert(address_input->text(), bookmark);
	
	add_button->hide();
	update_button->show();
	selectRow(row-1);
}

void bookmarks::update_clicked()
{
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
	
	bookmark_data bookmark = bookmark_map[address_index.data().toString()];
	
	size_input->setText(QString::number(bookmark.size));
	description_input->setPlainText(bookmark.description);
	set_color_button(bookmark.color);
	address_input->setText(address_index.data().toString());
	code->setChecked(bookmark.code);
	data->setChecked(!bookmark.code);
}

void bookmarks::create_bookmark(int start, int end, const ROM_buffer *buffer)
{
	size_input->setText(QString::number(end - start));
	description_input->setPlainText("");
	address_input->setText(buffer->get_formatted_address(start));
	code->setChecked(true);
	
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

void bookmarks::add_bookmark(QString address, QString description)
{
	model->setItem(row, 0, new QStandardItem(address));
	model->setItem(row, 1, new QStandardItem(description));
	row++;
}

void bookmarks::init_grid_layout()
{
	grid->addWidget(address_label, 0, 0, 1, 1, Qt::AlignRight);
	grid->addWidget(address_input, 0, 1);
	grid->addWidget(size_label, 0, 2, 1, 1, Qt::AlignRight);
	grid->addWidget(size_input, 0, 3);
	grid->addWidget(description_label, 1, 0, 1, 2);
	grid->addWidget(code, 1, 2, 1, 1, Qt::AlignRight);
	grid->addWidget(data, 1, 3, 1, 1);
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
	QPalette palette;
	palette.setColor(QPalette::Button, color);
	
	color_button->setPalette(palette);
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

bool bookmarks::display = false;
