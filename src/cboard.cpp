#include "cboard.h"

#include "csettings.h"
#include "common_types.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"
#include "crecordsmanager.h"

#include <QResizeEvent>
#include <QFileInfo>

CBoard::CBoard(CScene *scene, CField *field, QWidget *parent) : QGraphicsView(parent),
    m_field(field),
    m_scene(scene)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);

    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_field, &CField::signalStatusUndoRedo, this, &CBoard::signalUndoRedo);
    connect(m_field, &CField::signalStartConnect, this, &CBoard::slotStartConnect);

    // Очень важный параметр, см. комментарий к процедуре drawBackground
    setCacheMode(QGraphicsView::CacheBackground);

    setBackground(bg_manager->currentFile());

    setScene(m_scene);
}

// ------------------------------------------------------------------------------------------------
// Сбросить все параметры игры (начать новую игру)
void CBoard::newGame()
{
    m_path_state = CBoard::psNone;
    m_is_cunning = false;
}

// ------------------------------------------------------------------------------------------------
void CBoard::slotHint()
{
    if (m_game_state != gsGame) return;
    showHint();
    m_is_cunning = true;
}

// ------------------------------------------------------------------------------------------------
// Установить новый фон
void CBoard::slotSetBackground()
{
    setBackground(bg_manager->currentFile());
}

// ------------------------------------------------------------------------------------------------
void CBoard::resizeEvent(QResizeEvent *)
{
    recalcView();
}

// ------------------------------------------------------------------------------------------------
// Нажатие на кнопки мыши
void CBoard::mousePressEvent(QMouseEvent *event)
{
     if (event->buttons().testFlag(Qt::LeftButton))
         clickLeftButton(event);
     if (event->buttons().testFlag(Qt::RightButton) && settings->isTraining())
         clickRightButton(event);
}

// ------------------------------------------------------------------------------------------------
void CBoard::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_path_timer) {
        doFinishPath();
    }
    if (event->timerId() == m_demostration_timer) {
        doDemonstration();
    }
}

// ------------------------------------------------------------------------------------------------
// Нарисовать фон
// Так как установлено кеширование фона, эта функция вызывается только один раз
// после вызова setBackgroundBrush, далее QT сам рисует фон
void CBoard::drawBackground(QPainter *painter, const QRectF &rect)
{
    painter->drawPixmap(rect, m_bg_pixmap, m_bg_pixmap.rect());
}

// ------------------------------------------------------------------------------------------------
// Все что поверх фона
void CBoard::drawForeground(QPainter *painter, const QRectF &)
{
    if (!m_message.isEmpty())
        paintMessage(painter);
    if (m_path_state != psNone)
        paintPath(painter);
}

// ------------------------------------------------------------------------------------------------
// Установить изображение фона из файла
void CBoard::setBackground(const QString &file_name)
{
    if (!QFileInfo::exists(file_name)) {
        settings->setDefaultBackground();
    }

    if (!QFileInfo::exists(file_name)) return;

    QSize size;

    if (bg_manager->isCurrentSvg()) {
        // SVG item
        QSvgRenderer bg_renderer(file_name);
        QGraphicsSvgItem m_bg_svg;
        m_bg_svg.setSharedRenderer(&bg_renderer);
        size = m_bg_svg.boundingRect().size().toSize();
        // Рисуем на Image
        QImage img(size, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&img);
        bg_renderer.render(&painter);
        // И сохраним как зависимый Pixmap
        m_bg_pixmap = QPixmap::fromImage(img);
    } else {
        // Image item
        QImage img(file_name);
        m_bg_pixmap = QPixmap::fromImage(img);
    }

    setBackgroundBrush(m_bg_pixmap);
}

// ------------------------------------------------------------------------------------------------
// Пересчитать view (вызвается при инициализации и каждом изменении размера)
void CBoard::recalcView()
{
    fitInView(m_scene->m_viewport_rect, Qt::KeepAspectRatio);

    // Рассчитаем прямоугольник сообщения
    m_message_rect = QRectF(0, 0, m_scene->m_field_rect.width() * 0.7, m_scene->m_field_rect.height() * 0.7);
    // Центрируем прямоугольник
    m_message_rect.moveCenter(m_scene->m_field_rect.center());
    // Размер шрифта сообщения в пискелах. Думаю, процентов 7 от высоты
    m_message_font_pixel = static_cast<int>(m_message_rect.height() * 0.07);

}

