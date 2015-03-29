#include <QHeaderView>
#include <QColorDialog>
#include <QMenu>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFileDialog>

#include "bookmark_panel.h"
#include "hex_editor.h"
#include "debug.h"

bookmark_panel::bookmark_panel(panel_manager *parent, hex_editor *editor) :
        QTableView(parent), abstract_panel(parent, editor)
{
	QFontMetrics metrics(QApplication::font(address_input));
	QStringList labels;
	labels << "Address" << "Color" << "Description";
	model->setHorizontalHeaderLabels(labels);
	setSortingEnabled(true);
	
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
	
	connect(color_button, &QPushButton::clicked, this, &bookmark_panel::color_clicked);
	connect(address_input, &QLineEdit::textChanged, this, &bookmark_panel::address_updated);
	
	connect(add_button, &QPushButton::clicked, this, &bookmark_panel::add_clicked);
	connect(update_button, &QPushButton::clicked, this, &bookmark_panel::update_clicked);
	
	connect(this, &bookmark_panel::clicked, this, &bookmark_panel::row_clicked);
	connect(this, &bookmark_panel::doubleClicked, this, &bookmark_panel::row_double_clicked);
	
	update_button->hide();
	setEditTriggers(QAbstractItemView::NoEditTriggers);
	address_input->setInputMask("$HH:HHHH");
	size_input->setInputMask("999999");
	
	address_input->setMinimumWidth(address_width);
	address_input->setMaximumWidth(address_width);
	
	size_input->setMinimumWidth(metrics.width("2222222") + input_padding);
	size_input->setMaximumWidth(metrics.width("2222222") + input_padding);
	
	data_type->addItem("Code: Unknown A/Index", bookmark_data::CODE | bookmark_data::UNKNOWN);
	data_type->addItem("Code: 8 bit A, 8 bit index", bookmark_data::CODE);
	data_type->addItem("Code: 8 bit A, 16 bit index", bookmark_data::CODE | bookmark_data::I);
	data_type->addItem("Code: 16 bit A, 8 bit index", bookmark_data::CODE | bookmark_data::A);
	data_type->addItem("Code: 16 bit A, 16 bit index", bookmark_data::CODE | bookmark_data::A | bookmark_data::I);
	data_type->addItem("Data: packed byte", bookmark_data::PACKED | bookmark_data::BYTE);
	data_type->addItem("Data: packed word", bookmark_data::PACKED | bookmark_data::WORD);
	data_type->addItem("Data: packed long", bookmark_data::PACKED | bookmark_data::LONG);
	data_type->addItem("Data: packed double", bookmark_data::PACKED | bookmark_data::DOUBLE);
	data_type->addItem("Data: byte", bookmark_data::BYTE);
	data_type->addItem("Data: word", bookmark_data::WORD);
	data_type->addItem("Data: long", bookmark_data::LONG);
	data_type->addItem("Data: double", bookmark_data::DOUBLE);
	
	init_grid_layout();
	editor->get_buffer()->set_bookmark_map(&bookmarks);
	
	setContextMenuPolicy(Qt::CustomContextMenu);
	connect(this, &bookmark_panel::customContextMenuRequested, this, &bookmark_panel::context_menu);
}

void bookmark_panel::color_clicked()
{
	set_color_button(QColorDialog::getColor());
}

void bookmark_panel::address_updated(QString address)
{
	if(bookmarks.contains(address)){
		add_button->hide();
		update_button->show();
	}else{
		update_button->hide();
		add_button->show();
	}
}

void bookmark_panel::context_menu(const QPoint& position)
{	
	QMenu menu;
	if(indexAt(position).isValid()){
		menu.addAction("Delete", this, SLOT(delete_item()));
		menu.addSeparator();
	}
	menu.addAction("Open Bookmark Library", this, SLOT(read_json()));
	menu.addAction("Save Bookmark Library", this, SLOT(write_json()));
	menu.addAction("Save Bookmark Library As", this, SLOT(write_as_json()));
	menu.exec(mapToGlobal(position));
}

void bookmark_panel::add_clicked()
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
	bookmark.data_type = (bookmark_data::types)data_type->currentData().toInt();
	
	add_bookmark(address_input->text(), bookmark);
	bookmarks.insert(address_input->text(), bookmark);
	
	add_button->hide();
	update_button->show();
	selectRow(row-1);
	
	active_editor->update_window();
}

void bookmark_panel::update_clicked()
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

void bookmark_panel::row_clicked(QModelIndex index)
{	
	active_row = index.row();
	QModelIndex address_index = model->index(active_row, 0, QModelIndex());
	
	bookmark_data bookmark = bookmarks[address_index.data().toString()];
	
	size_input->setText(QString::number(bookmark.size));
	description_input->setPlainText(bookmark.description);
	set_color_button(bookmark.color);
	address_input->setText(address_index.data().toString());
	data_type->setCurrentIndex(data_type->findData(bookmark.data_type));
}

void bookmark_panel::row_double_clicked(QModelIndex index)
{	
	active_row = index.row();
	QModelIndex address_index = model->index(active_row, 0, QModelIndex());
	active_editor->goto_offset(check_address(address_index.data().toString()));
}

