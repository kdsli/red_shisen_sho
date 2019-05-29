#ifndef CSETTINGS_H
#define CSETTINGS_H

#include "ckeepersettings.h"

#include <QVector>

enum GameType { fz14x6, fz16x9, fz18x8, fz24x12, fz26x14, fz30x16 };

using LanguagesList = QMap<QString, QString>;
using GameTypes = QVector<QPair<GameType, QString>>;

// Настройки игры. Все параметры, сохраняемые в настройках, хранятся тут.
// Не дублируются в остальных частях программы

class CSettings : public CKeeperSettings
{
    Q_OBJECT
public:
    CSettings();

    // Путь к директории трансляции
    const QString translationsDirName();
    // Название файла текущей трансляции
    const QString translationFileName();
    // Список найденых языков
    const LanguagesList listLanguages();
    // Текущий язык (ru, en и т.д.)
    QString currentLanguage();
    void setCurrentLanguage(const QString &);
    void setCurrentLanguageIndex(int);

    // Массив игр
    const GameTypes &games();
    // Список только названий игр
    const QStringList &gamesName() const { return m_game_names; }

    // Путь к директории изображений mahjongg
    const QString mahjonggLibDir() const;

    // Текущий тип игры
    GameType currentGameType() const { return m_current_game_type; }
    // Индекс текущей игры в массиве GameTypes
    int currentGameIndex() const;
    void setCurrentGameType(GameType);

    // Настройка задержки (опрерирует целым от 0 до n)
    int timerDelayNumber() const { return m_timer_delay; }
    void setTimerDelay(int);
    // Непосредственно время, в течении которого снимаются костяшки, в милисекундах
    int timeDelay() const;

    // Имя текущего фона
    const QString bgName() const { return m_background; }
    void setBackground(const QString &);
    // Установить фон по умолчанию (только для этого сеанса)
    void setDefaultBackground();

    // Имя текущего наборы костяшек
    const QString currentTilesetName() const { return m_tileset; }
    void setCurrentTileset(const QString &);

    // Гравитация
    bool isGravity() const { return m_gravity; }
    void setGravity(bool gravity);

    // Не начинать нерешаемых задач
    bool isDecision() const { return m_decision; }
    void setDecision(bool decision);

    // Режим тренировки
    bool isTraining() const { return m_training; }
    void setTraining(bool training);

    // Вернуть имя пользователя
    QString userName();

private:
    GameType m_current_game_type;
    int m_timer_delay;
    QString m_background;
    QString m_tileset;
    bool m_gravity;
    bool m_decision;
    bool m_training;
    QStringList time_delay_names;
    QString m_locale;
    LanguagesList m_languages;
    QStringList m_game_names;

    void initLanguagesList();

};

extern CSettings *settings;
extern const Qt::CaseSensitivity FILE_NAME_SENSITIVITY;

#endif // CSETTINGS_H