// ------------------------------------------------------------------------------------------------
void CBoard::clickLeftButton(QMouseEvent *event)
{
    // Если перерисовывается путь - мышь недоступна
    if (m_path_state == psPath || m_path_state == psHint) return;

    switch (m_game_state) {
    case gsGame:
        m_scene->mouseLeft(mapToScene(event->pos()));
        break;
    case gsPause:
    case gsDemostration:
    case gsNotVariants:
    case gsVictory:
        emit signalClick();
        break;
    case gsEmpty:
        break;
    }
}

// ------------------------------------------------------------------------------------------------
void CBoard::clickRightButton(QMouseEvent *event)
{
    // Отдадим нажатие сцене
    if (settings->isTraining()) m_scene->mouseRight(mapToScene(event->pos()));
}

// ------------------------------------------------------------------------------------------------
void CBoard::slotUndo()
{
    if (m_game_state != gsGame) return;
    m_is_cunning = true;
    m_scene->slotUndo();
}

// ------------------------------------------------------------------------------------------------
void CBoard::slotRedo()
{
    if (m_game_state != gsGame) return;
    m_is_cunning = true;
    m_scene->slotRedo();
}

// ------------------------------------------------------------------------------------------------
// Нарисовать путь
void CBoard::paintPath(QPainter *painter)
{
    QPen pen(Qt::red);
    pen.setWidth(tiles_manager->pathLineSize());
    pen.setCapStyle(Qt::RoundCap);
    painter->setPen(pen);

    // Выводим в координатах сцены
    for (int i = 0; i < m_scene->m_path_coords.size() - 1; ++i) {
        painter->drawLine(m_scene->m_path_coords[i], m_scene->m_path_coords[i + 1]);
    }
}

// ------------------------------------------------------------------------------------------------
// Готовим регион отображения пути. Вообще-то не обязательно, ибо двойная буферизация все равно не
// даст перерисовывать не измененные участки, но учет clip region даст возможный небольшой выигрыш
// Ну и про перфекционизм не стоит забывать...
void CBoard::repaintPath()
{
    Q_ASSERT_X(!m_scene->m_path_coords.isEmpty(), "CBoard::repaintPath", "m_scene->m_path_coords is not empty");

    QRegion region;

    // Для горизонтальных и вертикальных линий толщина будет нулевой. Вот на эту величину и будем увеличивать
    auto delta = tiles_manager->pathLineSize();

    // Регион в абсолютных координатах виджета
    auto prev_coord = mapFromScene(m_scene->m_path_coords[0]);
    for (int i = 1; i < m_scene->m_path_coords.size(); ++i) {
        auto coord = mapFromScene(m_scene->m_path_coords[i]);
        auto rect = QRectF(prev_coord, coord).normalized();
        rect.adjust(-delta, -delta, delta, delta);
        region += rect.toRect();
        prev_coord = coord;
    }
    viewport()->update(region);
}

// ------------------------------------------------------------------------------------------------
// Запустить таймер рисования пути
void CBoard::doStartPath(const TilePair &tiles)
{
    // Пусть сцена заполнит координаты своего пути
    m_scene->setPathCoords();

    repaintPath();

    // Сохраним костяшки и запустим таймер
    m_scene->m_deleted_tiles = tiles;
    m_path_timer = startTimer(settings->timeDelay());
}

// ------------------------------------------------------------------------------------------------
// Окончание таймера пути
void CBoard::doFinishPath()
{
    m_scene->m_selected.clear();

    Q_ASSERT(m_path_timer != -1);
    killTimer(m_path_timer);
    m_path_timer = -1;

    // Для окончания снятия костяшек
    if (m_path_state == psPath) {

        // Удалить костяшки
        m_scene->removeTiles(m_scene->m_deleted_tiles);

        repaintPath();

        m_field->m_path.clear();

        // Проверим состояние программы
        auto status = m_field->getGameStatus();
        if (status == vsVictory)
            emit signalVictory();
        if (status == vsNotVariant)
            emit signalNotVariant();

        m_path_state = psNone;
    }

    // Для подсказки
    if (m_path_state == psHint) {
        m_path_state = psNone;
        repaintPath();
    }

    // Статусная строка
    emit signalUpdateInfo();
}

