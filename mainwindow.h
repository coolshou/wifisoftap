#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextCodec>
#include <QDebug>
#include <QTimer>
#include <QTime>
#include <QSettings>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDesktopServices>
#include <QUrl>
#include <QWaitCondition>
#include <QMutex>
#include <QMutexLocker>
#include <QSystemTrayIcon>
//event
#include <QEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QCloseEvent>

//#include <SettingHandle.h>

#include "aboutwindow.h"
#include "optionswindow.h"
#include "activateap.h"
#include "editorconfig.h"
#include "disactiveap.h"
#include "dialogindicator.h"
#include "getroot.h"
#include "logsystem.h"
#include "iostream"

#include "CheckProcess.cpp"
#include "PlatformVariable.cpp"

#include "version.h"

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QIcon iconReturn(QString name);
    void FirsStartDetector();
//    void setVisible(bool visible);

//public slots:
//    void restoreGeometry();
//    void saveGeometry();
    
protected:
    void closeEvent(QCloseEvent *event);
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);

signals:
	void windowShown(bool shown);
//      void window_loaded();


public slots:
    void FromIndicator();
    void SetTrafficInformation();
    void GetSI(int N);
    void LogSystemAppend( QString );

private slots:
    void CheckUbuntuPrecsisNM();
    void CheckHostapdInitdScript();
    void ClientPriseUpdate();
    void on_Activate_clicked();
    //void on_AboutW_activated();
    void on_AboutW_triggered();    
    //void on_OptW_activated();
    void on_OptW_triggered();    
    //void on_EditorW_activated();
    void on_EditorW_triggered();    
    //void on_Log_activated();
    void on_Log_triggered();
    void console( char cmd[], char *ret);
    void checkStatus();
    void on_Disactive_clicked();

    void TrafficUpdate();


    void iconActivated(QSystemTrayIcon::ActivationReason reason);
//    void your_function();

private:
    Ui::MainWindow *ui;
    PlatformVariable Platform;

    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

    QAction *minimizeAction;
    //QAction *maximizeAction;
    QAction *restoreAction;
    QAction *quitAction;
    void createActions();
    void createTrayIcon();
    
    //Setting::Handle<QRect> _windowGeometry;
    //Setting::Handle<int> _windowState;
    
};

#endif // MAINWINDOW_H