void bookmark_panel::delete_item()
{
	foreach(QModelIndex i, selectionModel()->selectedRows()){
		bookmarks.remove(i.data().toString());
		model->removeRow(i.row());
	}
}

void bookmark_panel::create_bookmark(int start, int end, const ROM_buffer *buffer)
{
	size_input->setText(QString::number(end - start));
	description_input->setPlainText("");
	address_input->setText(buffer->get_formatted_address(start));
	
	show();
	if(!state){
		state = true;
		toggle_event(BOOKMARKS);
	}
}

QLayout *bookmark_panel::get_layout()
{
	box->addWidget(this);
	box->addWidget(input_area);
	return box;
}

void bookmark_panel::add_bookmark(QString address, bookmark_data bookmark)
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

void bookmark_panel::init_grid_layout()
{
	QGridLayout *grid = new QGridLayout(this);
	grid->addWidget(address_label, 0, 0, 1, 1, Qt::AlignRight);
	grid->addWidget(address_input, 0, 1);
	grid->addWidget(size_label, 0, 2, 1, 1, Qt::AlignRight);
	grid->addWidget(size_input, 0, 3);
	grid->addWidget(description_label, 1, 0, 1, 2);
	grid->addWidget(data_type, 1, 2, 1, 2, Qt::AlignRight);
	grid->addWidget(description_input, 2, 0, 1, 4);
	grid->addWidget(color_label, 3, 0, 1, 1, Qt::AlignRight);
	grid->addWidget(color_button, 3, 1);
	grid->addWidget(add_button, 3, 2, 1, 2);
	grid->addWidget(update_button, 3, 2, 1, 2);
	
	
	input_area->setLayout(grid);
}

void bookmark_panel::set_color_button(QColor color)
{
	if(color.isValid()){
		QPalette palette;
		palette.setColor(QPalette::Button, color);
		
		color_button->setPalette(palette);
	}
}

int bookmark_panel::check_address(QString address)
{
	bool status;
	int clean = address.remove(QRegExp("[^0-9A-Fa-f]")).toInt(&status, 16);
	if(active_editor->get_buffer()->validate_address(clean)){
		return clean;
	}
	return -1;
}

void bookmark_panel::read_json()
{
	QString name = QFileDialog::getOpenFileName(this, "Open Bookmark library", QDir::currentPath(), 
                                            "Shex Bookmark Library (*.sbl);;JSON files (*.json);;All files(*)");
	if(name == ""){
		return; //user canceled or gave an invalid file
	}
	QFile file(name);
	if (!file.open(QIODevice::ReadOnly)){
		return;  //Not a chance recovering from this one
	}
	QByteArray file_data = file.readAll();
	QJsonDocument json = QJsonDocument::fromJson(file_data);
	if(!json.isArray()){
		return; //abandon hope
	}
	file_name = name; //Don't change the name until we are about to actually load data
	bookmarks.clear();
	model->setRowCount(0);
	foreach(QJsonValue value, json.array()){
		if(!value.isObject()){
			continue; //Ignore this object
		}
		QJsonObject bookmark_json = value.toObject();
		
		if(!bookmark_json["address"].isDouble() ||
		   !bookmark_json["size"].isDouble() ||
		   !bookmark_json["description"].isString() ||
		   !bookmark_json["type"].isDouble() ||
		   !bookmark_json["color"].isDouble()){
			continue; //Ignore this object
		}
		bookmark_data bookmark;
		bookmark.address = bookmark_json["address"].toInt();
		bookmark.size = bookmark_json["size"].toInt();
		bookmark.description = bookmark_json["description"].toString();
		bookmark.data_type = (bookmark_data::types)bookmark_json["type"].toInt();
		bookmark.color = QColor(bookmark_json["color"].toInt());
		
		QString address = active_editor->get_buffer()->get_formatted_address(
		                        active_editor->get_buffer()->snes_to_pc(bookmark.address));
		bookmarks[address] = bookmark;
		add_bookmark(address, bookmark);
	}
}

void bookmark_panel::write_json(bool save_as)
{
	if(save_as || file_name.isEmpty()){
		QString filter = "Shex Bookmark Library (*.sbl);;JSON files (*.json);;All files(*)";
		file_name = QFileDialog::getSaveFileName(this, "Save Bookmark library", QDir::currentPath(), 
							filter, &filter, QFileDialog::DontUseNativeDialog);
	}
	
	QFile file(file_name);
	if (file_name.isEmpty() || !file.open(QIODevice::WriteOnly)){
		return;  //Not a chance recovering from this one
	}
	QJsonArray json_data;
	foreach(bookmark_data bookmark, bookmarks){
		json_data.append(QJsonObject{
		                          {"address", bookmark.address},
		                          {"size", bookmark.size},
		                          {"description", bookmark.description},
		                          {"type", bookmark.data_type},
		                          {"color", (int)bookmark.color.rgb()},
		                  });
	}
	QJsonDocument json;
	json.setArray(json_data);
	file.write(json.toJson());
}

void bookmark_panel::write_as_json()
{
	write_json(true);
}

bool bookmark_panel::state = false;
