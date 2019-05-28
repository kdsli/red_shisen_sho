#include "cboard.h"

#include "options/cbackgroundmanager.h"
#include "options/ctilesmanager.h"

#include <QSvgRenderer>
#include <QApplication>
#include <QGraphicsSvgItem>
#include <QResizeEvent>
#include <QPixmap>
#include <QImage>
#include <QPainter>
#include <QVector>
#include <QDir>
#include <QTime>
#include <QFileInfo>
#include <math.h>

// Поля в процентах (отступ)
static double margin = 5;

CBoard::CBoard(QWidget *parent) : QWidget(parent),
    m_bg_renderer(nullptr),
    m_bg_svg(nullptr)
{
    m_game_state = gsNormal;

    m_ts = new CTileSet();
    m_field = new CField(m_ts, this);

    connect(m_field, &CField::signalRepaint, this, &CBoard::slotRepaint);
    connect(m_field, &CField::signalStartConnect, this, &CBoard::slotStartConnect);
    connect(m_field, &CField::signalNotVariants, this, &CBoard::slotNotVariants);
    connect(m_field, &CField::signalVictory, this, &CBoard::slotVictory);
    connect(m_field, &CField::signalStatusUndoRedo, this, &CBoard::signalUndoRedo);

    connect(this, &CBoard::signalUndo, m_field, &CField::slotUndo);
    connect(this, &CBoard::signalRedo, m_field, &CField::slotRedo);

    // Инициализация менеджера фонов
    bg_manager = new CBackgroundManager(this);

    // Инициализация фонового изображения
    if (!initBackground()) return;

    // Стартуем таймер учета игрового времени
    m_second = 0;
    m_timer_game = startTimer(1000);

    emit signalUpdateInfo();
}

// ================================================================================================
// Сформировать информацию о состоянии игры
QString CBoard::gameInfo()
{
    QTime t(0, 0, 0);
    t = t.addSecs(m_second);

    return tr("Ваше время: ") + t.toString("hh:m:ss") + tr("  Удалено: ")
            + QString::number(m_field->tileCount() - m_field->currentCount()) + "/"
            + QString::number(m_field->tileCount());
}

// ================================================================================================
// Инициализация фона
bool CBoard::initBackground()
{
    QString file_name = bg_manager->currentFile();
    if (!QFileInfo::exists(file_name)) {
        emit signalBackgroundNotFound();
        return false;
    }

    QSize size;

    if (bg_manager->isCurrentSvg()) {
        // SVG item
        m_bg_renderer = new QSvgRenderer(file_name, this);
        m_bg_svg = new QGraphicsSvgItem;
        m_bg_svg->setSharedRenderer(m_bg_renderer);
        size = m_bg_svg->boundingRect().size().toSize();
        // Рисуем на Image
        QImage img(size, QImage::Format_ARGB32_Premultiplied);
        QPainter painter(&img);
        m_bg_renderer->render(&painter);
        // И сохраним как зависимый Pixmap
        m_bg_pixmap = QPixmap::fromImage(img);
    } else {
        // Image item
        QImage img(file_name);
        m_bg_pixmap = QPixmap::fromImage(img);
    }

    return true;
}

// ================================================================================================
// Новая игра
void CBoard::slotNewGame()
{
    doNewGame();
    update();

//    startDemonstration();
}

// ================================================================================================
// Игра с новым типом
void CBoard::slotNewTypeGame()
{
    doNewGame();
    recalcBoardSize(geometry().width(), geometry().height());
    update();
}

// ================================================================================================
// Общие действия для новой игры
void CBoard::doNewGame()
{
    if (m_game_state == gsDemostration)
        closeDemonstration();

    m_field->newGame(settings->fieldType());

    m_second = 0;
    m_game_state = gsNormal;
}

// ================================================================================================
// Повторить игру
void CBoard::slotRepeatGame()
{
    m_field->repeatGame();
    m_second = 0;
    m_game_state = gsNormal;
    update();
}

// ================================================================================================
// Показать подсказку
void CBoard::slotHelp()
{
    if (m_is_path) return;
    // Получим подсказку
    TilePair tiles;
    if (!m_field->checkVariants(tiles)) return;

    m_is_help = true;
    // Тоже самое как для начала соединения
    slotStartConnect(tiles);
}

