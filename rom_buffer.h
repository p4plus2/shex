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
		void paste(int start, int end = 0, bool raw = false);
		void delete_text(int start, int end = 0);
		void update_nibble(char byte, int position);
		QString get_line(int index, int length);
		QString get_address(int address);
		
		inline int size(){ return buffer.size(); }
		inline char at(int index){ return buffer.at(index); }
		inline bool check_paste_data(){ return !clipboard->mimeData()->hasText(); }
		
		enum paste_style{
			NO_SPACES,
			SPACES,
			HEX_FORMAT,
			ASM_BYTE_TABLE,
			ASM_WORD_TABLE,
			ASM_LONG_TABLE,
			C_SOURCE
		};
	private:
		QFile ROM;
		QByteArray buffer;
		QClipboard *clipboard;
		paste_style paste_type;
};

#endif // ROM_BUFFER_H
