#include "cboard.h"

#include "csettings.h"
#include "cbackgroundmanager.h"

CBoard::CBoard(QWidget *parent) : QGraphicsView(parent),
    m_scene(new CScene(this)),
    m_game(new CGame(m_scene, this))
{
    setScene(m_scene);
    m_scene->setBackground(bg_manager->currentFile());

    slotNewGame();
}

QString CBoard::gameInfo()
{
    return "";
}

void CBoard::slotNewGame()
{
    m_game->newGame();
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
