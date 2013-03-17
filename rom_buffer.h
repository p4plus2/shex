#ifndef ROM_BUFFER_H
#define ROM_BUFFER_H

#include "rom_metadata.h"

#include <QFile>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QUndoGroup>
#include <QUndoStack>

class ROM_buffer : public ROM_metadata
{
	public:
		ROM_buffer(QString file_name);
		virtual ~ROM_buffer(){}
		void initialize_undo(QUndoGroup *undo_group);
		void cut(int start, int end);
		void copy(int start, int end);
		int paste(int start, int end = 0, bool raw = false);
		void delete_text(int start, int end = 0);
		void update_nibble(char byte, int position, int delete_start = 0, int delete_end = 0);
		virtual void update_byte(char byte, int position, int delete_start = 0, int delete_end = 0);
		QString get_line(int index, int length);
		QString get_formatted_address(int address);
		
		inline QString get_file_name(){ return ROM.fileName(); }
		inline virtual int size(){ return buffer.size() - header_size(); }
		inline virtual char at(int index){ return index == size() ? 0 : buffer.at(index + header_size()); }
		inline bool check_paste_data(){ return !clipboard->mimeData()->hasText(); }
		inline void set_active(){ undo_stack->setActive(); }
		
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
		QClipboard *clipboard = QApplication::clipboard();
		QUndoStack *undo_stack;
		paste_style paste_type = NO_SPACES;
};

#endif // ROM_BUFFER_H
