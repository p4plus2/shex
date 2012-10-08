#ifndef ROM_BUFFER_H
#define ROM_BUFFER_H

#include <QFile>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>

class ROM_buffer
{
	public:
		ROM_buffer();
		void cut(int start, int end);
		void copy(int start, int end);
		void paste(int start, int end = 0);
		void delete_text(int start, int end = 0);
		void update_nibble(char byte, int position);
		QString get_line(int index, int length);
		QString get_address(int address);
		
		inline int size(){ return buffer.size(); }
		inline char at(int index){ return buffer.at(index); }
		inline bool check_paste_data(){ return !clipboard->mimeData()->hasText(); }
	private:
		QFile ROM;
		QByteArray buffer;
		QClipboard *clipboard;
};

#endif // ROM_BUFFER_H
