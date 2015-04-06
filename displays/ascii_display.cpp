#include "ascii_display.h"
#include "character_mapper.h"
#include "debug.h"

void ascii_display::paintEvent(QPaintEvent *event)
{
	text_display::paintEvent(event);

}

void ascii_display::keyPressEvent(QKeyEvent *event)
{
	//Let the editor handle hotkeys
	if(event->modifiers() == Qt::ControlModifier){
		event->ignore();
		return;
	}
	if(event->key() >= Qt::Key_Space && event->key() <= Qt::Key_AsciiTilde){
		char key = event->text().at(0).unicode();
		emit character_typed(character_mapper::decode(key), true);
	}else{
		event->ignore();
	}
}

QSize ascii_display::sizeHint () const
{
	return QSize(editor_font::get_width() * line_characters, 200);
}


void ascii_display::get_line(int start, int end, QTextStream &stream)
{
	for(int i = start; i < end; i++){
		if(isprint(character_mapper::encode(buffer->at(i)))){
			stream << (char)character_mapper::encode(buffer->at(i));
		}else{
			stream << ".";
		}
	}
}

int ascii_display::screen_to_nibble(QPoint position, bool byte_align)
{
	Q_UNUSED(byte_align); //ascii is already byte aligned
	int x = position.x() / editor_font::get_width();
	int y = position.y() / editor_font::get_height();
	
	int last_byte = (line_characters - 1);
	if(x > last_byte){
		x = last_byte;
	}
	
	return (x + y * line_characters + get_offset()) * 2;
}

QPoint ascii_display::nibble_to_screen(int nibble)
{
	nibble -= get_offset() * 2;
	int y = nibble / (get_columns() * 2);
	int x = nibble % (get_columns() * 2);
	x /= 2;
	return QPoint(x * editor_font::get_width(), y * editor_font::get_height());
}
