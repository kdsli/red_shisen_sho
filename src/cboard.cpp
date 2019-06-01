#include "cboard.h"

#include "csettings.h"
#include "cbackgroundmanager.h"
#include "ctilesmanager.h"

#include <QResizeEvent>
//#include <QtOpenGL/QGLWidget>

CBoard::CBoard(QWidget *parent) : QGraphicsView(parent),
    m_field(new CField(this)),
    m_scene(new CScene(m_field, this))
{
    setFrameStyle(QFrame::NoFrame);

    setScene(m_scene);
    m_scene->setBackground(bg_manager->currentFile());

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

    recalcScene();
}

void CBoard::slotRepeatGame()
{

}

void CBoard::slotHelp()
{

}

void CBoard::slotPause()
{

}

void CBoard::slotNewTypeGame()
{

}

void CBoard::slotSetTileset()
{
    tiles_manager->initCurrentFile();
    slotNewGame();
}

void CBoard::slotSetBackground()
{
    m_scene->setBackground(bg_manager->currentFile());
}

void CBoard::resizeEvent(QResizeEvent *)
{
    recalcScene();
}

void CBoard::recalcScene()
{
    auto rect = m_scene->itemsBoundingRect();
    rect.setWidth(rect.width() * 1.1);
    rect.setHeight(rect.height() * 1.1);

    fitInView(rect, Qt::KeepAspectRatio);
}
