#include "csettings.h"

#include <QApplication>
#include <QDir>

CSettings *settings;

#ifdef Q_OS_WIN
    const Qt::CaseSensitivity FILE_NAME_SENSITIVITY = Qt::CaseInsensitive;
#else
    const Qt::CaseSensitivity FILE_NAME_SENSITIVITY = Qt::CaseSensitive;
#endif // Q_OS_WIN

static const QString current_field_type_name = "current_field_type";
static const QString timer_delay_name = "timer_delay";
static const QString background_name = "background_name";
static const QString tileset_name = "tileset_name";
static const QString gravity_name = "gravity";
static const QString decision_name = "decision";
static const QString training_name = "training";
static const QString locale_name = "locale";

static const QString kmahjongglib_dir = "kmahjongglib";
static const QString tilesets_dir = "tilesets";

static const QStringList field_type_names{"14x6", "16x9", "18x8", "24x12", "26x14", "30x16"};
static const QVector<int> time_delay_values{1000, 500, 200};

static const QString default_background = "egyptian.svgz";
static const QString user_bg_dir_name = "UserBackgroungs";

static const QString translations_dir_name = "translations";
static const QString translation_file_postfix = "red_shisen_sho_";

CSettings::CSettings() : CKeeperSettings ()
{
    m_current_field_type = static_cast<FieldType>(value(current_field_type_name, fz18x8).toInt());
    m_timer_delay = value(timer_delay_name, 1).toInt();
    m_background = value(background_name, "egyptian.svgz").toString();
    m_tileset = value(tileset_name, "default.svgz").toString();
    m_gravity = value(gravity_name, true).toBool();
    m_decision = value(decision_name, true).toBool();
    m_training = value(training_name, true).toBool();
    m_locale = value(locale_name, QLocale::system().bcp47Name()).toString();

    // Если такого перевода нет - сделаем по умолчанию
    initLanguagesList();
    if (!m_languages.contains(m_locale))
        m_locale = QLocale::system().bcp47Name();

    // Проверим существование пользовательской директории и создадим ее если надо
    m_user_bg_dir = qApp->applicationDirPath() + QDir::separator() + user_bg_dir_name + QDir::separator();
    QDir dir(m_user_bg_dir);
    if (!dir.exists()) dir.mkdir(m_user_bg_dir);
}

const QString CSettings::translationsDirName()
{
    return translations_dir_name;
}

const QString CSettings::translationFileName()
{
    return translation_file_postfix + m_locale;
}

const LanguagesList CSettings::listLanguages()
{
    return m_languages;
}

// Список найденных языков переводов
void CSettings::initLanguagesList()
{
    QDir dir(qApp->applicationDirPath() + QDir::separator() + settings->translationsDirName());
    QRegExp re(translation_file_postfix + "(..)" + "\\.qm");

    m_languages.clear();

    QStringList filter{translation_file_postfix + "??.qm"};
    for (const auto &file : dir.entryList(filter, QDir::Files | QDir::Readable)) {
        // Выделим название языка
        if (re.indexIn(file) == 0) {
            auto lang = re.cap(1);
            auto name = QLocale(lang).nativeLanguageName();
            if(!name.isEmpty()) name[0] = name[0].toUpper();
            m_languages.insert(lang, name);
        }
    }
}

// Текущий язык в виде строки
QString CSettings::currentLanguage()
{
    return m_languages[m_locale];
}

// Установить локаль (по названию Russian, English и т.д.)
void CSettings::setCurrentLanguage(const QString &language)
{
    // Найти в m_languages
    auto it = std::find_if(m_languages.cbegin(), m_languages.cend(), [&language](const auto lang){
        return lang.compare(language) == 0;
    });

    // Паранойя... на всякий случай
    if (it == m_languages.cend())
        m_locale = QLocale::system().bcp47Name();

    m_locale = it.key();
    setValue(locale_name, it.key());
}

void CSettings::setFieldType(FieldType field_type)
{
    m_current_field_type = field_type;
    setValue(current_field_type_name, field_type);
}

void CSettings::setTimerDelay(int timer_delay)
{
    m_timer_delay = timer_delay;
    setValue(timer_delay_name, timer_delay);
}

// Непосредственно время, в течении которого снимаются костяшки в милисекундах
int CSettings::timeDelay() const
{
    return time_delay_values[m_timer_delay];
}

const QStringList &CSettings::fieldTypeNames()
{
    return field_type_names;
}

// Путь к директории изображений mahjongg
const QString CSettings::mahjonggLibDir() const
{
    return qApp->applicationDirPath() + QDir::separator() + kmahjongglib_dir + QDir::separator();
}

void CSettings::setBackground(const QString &background)
{
    m_background = background;
    setValue(background_name, background);
}

void CSettings::setDefaultBackground()
{
    m_background = default_background;
}

void CSettings::setTileset(const QString &tileset)
{
    m_tileset = tileset;
    setValue(tileset_name, tileset);
}

void CSettings::setGravity(bool gravity)
{
    m_gravity = gravity;
    setValue(gravity_name, gravity);
}

void CSettings::setDecision(bool decision)
{
    m_decision = decision;
    setValue(decision_name, decision);

}

void CSettings::setTraining(bool training)
{
    m_training = training;
    setValue(training_name, training);
}
