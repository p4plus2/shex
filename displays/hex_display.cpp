#include "hex_display.h"
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

QSize hex_display::sizeHint () const
{
	const int padding = 20;
	return QSize(get_font_width() * line_characters + padding, get_font_height() * get_rows());
}


void hex_display::get_line(int start, int end, QTextStream &stream)
{
	for(int i = start; i < end; i++){
		stream << QString::number((unsigned char)buffer->at(i),16).rightJustified(2, '0').toUpper() << " ";
	}
}

//TODO possibly clean this up more
int hex_display::screen_to_nibble(QPoint position, bool byte_align)
{
	int x_remainder = position.x() % get_font_width();
	int x = position.x() / get_font_width();
	int y = position.y() / get_font_height();
	if(x % 3 == 2){
		x += ((x_remainder - get_font_width() / 2) < 0 ? -1 : 1);
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
	return QPoint(x * get_font_width(), y * get_font_height());
}
