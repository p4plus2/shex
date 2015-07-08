#include "hex_display.h"
#include "utility.h"
#include "debug.h"

void hex_display::paintEvent(QPaintEvent *event)
{
	text_display::paintEvent(event);
}

void hex_display::keyPressEvent(QKeyEvent *event)
{
	//Let the editor handle hotkeys
	if(event->modifiers() == Qt::ControlModifier){
		event->ignore();
		return;
	}
	if(event->key() >= Qt::Key_0 && event->key() <= Qt::Key_9){
		emit character_typed(event->key() - Qt::Key_0, false);
	}else if(event->key() >= Qt::Key_A && event->key() <= Qt::Key_F){
		emit character_typed(event->key() - Qt::Key_A + 10, false);
	}else{
		event->ignore();
	}
}

void hex_display::get_line(int start, int end, QTextStream &stream)
{
	for(int i = start; i < end; i++){
		stream << to_hex((unsigned char)buffer->at(i)) << " ";
	}
}

//TODO possibly clean this up more
int hex_display::screen_to_nibble(QPoint position, bool byte_align)
{
	int x_remainder = position.x() % editor_font::get_width();
	int x = position.x() / editor_font::get_width();
	int y = position.y() / editor_font::get_height();
	if(x % 3 == 2){
		x += ((x_remainder - editor_font::get_width() / 2) < 0 ? -1 : 1);
	}
	
	int last_byte = (line_characters - 1);
	if(x > last_byte){
		x = last_byte;
	}
	
	if(byte_align && x % 3 == 1){
		x -= 1;
	}

	return (x * 2) / 3 + (x % 3 == 1 && !byte_align) + y * get_columns() * 2 + get_offset() * 2;
}

QPoint hex_display::nibble_to_screen(int nibble)
{
	nibble -= get_offset() * 2;
	int y = nibble / (get_columns() * 2);
	int x = nibble % (get_columns() * 2);
	x = ((x / 2 * 3) + (x & 1));
	return QPoint(x * editor_font::get_width(), y * editor_font::get_height());
}
