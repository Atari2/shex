#ifndef DIFF_PANEL_H
#define DIFF_PANEL_H

#include <QDialog>

class hex_editor;

class diff_panel : public QDialog
{
	Q_OBJECT
public:
	static void diff_show(hex_editor* editor, const QString& path, QWidget* parent);
signals:

private:
	diff_panel(QWidget *parent);
	void diff_show(hex_editor* editor, const QString& path);
};

#endif // DIFF_PANEL_H
