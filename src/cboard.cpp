#include "cboard.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"

#ifdef DEFINED_OPENGL
#include <QtOpenGL/QGLWidget>
#endif

#include <QResizeEvent>

#include <QDebug>

CBoard::CBoard(QWidget *parent) : QGraphicsView(parent),
    m_field(new CField(this)),
    m_scene(nullptr),
    m_is_path(false)
{
    setFrameStyle(QFrame::NoFrame);
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_OpaquePaintEvent);
    setOptimizationFlag(QGraphicsView::DontAdjustForAntialiasing);

    setResizeAnchor(QGraphicsView::AnchorViewCenter);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Заполним массив типов полей
    m_field_types.insert(fz14x6, {14, 6, 4});
    m_field_types.insert(fz16x9, {16, 9, 4});
    m_field_types.insert(fz18x8, {18, 8, 4});
    m_field_types.insert(fz24x12, {24, 12, 8});
    m_field_types.insert(fz26x14, {26, 14, 8});
    m_field_types.insert(fz30x16, {30, 16, 12});

#ifdef DEFINED_OPENGL
    setViewport(new QGLWidget(QGLFormat(QGL::DoubleBuffer)));
#endif
}

void CBoard::createScene()
{
    m_scene = new CScene(m_field, this);
    m_scene->setBackground(bg_manager->currentFile());
    setScene(m_scene);

    slotNewGame();
}

QString CBoard::gameInfo()
{
    return "";
}

void CBoard::slotNewGame()
{
    m_game_state = gsNormal;

    m_field->newGame(m_field_types[settings->currentGameType()].x,
            m_field_types[settings->currentGameType()].y,
            m_field_types[settings->currentGameType()].count);

    // Заполним сцену новыми значениями
    m_scene->newGame();

    recalcView();
}

// ------------------------------------------------------------------------------------------------
void CBoard::slotRepeatGame()
{

}

// ------------------------------------------------------------------------------------------------
void CBoard::slotHelp()
{

}

// ------------------------------------------------------------------------------------------------
void CBoard::slotPause()
{
    if (m_game_state != gsPause) {
        m_scene->showMessage(tr("Игра приостановлена") + "\n" + tr("Щелкните для продолжения"), true);
        m_prev_pause_state = m_game_state;
        m_game_state = gsPause;
    } else {
        m_scene->hideMessage(true);
        m_game_state = m_prev_pause_state;
    }
}

// ------------------------------------------------------------------------------------------------
// Изменился тип игры
void CBoard::slotNewTypeGame()
{
    slotNewGame();
}

// ------------------------------------------------------------------------------------------------
// Изменился тип костяшек
void CBoard::slotSetTileset()
{
    tiles_manager->initCurrentFile();
}

// ------------------------------------------------------------------------------------------------
// Установить новый фон
void CBoard::slotSetBackground()
{
    m_scene->setBackground(bg_manager->currentFile());
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
    // Пока удаляются костяшки игнорируем мышь
     if (m_is_path) {
         event->accept();
         return;
     }
     if (event->buttons().testFlag(Qt::LeftButton))
         clickLeftButton(event);
     if (event->buttons().testFlag(Qt::RightButton) && settings->isTraining())
         clickRightButton(event);
 }

// ------------------------------------------------------------------------------------------------
// Пересчитать view (вызвается при инициализации и каждом изменении размера)
void CBoard::recalcView()
{
    fitInView(m_scene->fieldRect(), Qt::KeepAspectRatio);
}

// ------------------------------------------------------------------------------------------------
void CBoard::clickLeftButton(QMouseEvent *event)
{
    switch (m_game_state) {
    case gsNormal:
        m_scene->mouseLeft(mapToScene(event->pos()));
        break;
    case gsPause:
        slotPause();
        break;
    case gsDemostration:
//        closeDemonstration();
        break;
    case gsNotVariants:
//        startDemonstration();
        break;
    case gsVictory:
//        checkResult();
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
