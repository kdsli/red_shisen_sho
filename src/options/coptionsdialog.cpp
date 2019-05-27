#include "coptionsdialog.h"
#include "ui_coptionsdialog.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"

#include <QDir>

COptionsDialog::COptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::COptionsDialog),
    m_game_type_slider(nullptr),
    m_time_delay_slider(nullptr)
{
    ui->setupUi(this);

    settings->RegisterGeometry(this);

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &COptionsDialog::slotAccepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &COptionsDialog::reject);

    connect(ui->lwBackgrounds, &QListWidget::currentRowChanged, this, &COptionsDialog::slotBGRowChange);
    connect(ui->lwTiles, &QListWidget::currentRowChanged, this, &COptionsDialog::slotTilesRowChange);

    QStringList list;
    list << tr("медленно") << tr("нормально") << tr("быстро");

    m_game_type_slider = newSlider(settings->fieldTypeNames(), ui->gbField);
    m_time_delay_slider = newSlider(list, ui->gbDelay);

    // Заполним список языков
    fillLocales();

    // Настроим закладку костяшек
    initTilesTab();

    // Настроим закладку фона
    initBackgroundTab();
}

COptionsDialog::~COptionsDialog()
{
    delete ui;
}

CLabeledSlider *COptionsDialog::newSlider(const QStringList &labels, QGroupBox *group_box)
{
    auto slider = new CLabeledSlider(ui->gbField, labels);
    auto *layout = new QVBoxLayout;
    layout->addWidget(slider);
    group_box->setLayout(layout);

    return slider;
}

void COptionsDialog::Show()
{
    // Заполним элементы общей закладки
    ui->ckGravity->setCheckState(settings->isGravity() ? Qt::Checked : Qt::Unchecked);
    ui->ckDecision->setCheckState(settings->isDecision() ? Qt::Checked : Qt::Unchecked);
    ui->ckTraining->setCheckState(settings->isTraining() ? Qt::Checked : Qt::Unchecked);
    m_game_type_slider->setValue(settings->fieldType());
    m_time_delay_slider->setValue(settings->timerDelay());

    ui->tabWidget->setCurrentIndex(0);

    // Установим текущий индекс списка фона
    ui->lwTiles->setCurrentRow(tiles_manager->currentIndex());
    ui->lwBackgrounds->setCurrentRow(bg_manager->currentIndex());

    exec();
}

void COptionsDialog::slotAccepted()
{
    bool is_new_game(false);

    bool state = ui->ckGravity->checkState() == Qt::Checked;
    if (state != settings->isGravity()) {
        settings->setGravity(state);
        is_new_game = true;
    }
    state = ui->ckDecision->checkState() == Qt::Checked;
    if (state != settings->isDecision()) {
        settings->setDecision(state);
        is_new_game = true;
    }
    if (m_game_type_slider->value() != settings->fieldType()) {
        settings->setFieldType(static_cast<CSettings::FieldType>(m_game_type_slider->value()));
        is_new_game = true;
    }

    if (m_time_delay_slider->value() != settings->timerDelay()) {
        settings->setTimerDelay(m_time_delay_slider->value());
    }

    state = ui->ckTraining->checkState() == Qt::Checked;
    if (state != settings->isTraining()) {
        settings->setTraining(state);
        is_new_game = true;
        emit signalTrainingChange();
    }

    if (ui->cbLanguages->currentText() != settings->currentLanguage()) {
        settings->setCurrentLanguage(ui->cbLanguages->currentText());
        emit signalLanguageChange();
    }

    if (ui->lwTiles->currentRow() != tiles_manager->currentIndex()) {
        settings->setTileset(tiles_manager->tilesList()[ui->lwTiles->currentRow()].file_name);
        is_new_game = true;
        emit signalTilesChange();
    }

    if (ui->lwBackgrounds->currentRow() != bg_manager->currentIndex()) {
        settings->setBackground(bg_manager->bgList()[ui->lwBackgrounds->currentRow()].file_name);
        emit signalBackgroundChange();
    }

    if (is_new_game) emit signalRequiredNewGame();

    close();
}

// Изменился индекс в списке костяшек
void COptionsDialog::slotTilesRowChange(int index)
{
    if (index == -1) return;
    const auto &record = tiles_manager->tilesList()[index];
    ui->laFile_Tiles->setText(record.file_name);
    ui->laAuthor_Tiles->setText(record.author);
    ui->laContact_Tiles->setText(record.authorEmail);
    ui->laDescription_Tiles->setText(record.description);
    ui->laTiles->setPixmap(record.pixmap);
}

// Изменился индекс в списке фонов
void COptionsDialog::slotBGRowChange(int index)
{
    if (index == -1) return;
    const auto &record = bg_manager->bgList()[index];
    ui->laFile_BG->setText(record.file_name);
    ui->laAuthor_BG->setText(record.author);
    ui->laContact_BG->setText(record.authorEmail);
    ui->laDescription_BG->setText(record.description);
    ui->laBG->setPixmap(record.pixmap);
}

// Заполним закладку костяшек
void COptionsDialog::initTilesTab()
{
    for (auto const &record : tiles_manager->tilesList()) {
        auto text = record.description;
        if (text.isEmpty()) text = record.file_name;
        ui->lwTiles->addItem(text);
    }
}

// Заполним закладку фона
void COptionsDialog::initBackgroundTab()
{
    for (auto const &record : bg_manager->bgList()) {
        auto text = record.description;
        if (text.isEmpty()) text = record.file_name;
        ui->lwBackgrounds->addItem(text);
    }
}

void COptionsDialog::fillLocales()
{
    auto languages = settings->listLanguages();
    ui->cbLanguages->addItems(languages.values());
    ui->cbLanguages->setCurrentText(settings->currentLanguage());
}