// ================================================================================================
// Пауза
void CBoard::slotPause()
{
    if (m_game_state != gsPause) {
        m_old_game_state = m_game_state;
        m_game_state = gsPause;
    } else
        m_game_state = m_old_game_state;
    update();
}

// ================================================================================================
// Установить набор костяшек
void CBoard::slotSetTileset()
{
    m_ts->initTileSet();
    recalcBoardSize(geometry().width(), geometry().height());
    update();
}

// ================================================================================================
// Установить фон
void CBoard::slotSetBackground()
{
    initBackground();

    update();
}

// ================================================================================================
// Изменение размера
void CBoard::resizeEvent(QResizeEvent *event)
{
    recalcBoardSize(event->size().width(), event->size().height());
}

// ================================================================================================
// Провести расчет доски при изменении размера
void CBoard::recalcBoardSize(int width, int height)
{
    // Нужно рассчитать размер костяшки для данного типа и размера доски
    // и сформировать изображения для данного размера из SVG
    // Попытки сохранить изображения и масшабировать для разных размеров
    // приводят к очень плохому изображению, поэтому приходится формировать
    // изображения при каждом изменении размеров окна.

    // Область отображения - с обоих сторон оставить поля
    int x = static_cast<int>(width * margin / 100);
    int y = static_cast<int>(height * margin / 100);
    m_current_field_rect = QRect(x, y, width - 2 * x, height - 2 * y);

    // Вычислим размеры прямоугольника сообщений
    // процентов 60 по высоте и ширине
    m_message_rect.setWidth(width * 6 / 10);
    m_message_rect.setHeight(height * 6 / 10);
    m_message_rect.moveCenter(m_current_field_rect.center());

    // Размер шрифта сообщения в пискелах. Думаю, процентов 7 от высоты
    m_font_pixel = static_cast<int>(m_message_rect.height() / 100 * 7);

    // Определим нужную ширину и высоту костяшки
    double w = m_current_field_rect.width() / m_field->x();
    double h = m_current_field_rect.height() / m_field->y();
    double calc_ratio = w / h;

    // Если отношение вычисленной ширины костяшки к высоте больше отношения
    // оригинальной ширины костяшки к высоте, значит у нас поле вытянуто
    // по горизонтали (поля слева и справа), поэтому ratio считаем по
    // вертикали (костяшки по вертикали занимают всю высоту).
    // Аналогично наоборот
    // Заодно для каждого варианта вычислим левую верхнюю точку костяшек
    double ratio;
    auto base_ratio = m_ts->base_size().width() / m_ts->base_size().height();
    if (calc_ratio > base_ratio) {
        ratio = h / m_ts->tile_size().height();
        // Смещение слева
        double left = m_current_field_rect.left() +
                (m_current_field_rect.width() - m_ts->tile_size().width() * ratio * m_field->x()) / 2;
        m_left_top = QPoint(static_cast<int>(left), m_current_field_rect.top());
    } else {
        ratio = w / m_ts->tile_size().width();
        // Смещение сверху
        double top = m_current_field_rect.top() +
                (m_current_field_rect.height() - m_ts->tile_size().height() * ratio * m_field->y()) / 2;
        m_left_top = QPoint(m_current_field_rect.left(), static_cast<int>(top));
    }

    // Инициализация элементов
    m_ts->CalcSizes(ratio);

    // А теперь рассчитаем реальные координаты поля
    // Точка m_left_top - реальный верхний угол
    m_current_field_rect.setTopLeft(m_left_top);

    auto left = m_ts->scaled_tile_size().width() * m_field->x() + 10;
    m_current_field_rect.setRight(m_left_top.x() + static_cast<int>(left));

    auto bottom = m_ts->scaled_tile_size().height() * m_field->y() + 10;
    m_current_field_rect.setBottom(m_left_top.y() + static_cast<int>(bottom));
}

