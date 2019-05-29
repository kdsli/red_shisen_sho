#include "crecordsdialog.h"
#include "ui_crecordsdialog.h"

#include "csettings.h"
#include "crecordsmanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLocale>
#include <QHeaderView>

CRecordsDialog::CRecordsDialog(QWidget *parent) : QDialog(parent),
    ui(new Ui::CRecordsDialog),
    m_tabbar(new QTabBar(this)),
    m_table_widget(new QTableWidget(this))
{
    ui->setupUi(this);

    settings->RegisterGeometry(this);

    m_tabbar->setShape(QTabBar::RoundedWest);

    m_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table_widget->setColumnCount(4);
    m_table_widget->horizontalHeader()->setStretchLastSection(true);

    // Заполнить TabBar
    fillTabWidget();

    connect(m_tabbar, &QTabBar::currentChanged, this, &CRecordsDialog::slotTabChanged);

    // Layouts
    hlayout = new QHBoxLayout;
    hlayout->addWidget(m_tabbar);
    hlayout->addWidget(m_table_widget);
    hlayout->setStretch(1, 0);

    auto vlayout = new QVBoxLayout;
    vlayout->addLayout(hlayout);
    vlayout->addWidget(ui->buttonBox);
    setLayout(vlayout);
}

CRecordsDialog::~CRecordsDialog()
{
    delete ui;
}

void CRecordsDialog::Show(int result_index)
{
    auto current_game_index = settings->currentGameIndex();

    m_tabbar->setCurrentIndex(current_game_index);
    filltableWidget(current_game_index, result_index);

    exec();
}

// Заполнить зкаладки с названиями типов игры
void CRecordsDialog::fillTabWidget()
{
    m_tabbar->setExpanding(false);

    for (auto const &game : settings->gamesName()) {
        m_tabbar->addTab(game);
    }
}

// Заполнить таблицу данными конкретной игры
void CRecordsDialog::filltableWidget(int game_index, int result_index)
{
    auto records_list = records_manager->gameRecords(settings->games()[game_index].first);

    m_table_widget->clear();
    m_table_widget->setRowCount(records_manager->maxRecord());
    m_table_widget->setHorizontalHeaderLabels(QStringList{tr("Дата"), tr("Время"), tr("Гравитация"), tr("Имя")});
    m_table_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto n = 0;
    for (const auto &record : records_list) {
        addTableItem(n, 0, record.date.toString(QLocale::system().dateFormat(QLocale::ShortFormat)), result_index);

        QTime t(0, 0, 0);
        t = t.addSecs(record.time);
        addTableItem(n, 1, t.toString("hh:mm:ss"), result_index);

        addTableItem(n, 2, record.is_gravity ? tr("да") : tr("нет"), result_index);
        addTableItem(n, 3, record.name, result_index);

        ++n;
    }

    for (auto i = n; i < records_manager->maxRecord(); ++i) {
        for (int j = 0; j < 4; ++j) {
            addTableItem(i, j, "-");
        }
    }
}

void CRecordsDialog::addTableItem(int row, int column, const QString &text, int result_index)
{
    auto item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    if (row == result_index)
        item->setBackground(QBrush("#9cfff4"));
    m_table_widget->setItem(row, column, item);
}

// Изменилась закладка
void CRecordsDialog::slotTabChanged(int index)
{
    filltableWidget(index);
}
