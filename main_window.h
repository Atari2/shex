#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QMainWindow>
#include <QUndoGroup>
#include <QFileSystemWatcher>
#include <QQueue>
#include <QMap>
#include "dialog_manager.h"
#include "menu_manager.h"
#include "rom_buffer.h"
#include "editor_font.h"
#include "panels/diff_panel.h"
#include "debug.h"

class hex_editor;
class dynamic_scrollbar;
class panel_manager;

class main_window : public QMainWindow
{
		Q_OBJECT
		
	public:
		main_window(QWidget *parent = 0);
		hex_editor *get_active_editor();
		~main_window();
		QMap<QString, QVector<hex_editor*>> current_files_in_diff{};
	public slots:
		bool close_tab(int i);
		void changed_tab(int i);
		void file_save_state(bool clean);
		void new_file();
		void open();
		void compare_open();
		void generate_patch();
		bool save(bool override_name = false, int target = -1);
		bool watch_file(const QString& name);
		bool unwatch_file(const QString& name);
		void file_state_changed(const QString& path);
	signals:
		void on_file_state_change(const QString& path);
	protected:
		virtual bool event(QEvent *event);
		virtual void closeEvent(QCloseEvent *event);

	private:
		QLabel *statusbar = new QLabel(this);
		QTabWidget *tab_widget = new QTabWidget(this);
		QUndoGroup *undo_group = new QUndoGroup(this);
		dialog_manager *dialog_controller = new dialog_manager(this);
		menu_manager *menu_controller = new menu_manager(this, menuBar(), undo_group);
		int new_counter = 0;
		editor_font *font = new editor_font(this);  //We just need an instance for sending events to
		QString last_directory;
		QFileSystemWatcher *file_watcher;

		QQueue<QString> queue_external_diffs{};
		void init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar, panel_manager *panel);
		void create_new_tab(QString name, bool new_file = false);
		hex_editor *get_editor(int i) const;
		int get_editor_index_by_filename(const QString& path, int start = 0) const;
		
};

#endif // MAIN_WINDOW_H
