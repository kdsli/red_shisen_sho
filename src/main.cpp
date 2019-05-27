#include "cmainwindow.h"

#include <QApplication>

/**************************************************************************************************
 * Программа red_shisen_sho
 * Автор: Кудинов Дмитрий kdsli@kdsli.ru, 2019
 *************************************************************************************************/

#include "csettings.h"

#include <QTranslator>
#include <QLibraryInfo>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setOrganizationName("kdsli");
    a.setApplicationName("red_shisen_sho");

    settings = new CSettings;

    auto lang = settings->currentLanguage();

    QTranslator translator;
    if (translator.load(settings->translationFileName(), settings->translationsDirName()))
        a.installTranslator(&translator);

    CMainWindow w;
    w.show();

    return a.exec();
}
