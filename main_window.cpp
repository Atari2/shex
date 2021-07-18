#include <QStatusBar>
#include <QTabWidget>
#include <QFileDialog>
#include <QDesktopWidget>

#include "main_window.h"
#include "hex_editor.h"
#include "dynamic_scrollbar.h"
#include "version.h"
#include "debug.h"
#include "character_mapper.h"
#include "panel_manager.h"
#include "settings_manager.h"
#include "utility.h"

main_window::main_window(QWidget *parent)
        : QMainWindow(parent)
{
	statusBar()->addWidget(statusbar);

	file_watcher = new QFileSystemWatcher(this);
	connect(file_watcher, &QFileSystemWatcher::fileChanged, this, &main_window::file_state_changed);
	QWidget* widget = new QWidget(this);
	QHBoxLayout *tab_layout = new QHBoxLayout(widget);
	tab_layout->addWidget(tab_widget);
	widget->setLayout(tab_layout);
	setCentralWidget(widget);
	tab_widget->setTabsClosable(true);
	tab_widget->setMovable(true);
	
	settings_manager settings;
	QVariant geometry = settings.get("geometry");
	QVariant state = settings.get("window_state");
	QVariant saved_last_directory = settings.get("last_directory");
	if(state.isValid()){
		restoreState(state.toByteArray());
	}
	if(geometry.isValid()){
		restoreGeometry(geometry.toByteArray());
		resize(600, size().height());
		tab_widget->resize(size());
	}
	
	if(saved_last_directory.isValid()){
		last_directory = saved_last_directory.toString();
	}else{
		last_directory = QDir::homePath();
	}
	
	connect(tab_widget, &QTabWidget::tabCloseRequested, this, &main_window::close_tab);
	connect(tab_widget, &QTabWidget::currentChanged, this, &main_window::changed_tab);
	menu_controller->connect_to_widget(this, WINDOW_EVENT);
	menu_controller->connect_to_widget(dialog_controller, DIALOG_EVENT);
#ifdef USE_DEFAULT_ROM
	create_new_tab("SMW.smc");
	create_new_tab("speedy.sfc");
#endif
}

hex_editor *main_window::get_active_editor()
{
	int current_tab = tab_widget->currentIndex();
	if(current_tab != -1){
		return get_editor(current_tab);
	}
	return nullptr;
}

bool main_window::close_tab(int i)
{
	hex_editor *editor = get_editor(i);
	if(editor->can_save()){
		typedef QMessageBox message;
		QString name = editor->get_file_name();
		int button = message::warning(this, "Save", "Do you wish to save any unsaved changes to " + name + "?", 
		                              message::Yes | message::No | message::Cancel, message::Yes);
		switch(button){
			case message::Yes:
				if(!save(i)){
					return false;
				}
				
			break;
			case message::Cancel:
				return false;
			break;
			default:
			break;
		}
	}
	unwatch_file(editor->get_file_name());
	QWidget *widget = tab_widget->widget(i);
	tab_widget->removeTab(i);
	delete widget;
	return true;
}

void main_window::changed_tab(int i)
{
	if(i == -1){
		dialog_controller->set_active_editor(nullptr);
		menu_controller->connect_to_widget(nullptr, EDITOR_EVENT);
		return;
	}
	
	hex_editor *editor = get_editor(i);

	editor->set_focus();
	dialog_controller->set_active_editor(editor);
	menu_controller->connect_to_widget(editor, EDITOR_EVENT);
}

void main_window::file_save_state(bool clean)
{
	hex_editor *editor = get_editor(tab_widget->currentIndex());
	if(clean){
		tab_widget->setTabText(tab_widget->currentIndex(), editor->get_file_name());
	}else{
		tab_widget->setTabText(tab_widget->currentIndex(), "* "+editor->get_file_name());
	}
}

void main_window::new_file()
{
	new_counter++;
	create_new_tab("Untitled_"+QString::number(new_counter), true);
}

void main_window::open()
{
	QStringList file_list = QFileDialog::getOpenFileNames(this, "Open file(s)", last_directory,
	                                                      "ROM files (*.smc *.sfc);;All files(*.*)");
	for(auto &file_name : file_list){
		create_new_tab(file_name);
	}
	
	if(file_list.size()){
		last_directory = absolute_path(file_list.at(0));
	}
}

void main_window::compare_open()
{
	QString file = QFileDialog::getOpenFileName(this, "Open file to compare", last_directory,
	                                                  "ROM files (*.smc *.sfc);;All files(*.*)");
	if(!file.isNull()){
		get_editor(tab_widget->currentIndex())->compare(file);
		last_directory = absolute_path(file);
	}
}

void main_window::generate_patch()
{
	hex_editor *editor = get_editor(tab_widget->currentIndex());
	if(!editor->is_comparing()){
		return;
	}
	QString file_name = QFileDialog::getSaveFileName(this, "Save patch as", last_directory,
	                                                  "ASM files (*.asm *.txt);;All files(*.*)");
	if(!file_name.isNull()){
		QString patch = editor->generate_patch();
		QFile file(file_name);
		file.open(QFile::WriteOnly);
		file.write(patch.toLatin1());
		last_directory = absolute_path(file_name);
	}
}

bool main_window::save(bool override_name, int target)
{
	hex_editor *editor = (target != -1 ) ? get_editor(target) : get_editor(tab_widget->currentIndex());
	QString name = "";
	if(editor->new_file() || override_name){
		name = QFileDialog::getSaveFileName(this, "Save", last_directory, 
	                                            "ROM files (*.smc *.sfc);;All files(*.*)");
		if(name == ""){
			return false;
		}
		last_directory = absolute_path(name);
	}
	unwatch_file(get_active_editor()->get_file_name());
	editor->save(name);
	if (editor->load_error() != "") {
		QMessageBox::critical(this, "Invalid ROM", editor->load_error(), QMessageBox::Ok);
		return false;
	}
	watch_file(get_active_editor()->get_file_name());
	return true;
}