// ================================================================================================
void CBoard::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // Фон
    paintBackground(painter);

    // Без оптимизации для наглядности
    switch (m_game_state) {
    case gsNormal:
        paintTiles(painter, event);
        paintPath(painter);
        break;
    case gsNotVariants:
        paintTiles(painter, event);
        paintMessage(painter);
        break;
    case gsPause:
        paintMessage(painter);
        break;
    case gsVictory:
        paintMessage(painter);
        break;
    case gsDemostration:
        paintTiles(painter, event);
        paintPath(painter);
    }
}

// ================================================================================================
// Нарисовать фон
void CBoard::paintBackground(QPainter &painter) const
{
    painter.drawPixmap(rect(), m_bg_pixmap);
}

// ================================================================================================
// Нарисовать костяшки
void CBoard::paintTiles(QPainter &painter, QPaintEvent *event) const
{
    QPointF point(m_left_top);
    int n = 0;
    // Рисуем костяшки
    for (int i = 0; i < m_field->y(); ++i) {
        for (int j = 0; j < m_field->x(); ++j) {
            // Прямоугольник, занимаемый костяшкой
            QRectF tile_rect(point, m_ts->scaled_tile_size());
            // Рисуем только если попадает в clip region
            if (event->region().intersects(tile_rect.toRect())) {
                // Тип костяшки
                auto tile_type = m_field->tileType(n);
                // Только если она есть
                if (tile_type != -1) {
                    // Получим pixmap костяшки
                    auto &pix = m_ts->pixmaps()[tile_type];
                    // Выводим фон
                    auto is_selected = m_field->isSelected(j, i);
                    painter.drawPixmap(point, is_selected ? m_ts->base_sel_pixmap() : m_ts->base_pixmap());
                    // Выводим костяшку
                    painter.drawPixmap(point, pix);
                }
            }
            // Дальше
            point.setX(point.x() + m_ts->scaled_tile_size().width());
            ++n;
        }
        point.setX(m_left_top.x());
        point.setY(point.y() + m_ts->scaled_tile_size().height());
    }
}

// ================================================================================================
// Нарисовать путь
void CBoard::paintPath(QPainter &painter) const
{
    QPen pen(Qt::red);
    // Толщина линии в 10% от высоты костяшки
    pen.setWidth(static_cast<int>(m_ts->scaled_tile_size().height() / 10));
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);

    for (int i = 0; i < m_field->path().size() - 1; ++i) {
        auto point1 = getTileRect(m_field->path()[i]).center();
        auto point2 = getTileRect(m_field->path()[i+1]).center();
        // Шаманим с координатами, чтобы они не сильно вылезали за границу
        correctPoint(point1);
        correctPoint(point2);
        painter.drawLine(point1, point2);
    }
}

// ================================================================================================
// Вывести сообщение в прозрачном прямоугольнике
void CBoard::paintMessage(QPainter &painter) const
{
    QPen pen(Qt::black);
    pen.setWidth(1);
    pen.setColor(Qt::white);
    painter.setPen(pen);
    painter.setBrush(QColor::fromRgb(50, 50, 50, 150));
    painter.drawRoundedRect(m_message_rect, 5, 5);

    QFont font;
    font.setPixelSize(m_font_pixel);
    painter.setFont(font);

    QTextOption to;
    to.setAlignment(Qt::AlignCenter);

    painter.drawText(m_message_rect, getMessageText(), to);
}

// ================================================================================================
// Поле сказало, что нужно перерисовать некоторые элементы
void CBoard::slotRepaint()
{
    QRegion region;

    // Нужные ячейки
    for (const auto &point : m_field->repaintList()) {
        QRectF rect = getBaseRect(point);
        // Если это последняя строка или колонка - взять прямоугольник основы
        if (point.x() == m_field->x()-1 || point.y() == m_field->y()-1) {
        } else {
            auto r = QRectF(rect.right(), rect.top(),
                            m_ts->base_size().width() - m_ts->tile_size().width(),
                            m_ts->tile_size().height());
            region += r.toRect();
        }

        // Зацепим костяшки слева и сверху - иначе будет непрятный артефакт
        rect.adjust(-1, -1, 0, 0);
        region += rect.toRect();
    }

    repaint(region);
}

