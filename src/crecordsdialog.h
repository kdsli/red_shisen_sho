#ifndef CRECORDSDIALOG_H
#define CRECORDSDIALOG_H

#include <QDialog>
#include <QTabBar>
#include <QTableWidget>
#include <QHBoxLayout>

namespace Ui {
class CRecordsDialog;
}

class CRecordsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CRecordsDialog(QWidget *parent = nullptr);
    ~CRecordsDialog() override;

    void Show(int result_index);

private:
    Ui::CRecordsDialog *ui;
    QTabBar *m_tabbar;
    QTableWidget *m_table_widget;

    QHBoxLayout *hlayout;

    void fillTabWidget();
    void filltableWidget(int game_index, int result_index = -1);
    void addTableItem(int row, int column, const QString &text, int result_index = -1);

private slots:
    void slotTabChanged(int index);

};

#endif // CRECORDSDIALOG_H