// ------------------------------------------------------------------------------------------------
// Показать hint
void CBoard::showHint()
{
    if (m_path_state != psNone) return;

    m_scene->clearSelected();
    // В m_scene.path() лежит текущая подсказка
    m_path_state = psHint;

    doStartPath(m_field->m_hint_tiles);
}

// ------------------------------------------------------------------------------------------------
// Начать демонстрацию
void CBoard::startDemonstration()
{
    hideMessage(false);

    // Вернуть на место поля и сцену
    m_field->restoreField();
    m_scene->fillScene();
    m_scene->m_path_coords.clear();

    if (!settings->isDecision()) return;

    m_path_state = psDemostration;

    // Текущий номер демонстрации - индекс в массиве m_field->m_right_pathes
    m_demostration_index = 0;

    m_demostration_timer = startTimer(settings->timeDelay());

}

// ------------------------------------------------------------------------------------------------
// Нарисовать окно сообщений
void CBoard::paintMessage(QPainter *painter) const
{
    QPen pen(Qt::black);
    pen.setWidth(2);
    pen.setColor(Qt::white);
    painter->setPen(pen);
    painter->setBrush(QColor::fromRgb(50, 50, 50, 150));

    painter->drawRoundedRect(m_message_rect, 20, 20);

    QFont font;
    font.setPixelSize(m_message_font_pixel);
    painter->setFont(font);

    QTextOption to;
    to.setAlignment(Qt::AlignCenter);

    painter->drawText(m_message_rect, m_message, to);
}

// ------------------------------------------------------------------------------------------------
void CBoard::showMessage(const QString &message, bool is_hide_tiles)
{
    // Скрыть все плитки
    if (is_hide_tiles) m_scene->setTilesVisible(false);
    m_message = message;
    viewport()->update();
}

// ------------------------------------------------------------------------------------------------
void CBoard::hideMessage(bool is_show_tiles)
{
    m_scene->setTilesVisible(is_show_tiles);
    m_message.clear();
    viewport()->update();
}

// ------------------------------------------------------------------------------------------------
// Демонстрация (по таймеру)
void CBoard::doDemonstration()
{
    // Старый путь
    if (m_demostration_index > 0) {
        m_field->m_path.clear();
        m_scene->m_path_coords = m_old_path_coords;
        repaintPath();
        // Снимем ячейки
        clearDemostrationTiles(m_field->m_right_pathes[m_demostration_index-1]);
    }

    // Проверка окончания
    if (m_demostration_index == m_field->m_right_pathes.size()) {
        closeDemonstration();
        return;
    }

    // Новый путь
    auto tiles = m_field->m_right_pathes[m_demostration_index];
    // Находим его
    m_field->checkConnect(m_field->m_tiles, tiles);
    // Формируем путь из m_field->m_path
    m_scene->setPathCoords();
    // Старые координаты в принципе не нужно сохранять, но так для верности
    m_old_path_coords = m_scene->m_path_coords;

    repaintPath();

    ++m_demostration_index;
}

// ------------------------------------------------------------------------------------------------
// Снять костяшки для демонстрации
void CBoard::clearDemostrationTiles(const TilePair &tiles)
{
    // Снимем в поле
    m_field->removeTiles(m_field->m_tiles, tiles);
    // Снимем в сцене
    m_scene->removeTiles(tiles);
}

// ------------------------------------------------------------------------------------------------
// Завершаем демонстрцию
void CBoard::closeDemonstration()
{
    Q_ASSERT(m_demostration_timer != -1);
    killTimer(m_demostration_timer);
    m_demostration_timer = -1;

    m_scene->m_path_coords.clear();
    m_scene->clear();
    m_field->m_tiles.fill(-1);
    viewport()->update();

    emit signalClick();

    emit signalUndoRedo(false, false);
}

// ------------------------------------------------------------------------------------------------
// Поле сказало, что надо начинать удалять костяшки
void CBoard::slotStartConnect(const TilePair &tiles)
{
    m_path_state = psPath;

    // Выделить вторую костяшку
    m_scene->addSelected(tiles.second);

    // Стартуем показ пути
    doStartPath(tiles);
}