// ================================================================================================
// Поле сказало перерисовать путь
void CBoard::slotStartConnect(const TilePair &tiles)
{
    // Сформируем region для перерисовки и перерисуемся
    repaint(setPathRegion(m_field->path()));
    // Сохраним костяшки и запустим таймер
    m_deleted_tile = tiles;
    m_is_path = true;
    m_timer_path = startTimer(settings->timeDelay());
}

// ================================================================================================
// Нет вариантов
void CBoard::slotNotVariants()
{
    m_game_state = gsNotVariants;
    update();
}

// ================================================================================================
//
void CBoard::slotVictory()
{
    m_game_state = gsVictory;
    update();
}

// ================================================================================================
// Нажатие мыши
void CBoard::mousePressEvent(QMouseEvent *event)
{
    // Пока удаляются костяшки игнорируем мышь
    if (m_is_path) return;

    if (event->buttons().testFlag(Qt::LeftButton)) {
        // Если идет демонстрация
        if (m_game_state == gsDemostration) {
            closeDemonstration();
            event->accept();
            return;
        }
        if (m_game_state == gsNotVariants) {
            startDemonstration();
        } else {
            clickLeftButton(event);
        }
    }
    if (event->buttons().testFlag(Qt::RightButton) && settings->isTraining()) {
        clickRightButton(event);
    }
    event->accept();
}

// ================================================================================================
// Найти координаты в поле по координатам виджета
Tile CBoard::getTileIndex(QPointF point) const
{
    auto x = static_cast<int>(floor((point.x() - m_left_top.x()) / m_ts->scaled_tile_size().width()));
    auto y = static_cast<int>(floor((point.y() - m_left_top.y()) / m_ts->scaled_tile_size().height()));

    if (x < 0 || x >= m_field->x() || y < 0 || y >= m_field->y())
        return Tile(-1, -1);

    return Tile(x, y);
}

// ================================================================================================
// Вернуть прямоугольник, который занимает костяшка

// По координатам поля (только tile)
QRectF CBoard::getTileRect(Tile tile) const
{
    return QRectF(m_left_top.x() + tile.x() * m_ts->scaled_tile_size().width(),
                  m_left_top.y() + tile.y() * m_ts->scaled_tile_size().height(),
                  m_ts->scaled_tile_size().width(), m_ts->scaled_tile_size().height());
}

// По координатам поля (вместе с основой)
QRectF CBoard::getBaseRect(Tile tile) const
{
    return QRectF(m_left_top.x() + tile.x() * m_ts->scaled_tile_size().width(),
                  m_left_top.y() + tile.y() * m_ts->scaled_tile_size().height(),
                  m_ts->scaled_base_size().width(), m_ts->scaled_base_size().height());
}

// ================================================================================================
// Нажата левая кнопка мыши
void CBoard::clickLeftButton(QMouseEvent *event)
{
    if (m_game_state == gsNormal) {
        // Получим номер костяшки, которую нажали
        const auto point = getTileIndex(event->pos());
        // Отдаем нажатие полю
        m_field->mouseLeft(point);
    }
    if (m_game_state == gsPause) {
        m_game_state = gsNormal;
        update();
    }
}

// ================================================================================================
// Нажата правая кнопка мыши - покажем все такие же костяшки
void CBoard::clickRightButton(QMouseEvent *event)
{
    // Получим номер костяшки, которую нажали
    const auto point = getTileIndex(event->pos());
    // Отдаем нажатие полю
    m_field->mouseRight(point);
}

// ================================================================================================
// Сработал таймер
void CBoard::timerEvent(QTimerEvent *event)
{
    // Если это таймер пути
    if (event->timerId() == m_timer_path) {
        doTimerPath();
    }
    // Если это таймер игрового времени
    if (event->timerId() == m_timer_game) {
        doTimerGame();
    }
    // Если это таймер демонстрации
    if (event->timerId() == m_timer_demostration) {
        doTimerDemostration();
    }
}

// ================================================================================================
// Установить регион обновления для пути
QRegion CBoard::setPathRegion(const TileList &path)
{
    QRegion region;

    // По всем линиями пути
    for (int i = 0; i < path.size() - 1; ++i) {
        auto rect1 = getBaseRect(path[i]);
        auto rect2 = getBaseRect(path[i+1]);
        // Чтобы отрисовалась тень верхней костяшки - увеличим
        rect1.adjust(-1, -1, 0, 0);
        rect2.adjust(-1, -1, 0, 0);
        auto rect = rect1.united(rect2);
        region += rect.toRect();
    }

    return region;
}

