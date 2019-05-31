#include "cboard.h"

#include "csettings.h"
#include "cbackgroundmanager.h"

CBoard::CBoard(QWidget *parent) : QGraphicsView(parent),
    m_field(new CField(this)),
    m_scene(new CScene(m_field, this))
{
    setScene(m_scene);
    m_scene->setBackground(bg_manager->currentFile());

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

}

void CBoard::slotSetBackground()
{
    m_scene->setBackground(bg_manager->currentFile());
}
