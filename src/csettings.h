#ifndef CSETTINGS_H
#define CSETTINGS_H

#include "ckeepersettings.h"

using LanguagesList = QMap<QString, QString>;

// Настройки игры. Все параметры, сохраняемые в настройках, хранятся тут.
// Не дублируются в остальных частях программы

class CSettings : public CKeeperSettings
{
    Q_OBJECT
public:
    CSettings();

    enum FieldType { fz14x6, fz16x9, fz18x8, fz24x12, fz26x14, fz30x16 };

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

    // Массив названий игра
    const QStringList &fieldTypeNames();

    // Путь к директории изображений mahjongg
    const QString mahjonggLibDir() const;
    // Путь с изображениями пользователя
    const QString userBGDir() const { return m_user_bg_dir; }

    // Настройка типа игры
    FieldType fieldType() const { return m_current_field_type; }
    void setFieldType(FieldType);

    // Настройка задержки (опрерирует целым от 0 до n)
    int timerDelay() const { return m_timer_delay; }
    void setTimerDelay(int);
    // Непосредственно время, в течении которого снимаются костяшки в милисекундах
    int timeDelay() const;

    // Имя текущего фона
    const QString bgName() const { return m_background; }
    void setBackground(const QString &);
    void setDefaultBackground();

    // Имя текущего наборы костяшек
    const QString tilesetName() const { return m_tileset; }
    void setTileset(const QString &);

    // Гравитация
    bool isGravity() const { return m_gravity; }
    void setGravity(bool gravity);

    // Не начинать нерешаемых задач
    bool isDecision() const { return m_decision; }
    void setDecision(bool decision);

    // Режим тренировки
    bool isTraining() const { return m_training; }
    void setTraining(bool training);

private:
    FieldType m_current_field_type;
    int m_timer_delay;
    QString m_background;
    QString m_tileset;
    bool m_gravity;
    bool m_decision;
    bool m_training;
    QString m_user_bg_dir;
    QStringList time_delay_names;
    QString m_locale;
    LanguagesList m_languages;

    void initLanguagesList();

};

extern CSettings *settings;
extern const Qt::CaseSensitivity FILE_NAME_SENSITIVITY;

#endif // CSETTINGS_H
