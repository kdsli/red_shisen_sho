#include "crecordsdialog.h"
#include "ui_crecordsdialog.h"

#include "csettings.h"
#include "crecordsmanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLocale>
#include <QHeaderView>
#include <QMessageBox>

CRecordsDialog::CRecordsDialog(QWidget *parent) : QDialog(parent),
    ui(new Ui::CRecordsDialog),
    m_tabbar(new QTabBar(this)),
    m_table_widget(new QTableWidget(this)),
    m_result_index(-1)
{
    ui->setupUi(this);

    settings->registerGeometry(this);

    m_tabbar->setShape(QTabBar::RoundedWest);

    m_table_widget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table_widget->setColumnCount(4);
    m_table_widget->horizontalHeader()->setStretchLastSection(true);

    // Заполнить TabBar
    fillTabWidget();

    connect(m_tabbar, &QTabBar::currentChanged, this, &CRecordsDialog::slotTabChanged);

    // Layouts
    auto layout_tab = new QHBoxLayout;
    layout_tab->addWidget(m_tabbar);
    layout_tab->addWidget(m_table_widget);
    layout_tab->setStretch(1, 0);

    auto layout_button = new QHBoxLayout;
    layout_button->addWidget(ui->buClear);
    layout_button->addStretch();
    layout_button->addWidget(ui->buttonBox);

    auto main_layout = new QVBoxLayout;
    main_layout->addLayout(layout_tab);
    main_layout->addLayout(layout_button);
    setLayout(main_layout);

    connect(ui->buClear, &QPushButton::clicked, this, &CRecordsDialog::slotClear);
}

CRecordsDialog::~CRecordsDialog()
{
    delete ui;
}

void CRecordsDialog::Show(int result_index)
{
    m_result_index = result_index;
    m_current_game_index = settings->currentGameIndex();

    m_tabbar->setCurrentIndex(m_current_game_index);
    fillTableWidget();

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
void CRecordsDialog::fillTableWidget()
{
    auto &records_list = records_manager->gameRecords(settings->games()[m_tabbar->currentIndex()].first);

    m_table_widget->clear();
    m_table_widget->setRowCount(records_manager->maxRecord());
    m_table_widget->setHorizontalHeaderLabels(QStringList{tr("Дата"), tr("Время"), tr("Гравитация"), tr("Имя")});
    m_table_widget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    auto n = 0;
    for (const auto &record : records_list) {
        addTableItem(n, 0, record.date.toString(QLocale::system().dateFormat(QLocale::ShortFormat)));

        QTime t(0, 0, 0);
        t = t.addSecs(record.time);
        addTableItem(n, 1, t.toString("hh:mm:ss"));

        addTableItem(n, 2, record.is_gravity ? tr("да") : tr("нет"));
        addTableItem(n, 3, record.name);

        ++n;
    }

    for (auto i = n; i < records_manager->maxRecord(); ++i) {
        for (int j = 0; j < 4; ++j) {
            addTableItem(i, j, "-");
        }
    }
}

void CRecordsDialog::addTableItem(int row, int column, const QString &text)
{
    auto item = new QTableWidgetItem(text);
    item->setTextAlignment(Qt::AlignCenter);
    if (row == m_result_index and m_tabbar->currentIndex() == m_current_game_index)
        item->setBackground(QBrush("#9cfff4"));
    m_table_widget->setItem(row, column, item);
}

// Изменилась закладка
void CRecordsDialog::slotTabChanged(int)
{
    fillTableWidget();
}

// Очистить
void CRecordsDialog::slotClear()
{
    auto index = m_tabbar->currentIndex();
    if (QMessageBox::question(this, tr("Внимание"), tr("Вы действительно хотите очистить рекорды текушего типа ")
                              + settings->gamesName()[index] + "?",
                              QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes) {
        records_manager->clear();
        fillTableWidget();
    }
}
