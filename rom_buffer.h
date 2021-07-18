#ifndef ROM_BUFFER_H
#define ROM_BUFFER_H

#include <QFileInfo>
#include <QClipboard>
#include <QApplication>
#include <QMimeData>
#include <QUndoGroup>
#include <QUndoStack>

#include "rom_metadata.h"
#include "panels/bookmark_panel.h"

class ROM_buffer : public ROM_metadata
{
	public:
		enum copy_style{
			NO_SPACES,
			SPACES,
			HEX_FORMAT,
			ASM_BYTE_TABLE,
			ASM_WORD_TABLE,
			ASM_LONG_TABLE,
			C_SOURCE
		};
		
		enum search_error{
			NOT_FOUND = -1,
			INVALID_FIND = -2,
			INVALID_REPLACE = -3
		};
		
		ROM_buffer(){}
		ROM_buffer(QString file_name, bool new_file = false);
		virtual ~ROM_buffer(){}
		virtual void remove_copy_header();
		void open(QString path);
		void save(QString path);
		void reload();
		void initialize_undo(QUndoGroup *undo_group);
		void cut(int start, int end, bool ascii_mode);
		void copy(int start, int end, bool ascii_mode);
		QString copy_format(int start, int end, copy_style style);
		int paste(int start, int end = 0, bool raw = false);
		void delete_text(int start, int end = 0);
		void update_nibble(char byte, int position, int delete_start = 0, int delete_end = 0);
		virtual void update_byte(char byte, int position, int delete_start = 0, int delete_end = 0);
		void update_raw_range(int begin, int end, const QByteArray& arr);
		QByteArray get_range(int begin, int end);
		QString get_formatted_address(int address) const;
		int count(QString find, bool mode);
		int search(QString find, int position, bool direction, bool mode);
		int replace(QString find, QString replace, int position, bool direction, bool mode);
		int replace_all(QString find, QString replace, bool mode);
		QVector<int> get_rats_tags() const;
		
		virtual int size() const { return buffer.size(); }
		virtual char at(int index) const { return index == size() ? 0 : buffer.at(index); }
		
		void set_active(){ undo_stack->setActive(); }
		bool is_active(){ return undo_stack->isActive(); }
		bool check_paste_data(){ return clipboard->mimeData()->hasText(); }
		QString get_hex(QString input) { return input.remove(QRegExp("[^0-9A-Fa-f]")); }
		QString load_error() { return ROM_error; }
		QString get_file_name(){ QFileInfo info(ROM); return info.fileName();  }
		QString get_full_path(){ QFileInfo info(ROM); return info.filePath(); }
		QByteArray range(int start, int end) const { return buffer.mid(start/2, (end-start)/2); }
		
		const bookmark_map *get_bookmark_map() const { return bookmarks; }
		void set_bookmark_map(const bookmark_map *b){ bookmarks = b; }
		
		static void set_copy_style(copy_style style){ copy_type = style; }
		
		

	private:
		QFile ROM;
		QByteArray buffer;
		QByteArray header_buffer;
		QUndoStack *undo_stack;
		QString ROM_error = "";
		const bookmark_map *bookmarks = nullptr;
		
		static copy_style copy_type;
		static QClipboard *clipboard;
		
		QByteArray input_to_byte_array(QString input, int mode);
};

#endif // ROM_BUFFER_H