bool main_window::watch_file(const QString& name) {
	return file_watcher->addPath(name);
}

bool main_window::unwatch_file(const QString& name) {
	return file_watcher->removePath(name);
}

void main_window::file_state_changed(const QString& path) {
	QFileInfo info{path};
	QString filename = info.fileName();
	if (!info.exists()) {
		int button = QMessageBox::warning(this, "Warning", "File " + filename + " was removed from disk or renamed, do you want to keep it open in the editor?",
                                        QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (button == QMessageBox::No) {
			close_tab(get_editor_index_by_filename(filename));
		}
	}
	else {
		int button = QMessageBox::warning(this, "Warning", "File " + filename + " was modified outside of shex, do you want to load external changes?",
                                               QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		if (button == QMessageBox::Yes) {
			int index = get_editor_index_by_filename(filename);
			hex_editor* editor = get_editor(index);
			hex_editor placeholder{nullptr, "", undo_group, true};
			tab_widget->widget(index)->layout()->replaceWidget(editor, &placeholder);
			diff_panel::diff_show(editor, path, this);
			tab_widget->widget(index)->layout()->replaceWidget(&placeholder, editor);
			tab_widget->setTabText(index, editor->get_file_name());
			// editor->reload();
			if (editor->load_error() != "") {
				QMessageBox::critical(this, "Invalid ROM", editor->load_error(), QMessageBox::Ok);
			}
		}
	}
}

bool main_window::event(QEvent *event)
{
	if(event->type() != (QEvent::Type)WINDOW_EVENT){
		return QWidget::event(event);
	}
	switch(((window_event *)event)->sub_type()){
		case NEW:
			new_file();
			return true;
	        case OPEN:
			open();
			return true;
		case OPEN_COMPARE:
			compare_open();
			return true;
		case DIFF_PATCH:
			generate_patch();
			return true;
	        case SAVE:
			save();
			return true;
	        case SAVE_AS:
			save(true);
			return true;
	        case CLOSE_TAB:
			close_tab(tab_widget->currentIndex());
			return true;
	        case CLOSE:
			close();
			return true;
	        case VERSION:
			display_version_dialog();
			return true;
		default:
			qDebug() << "Bad event" << ((editor_event *)event)->sub_type();
			return false;
	}
}

void main_window::closeEvent(QCloseEvent *event)
{
	while(tab_widget->count()){
		if(!close_tab(0)){
			event->setAccepted(false);
			return;
		}
	}
	settings_manager settings;
	settings.set("geometry", saveGeometry());
	settings.set("window_state", saveState());
	settings.set("last_directory", last_directory);
	QApplication::quit();
	QMainWindow::closeEvent(event);
}

void main_window::init_connections(hex_editor *editor, dynamic_scrollbar *scrollbar, panel_manager *panel)
{
	connect(scrollbar, &dynamic_scrollbar::valueChanged, editor, &hex_editor::slider_update);
	connect(editor, &hex_editor::update_slider, scrollbar, &dynamic_scrollbar::setValue);
	connect(editor, &hex_editor::update_range, scrollbar, &dynamic_scrollbar::set_range);
	connect(editor, &hex_editor::toggle_scroll_mode, scrollbar, &dynamic_scrollbar::toggle_mode);
	connect(editor, &hex_editor::update_status_text, statusbar, &QLabel::setText);
	connect(editor, &hex_editor::save_state_changed, this, &main_window::file_save_state);
	
	dialog_controller->connect_to_editor(editor);
	panel->connect_to_editor(editor);
	menu_controller->connect_to_widget(editor, EDITOR_EVENT);
	menu_controller->group_connect_to_widget(panel, PANEL_EVENT);
	
	editor->update_window();
}

void main_window::create_new_tab(QString name, bool new_file)
{
	QWidget *widget = new QWidget(this);
	QSize window_size = size();
	hex_editor *editor = new hex_editor(widget, name, undo_group, new_file);
	if(editor->load_error() != ""){
		QMessageBox::critical(this, "Invalid ROM", editor->load_error(), QMessageBox::Ok);
		delete editor;
		delete widget;
		return;
	}
	if (!new_file) {
		watch_file(name);
	}
	dynamic_scrollbar *scrollbar = new dynamic_scrollbar(editor);
	panel_manager *panel_controller = new panel_manager(editor);
	init_connections(editor, scrollbar, panel_controller);
	
	QHBoxLayout *hex_layout = new QHBoxLayout(widget);
	hex_layout->addWidget(editor);
	hex_layout->addWidget(scrollbar);
	hex_layout->addWidget(panel_controller);
	widget->setLayout(hex_layout);
	tab_widget->addTab(widget, QFileInfo(name).fileName());
	
	tab_widget->setCurrentWidget(widget);
	editor->set_focus();
	panel_controller->init_displays();
	resize(window_size);
}

hex_editor *main_window::get_editor(int i) const
{
	return dynamic_cast<hex_editor *>(tab_widget->widget(i)->layout()->itemAt(0)->widget());
}

int main_window::get_editor_index_by_filename(const QString& path) const {
	for (int i = 0; i < tab_widget->count(); i++) {
		if (tab_widget->tabText(i) == path)
			return i;
	}
	return -1;
}

main_window::~main_window()
{
	character_mapper::delete_active_map();
}

