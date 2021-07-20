#include "diff_panel.h"
#include "../hex_editor.h"
#include "../main_window.h"
#include "../utility.h"
#include <QPushButton>
#include <QDebug>
#include <QSizePolicy>

diff_panel::diff_panel(QWidget *parent) : QWidget(parent)
{
	QBoxLayout* layout = new QBoxLayout(QBoxLayout::Direction::Down, this);
	setLayout(layout);
}

diff_panel* diff_panel::diff_show(main_window* window, QTableWidget* table, hex_editor* editor, QWidget* parent) {
	diff_panel* panel = new diff_panel{parent};
	panel->diff_show(window, table, editor);
	editor->set_diff_panel(panel);
	return panel;
}

void diff_panel::toggle_display(bool state) {
	setVisible(state);
	propagate_resize(this);
}

void diff_panel::diff_show(main_window* window, QTableWidget* table, hex_editor* editor) {
	this->table = table;
	QFont font{};
	font.setPointSize(12);

	QPushButton* butn_accept_all_inc = new QPushButton(this);
	butn_accept_all_inc->setFont(font);
	butn_accept_all_inc->setText("Accept all incoming");
	butn_accept_all_inc->adjustSize();
	auto preferredWidth = butn_accept_all_inc->width() * 1.1;
	auto preferredHeight = butn_accept_all_inc->height() * 1.25;
	butn_accept_all_inc->setFixedSize(preferredWidth, preferredHeight);

	QPushButton* butn_accept_all_curr = new QPushButton(this);
	butn_accept_all_curr->setFont(font);
	butn_accept_all_curr->setText("Accept all current");
	butn_accept_all_curr->setFixedSize(preferredWidth, preferredHeight);

	QPushButton* butn_accept_inc = new QPushButton(this);
	butn_accept_inc->setFont(font);
	butn_accept_inc->setText("Accept incoming");
	butn_accept_inc->setFixedSize(preferredWidth, preferredHeight);

	QPushButton* butn_accept_curr = new QPushButton(this);
	butn_accept_curr->setFont(font);
	butn_accept_curr->setText("Accept current");
	butn_accept_curr->setFixedSize(preferredWidth, preferredHeight);

	connect(butn_accept_curr, &QPushButton::clicked, this, [window, editor, table, this]() {
		editor->accept_current_diff();
		table->removeRow(table->currentIndex().row());
		if (editor->get_diff()->empty()) {
			window->unwatch_file(editor->get_comparing_full_path());
			editor->save_compared();
			window->watch_file(editor->get_comparing_full_path());
			editor->close_compare();
			emit done_editing();
		}
	});
	connect(butn_accept_inc, &QPushButton::clicked, this, [window, editor, table, this]() {
		editor->accept_incoming_diff();
		table->removeRow(table->currentIndex().row());
		if (editor->get_diff()->empty()) {
			window->unwatch_file(editor->get_comparing_full_path());
			editor->save_compared();
			window->watch_file(editor->get_comparing_full_path());
			editor->close_compare();
			emit done_editing();
		}
	});
	connect(butn_accept_all_curr, &QPushButton::clicked, this, [window, editor, this]() {
		editor->accept_all_current_diffs();
		window->unwatch_file(editor->get_comparing_full_path());
		editor->save_compared();
		window->watch_file(editor->get_comparing_full_path());
		editor->close_compare();
		emit done_editing();
	});
	connect(butn_accept_all_inc, &QPushButton::clicked, this, [window, editor, this]() {
		editor->accept_all_incoming_diffs();
		window->unwatch_file(editor->get_comparing_full_path());
		editor->save_compared();
		window->watch_file(editor->get_comparing_full_path());
		emit done_editing();
	});
	QWidget* grid_widget = new QWidget(this);
	QGridLayout* grid = new QGridLayout(grid_widget);
	grid->addWidget(butn_accept_inc, 0, 0);
	grid->addWidget(butn_accept_curr, 0, 1);
	grid->addWidget(butn_accept_all_inc, 1, 0);
	grid->addWidget(butn_accept_all_curr, 1, 1);
	grid_widget->setLayout(grid);
	layout()->addWidget(grid_widget);
}