// ================================================================================================
QString CBoard::getMessageText() const
{
    switch (m_game_state) {
    case gsPause: return QString(tr("Игра приостановлена.") + "\n" + tr("Щелкните для продолжения..."));
    case gsVictory: return tr("Вы выиграли!");
    case gsNotVariants: return getNotVariantText();
    default: Q_ASSERT_X(false, "CBoard::getMessageText", "Message in normal mode");
    }
    return QString();
}

// ================================================================================================
// Вернуть текст для варианта без вариантов
QString CBoard::getNotVariantText() const
{
    QString s = tr("Игра зашла в тупик.") + "\n";
    if (settings->isDecision())
        s += tr("А вариант был - посмотрите демонстрацию.") + "\n";
    return s + tr("Щелкните для продолжения...");
}

// ================================================================================================
// Таймер пути
void CBoard::doTimerPath()
{
    // Сформируем регион для пути (может быть пустой)
    auto region = setPathRegion(m_field->path());
    // Скажем полю, что завершилось удаление (только если не подсказка)
    if (!m_is_help) {
        m_field->cancelRemoveTiles(m_deleted_tile);
    } else {
        m_is_help = false;
    }

    m_field->clearPath();
    // Сохраненный регион перерисуем
    repaint(region);

    killTimer(m_timer_path);
    m_timer_path = -1;
    m_is_path = false;

    // Статусная строка
    emit signalUpdateInfo();
}

// ================================================================================================
// Таймер игры
void CBoard::doTimerGame()
{
    if (m_game_state == gsNormal) {
        ++m_second;
        emit signalUpdateInfo();
    }
}

// ================================================================================================
// Запустить демонстрацию
void CBoard::startDemonstration()
{
    // Только если установлен режим решаемых комбинаций
    if (!settings->isDecision()) return;

    m_game_state = gsDemostration;
    m_demostration_index = 0;

    m_field->repeatGame();
    update();

    emit signalUndoRedo(false, false);

    // Запустим таймер демонстрации
    m_timer_demostration = startTimer(settings->timeDelay());
}

// ================================================================================================
void CBoard::closeDemonstration()
{
    m_game_state = gsNormal;
    killTimer(m_timer_demostration);
    m_timer_demostration = -1;
    m_field->clearGame();
    update();
}

// ================================================================================================
// Шаманство с координатами линий, чтобы не сильно уходили за границу поля
void CBoard::correctPoint(QPointF &point) const
{
    // Примерно треть костяшки по ширине
    auto offset = m_ts->scaled_tile_size().width() / 3;
    if (point.x() < m_current_field_rect.left())
        point.setX(m_current_field_rect.left() - offset);
    if (point.x() > m_current_field_rect.right())
        point.setX(m_current_field_rect.right() + offset);
    if (point.y() < m_current_field_rect.top())
        point.setY(m_current_field_rect.top() - offset);
    if (point.y() > m_current_field_rect.bottom())
        point.setY(m_current_field_rect.bottom() + offset);
}

// ================================================================================================
// Обработка таймера демонстрации
void CBoard::doTimerDemostration()
{
    if (m_game_state != gsDemostration) return;

    QRegion region;

    // В регион обновления добавить старый регион
    if (m_demostration_index > 0) {
        // Старый путь добавим в регион
        region = setPathRegion(m_old_path);
        // Снимем костяшки
        m_field->clearDemostrationTiles(m_demostration_index - 1);
    }

    // Проверка на выход
    if (m_demostration_index == m_field->tileCount() / 2) {
        m_field->clearPath();
        repaint(region);
        closeDemonstration();
        return;
    }

    // И новый регион
    m_field->setDemostrationPath(m_demostration_index);
    region += setPathRegion(m_field->path());
    // Сохраним путь для следующей итерации
    m_old_path = m_field->path();

    // Перерисуемся
    repaint(region);

    ++m_demostration_index;
}

