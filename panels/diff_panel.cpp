#include "diff_panel.h"
#include "../hex_editor.h"
#include "../main_window.h"
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QDebug>

diff_panel::diff_panel(QWidget *parent) : QDialog(parent)
{
	QBoxLayout* layout = new QBoxLayout(QBoxLayout::Direction::Down, this);
	setLayout(layout);
}

void diff_panel::diff_show(hex_editor* editor, const QString& path, QWidget* parent) {
	diff_panel panel{parent};
	panel.setWindowTitle("Diff view");
	panel.diff_show(editor, path);
}

void diff_panel::diff_show(hex_editor* editor, const QString& path) {
	QRect editor_geometry = editor->geometry();
	QPushButton* butn_next = new QPushButton(this);
	butn_next->setText("Next difference");
	butn_next->adjustSize();
	QPushButton* butn_prev = new QPushButton(this);
	butn_prev->setText("Previous difference");
	butn_prev->adjustSize();
	QPushButton* butn_accept_inc = new QPushButton(this);
	butn_accept_inc->setText("Accept incoming difference");
	butn_accept_inc->adjustSize();
	QPushButton* butn_accept_curr = new QPushButton(this);
	butn_accept_curr->setText("Accept current difference");
	butn_accept_curr->adjustSize();
	QPushButton* butn_accept_all_inc = new QPushButton(this);
	butn_accept_all_inc->setText("Accept all incoming differences");
	butn_accept_all_inc->adjustSize();
	QPushButton* butn_accept_all_curr = new QPushButton(this);
	butn_accept_all_curr->setText("Accept all current differences");
	butn_accept_all_curr->adjustSize();

	connect(butn_next, &QPushButton::clicked, this, [editor]() {
		editor->goto_diff(true);
	});
	connect(butn_prev, &QPushButton::clicked, this, [editor]() {
		editor->goto_diff(false);
	});
	connect(butn_accept_curr, &QPushButton::clicked, this, [editor, this]() {
		editor->accept_current_diff();
		if (editor->get_diff()->empty()) {
			((main_window*)parent())->unwatch_file(editor->get_comparing_full_path());
			editor->save_compared();
			((main_window*)parent())->watch_file(editor->get_comparing_full_path());
			this->done(0);
		}
	});
	connect(butn_accept_inc, &QPushButton::clicked, this, [editor, this]() {
		editor->accept_incoming_diff();
		if (editor->get_diff()->empty()) {
			((main_window*)parent())->unwatch_file(editor->get_comparing_full_path());
			editor->save_compared();
			((main_window*)parent())->watch_file(editor->get_comparing_full_path());
			this->done(0);
		}
	});
	connect(butn_accept_all_curr, &QPushButton::clicked, this, [editor, this]() {
		editor->accept_all_current_diffs();
		((main_window*)parent())->unwatch_file(editor->get_comparing_full_path());
		editor->save_compared();
		((main_window*)parent())->watch_file(editor->get_comparing_full_path());
		this->done(0);
	});
	connect(butn_accept_all_inc, &QPushButton::clicked, this, [editor, this]() {
		editor->accept_all_incoming_diffs();
		((main_window*)parent())->unwatch_file(editor->get_comparing_full_path());
		editor->save_compared();
		((main_window*)parent())->watch_file(editor->get_comparing_full_path());
		this->done(0);
	});

	QHBoxLayout* hbox_top = new QHBoxLayout(this);
	QHBoxLayout* hbox_bot = new QHBoxLayout(this);
	layout()->addWidget(editor);
	hbox_top->addWidget(butn_accept_inc);
	hbox_top->addWidget(butn_accept_curr);
	hbox_top->addWidget(butn_accept_all_inc);
	hbox_top->addWidget(butn_accept_all_curr);
	hbox_bot->addWidget(butn_next);
	hbox_bot->addWidget(butn_prev);
	layout()->addItem(hbox_top);
	layout()->addItem(hbox_bot);
	resize(editor_geometry.width(), editor_geometry.height());
	bool was_comparing = editor->is_comparing();
	QString old_compare{};
	if (editor->is_comparing()) {
		old_compare = editor->get_comparing_full_path();
		editor->close_compare();
	}
	editor->compare(path);
	editor->set_focus();
	exec();
	editor->close_compare();
	if (was_comparing) {
		editor->compare(old_compare);
	}
	layout()->removeWidget(editor);
	editor->setParent(nullptr);
}
