#ifndef DIFF_PANEL_H
#define DIFF_PANEL_H

#include <QWidget>
#include <QTableWidget>

class hex_editor;
class main_window;

class diff_panel : public QWidget
{
	Q_OBJECT
public:
	static diff_panel* diff_show(main_window* window, QTableWidget* table, hex_editor* editor, QWidget* parent);
	void toggle_display(bool state);
	QTableWidget* get_table() { return table; }
signals:
	void done_editing();
private:
	diff_panel(QWidget *parent);
	void diff_show(main_window* window, QTableWidget* table, hex_editor* editor);

	QTableWidget* table = nullptr;
};

#endif // DIFF_PANEL_H
