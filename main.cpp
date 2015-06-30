//#include <QtGui/QApplication>
#include <QApplication>
#include "mainwindow.h"
#include <QTranslator>
#include <QLocale>
#include <QDir>

const int N = 300;
const QString PathForTranslations = "./translation";


int main(int argc, char *argv[])
{
	//TODO:do not use system theme
	//QApplication::setDesktopSettingsAware(false);
/* QT5 theme?
 qApp->setStyle(QStyleFactory::create("Fusion"));
 
QPalette darkPalette;
darkPalette.setColor(QPalette::Window, QColor(53,53,53));
darkPalette.setColor(QPalette::WindowText, Qt::white);
darkPalette.setColor(QPalette::Base, QColor(25,25,25));
darkPalette.setColor(QPalette::AlternateBase, QColor(53,53,53));
darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
darkPalette.setColor(QPalette::ToolTipText, Qt::white);
darkPalette.setColor(QPalette::Text, Qt::white);
darkPalette.setColor(QPalette::Button, QColor(53,53,53));
darkPalette.setColor(QPalette::ButtonText, Qt::white);
darkPalette.setColor(QPalette::BrightText, Qt::red);
darkPalette.setColor(QPalette::Link, QColor(42, 130, 218));
 
darkPalette.setColor(QPalette::Highlight, QColor(42, 130, 218));
darkPalette.setColor(QPalette::HighlightedText, Qt::black);
qApp->setPalette(darkPalette);
 
qApp->setStyleSheet("QToolTip { color: #ffffff; background-color: #2a82da; border: 1px solid white; }");
*/
	//TODO: parser theme name from args
	QApplication::setStyle("plastique");
	
    QApplication qappWiFiSoftAp(argc, argv);
    QSettings *LocalSettings = new QSettings(QString("%1%2").arg(QDir::homePath(), "/.WiFiSoftAP/WiFiSoftAP.conf"), QSettings::NativeFormat);
    //QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    //QTextCodec::setCodecForCStrings(codec);
    QString bb = QApplication::applicationFilePath();
    QStringList argument = QCoreApplication::arguments();
    QString com;
    char command[N];
    char *tmp1;
    char buf[BUFSIZ];


	FILE *ptr;
    int test=0;
    if ((ptr = popen("id | grep uid=0", "r")) != NULL) 
		while (fgets(buf, BUFSIZ, ptr) != NULL) test++;
			pclose(ptr);


  if(test==0) {
        qDebug() << "USER: get ROOT ... ";
        if(LocalSettings->value("Program/suCommand", true).toBool() || argument.last() == "-su") {
            LocalSettings->setValue("Program/suCommand", false);
            GetRoot ppp; ppp.exec(); }
            
        com.clear();
        com = LocalSettings->value("Program/rootCommand", "gksu ").toString();

        qDebug() << "Get root with " << com << ".";

        com.toLocal8Bit();
        //tmp1 = com.toAscii().data();
        tmp1 = com.toUtf8().data();
        strcpy(command, tmp1);
        strcpy(tmp1, "");

        bb.toLocal8Bit();
        //tmp1 = bb.toAscii().data();
        tmp1 = bb.toUtf8().data();
        strcat(command, tmp1);
        bb.clear();

        system(command);
        qappWiFiSoftAp.exit();
    } else {
		QSettings *GlobalSettings = new QSettings("/root/.WiFiSoftAP/WiFiSoftAP.conf",QSettings::NativeFormat);
		qDebug() << "ROOT: ok. Begin ... ";
        // Setting the translation:
        int Translate = GlobalSettings->value("Program/Language", 0).toInt(); // 0 - automatic 1 - English 2 - Russian
		QString Trans;
		if(Translate==0) Trans = "app_" + QLocale::system().name();
		else if(Translate==1) Trans = "app_en";
		else if(Translate==2) Trans = "app_ru";
		else Trans = "app_en";

        // Set translation
        QTranslator appTranslator;
        appTranslator.load(QString("%1%2").arg(PathForTranslations, Trans), qApp->applicationDirPath());
        qappWiFiSoftAp.installTranslator(&appTranslator);
        GlobalSettings->setValue("Program/suCommand", false);

        //system tray icon
        /*
        if (!QSystemTrayIcon::isSystemTrayAvailable()) {
            QMessageBox::critical(0, QObject::tr("Systray"),
                                  QObject::tr("I couldn't detect any system tray "
                                              "on this system."));
            return 1;
        }
        */
        //start main window
        MainWindow w;
        w.show();
		
		int exitCode;
		{
			exitCode = qappWiFiSoftAp.exec();
		}

		delete GlobalSettings;
		delete LocalSettings;
		
        return exitCode; 
    }
}

