#include "mainwindow.h"
#include "ui_mainwindow.h"

///////////////////////////////////////////////////////////////////////////////
int Status = 0; // The status of a point of access
//QString ProgramVersion = "1.1"; // software Version
QString ProgramVersion = WIFISOFTAP_VERSION; // software Version

unsigned long long GlobalTrafficIn = 0; // current Incoming traffic
unsigned long long GlobalTrafficOut = 0; //current Outgoing traffic

unsigned long long GlobalTrafficInOld = 0; // old Incoming traffic
unsigned long long GlobalTrafficOutOld = 0; // old Outgoing traffic

unsigned long long GlobalTrafficInSpeedINT = 0; // Incoming bit rate
unsigned long long GlobalTrafficOutSpeedINT = 0; // Outgoing bit rate

unsigned long long GlobalTrafficInMaxSpeedINT = 0; // Maximum Incoming bit rate
unsigned long long GlobalTrafficOutMaxSpeedINT = 0; // Maximum Outgoing bits rate

float GlobalTrafficInFloat = 0.1; // Incoming traffic in the final result
float GlobalTrafficOutFloat = 0.1; // Outgoing traffic in the final result

float GlobalTrafficInSpeed = 0.0; // Download speed
float GlobalTrafficOutSpeed = 0.0; // outgoing speed

float GlobalTrafficInMaxSpeed = 0.0; // Maximum input speed
float GlobalTrafficOutMaxSpeed = 0.0; // The maximum speed of outgoing

QString InSI = "Byte"; // Units incoming traffic
QString OutSI = "Byte"; // Units Outgoing traffic

QString InSISpeed = "Byte"; // Units incoming speed
QString OutSISpeed = "Byte"; // Units Outgoing speed

QString InSIMaxSSpeed = "Byte"; // Units Maximum input speed
QString OutSIMaxSpeed = "Byte"; // Units Maximum Outgoing speed
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    //_windowGeometry("WindowGeometry"),
    //_windowState("WindowState", 0),
    ui(new Ui::MainWindow)
{
    QSettings *GlobalSettings = new QSettings("/root/.WiFiSoftAP/WiFiSoftAP.conf",QSettings::NativeFormat);
    ui->setupUi(this);

    if(GlobalSettings->value("Program/version", "").toString() != ProgramVersion)
            GlobalSettings->setValue("Program/version", ProgramVersion); // We update the version in a config

    //system tray icon
    createActions();
    createTrayIcon(); // TODO: why cause MainWindow not showup?

    //TODO: detect interface & gateway to internet
    //
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(Platform.DisableDNSMASQinUbuntuPreciseInNM)
        CheckUbuntuPrecsisNM(); // check, Ubuntu 12.04 Is not this where I have taken control DNSMASQ?
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    if(Platform.CheckHOSTAPDscript)
        CheckHostapdInitdScript(); // check, Ubuntu Is not this where broke startup script Hostapd?
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    FirsStartDetector(); // check, Is this not the first launch of this program? Maybe it is necessary to restore configs?
    ////////////////////////////////////////////////////////////////////////////////////////////////////////////

    QDateTime TimeNow;
    QString dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());
    LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("WiFi Soft AP"), dateUNIXNow, tr("Program loaded and ready"), QString("0")));
    
    checkStatus();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(TrafficUpdate()));
    timer->start(GlobalSettings->value("Program/UpdateTraffic", 1).toInt() * 1000);

    QTimer *ClientUpdate = new QTimer(this);
    connect(ClientUpdate, SIGNAL(timeout()), this, SLOT(ClientPriseUpdate()));
    ClientUpdate->start(GlobalSettings->value("Program/ClientsPriceUpdateTime", 5).toInt() * 1000);
    // Icons?
    ui->Activate->setIcon(iconReturn("system-run"));
    ui->Disactive->setIcon(iconReturn("dialog-close"));
    ui->Exit->setIcon(iconReturn("application-exit"));
    ui->OptW->setIcon(iconReturn("configure"));
    ui->Log->setIcon(iconReturn("applications-system"));
    ui->EditorW->setIcon(iconReturn("document-edit"));
    ui->AboutW->setIcon(iconReturn("dialog-information"));
    //ui->trayIcon->setIcon(iconReturn("wifisoftap"));

    //connect(this, SIGNAL(window_loaded), this, SLOT(your_function()));

    delete GlobalSettings;
}

MainWindow::~MainWindow()
{
    QDateTime TimeNow;
    QString dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());
    LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("WiFi Soft AP"), dateUNIXNow, tr("Program closed"), QString("0")));
    delete ui;
}
/*
void MainWindow::restoreGeometry() {
    // Did not use setGeometry() on purpose,
    // see http://doc.qt.io/qt-5/qsettings.html#restoring-the-state-of-a-gui-application
    QRect geometry = _windowGeometry.get(qApp->desktop()->availableGeometry());
    move(geometry.topLeft());
    resize(geometry.size());

    // Restore to maximized or full screen after restoring to windowed so that going windowed goes to good position and sizes.
    Qt::WindowStates state = (Qt::WindowStates)_windowState.get(Qt::WindowNoState);
    if (state != Qt::WindowNoState) {
        setWindowState(state);
    }
}

void MainWindow::saveGeometry() {
    // Did not use geometry() on purpose,
    // see http://doc.qt.io/qt-5/qsettings.html#restoring-the-state-of-a-gui-application
    _windowState.set((int)windowState());

    // Save position and size only if windowed so that have good values for windowed after starting maximized or full screen.
    if (windowState() == Qt::WindowNoState) {
        QRect geometry(pos(), size());
        _windowGeometry.set(geometry);
    }
}
*/
void MainWindow::FirsStartDetector() {
	QSettings *GlobalSettings = new QSettings("/root/.WiFiSoftAP/WiFiSoftAP.conf",QSettings::NativeFormat);
	QDateTime TimeNow;
	QString dateUNIXNow;

	if(GlobalSettings->value("Program/FirstLoad", true).toBool()) {
		OptionsWindow options;
		options.on_APSave_clicked(true);
		options.on_DHCPSave_clicked(true);

		dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());
		LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("Options"), dateUNIXNow, tr("Configs created succesfully."), QString("1")));
		GlobalSettings->setValue("Program/FirstLoad", false);
	}
	delete GlobalSettings;
}

void MainWindow::ClientPriseUpdate() {
	// Slot update client list after a preset period of time
	QSettings *GlobalSettings = new QSettings("/root/.WiFiSoftAP/WiFiSoftAP.conf",QSettings::NativeFormat);
	QFile leaseIP("/var/lib/misc/dnsmasq.leases");
	QString LineRead;
	QStringList fields;
	QStringList horizontal;
	QStringList vertical;
	QTableWidgetItem *item = 0;
	QDateTime *date = 0;
	int i = 0;
	int f = 1;
	int timeconnect;

	horizontal << tr("IP") << tr("Connected in") << tr("Device") << tr("Rent to");
	ui->Clients->setHorizontalHeaderLabels(horizontal);
	ui->Clients->verticalHeader()->setVisible(true);

	if(leaseIP.open(QIODevice::ReadOnly)) {
		// file has been successfully opened. Now read line by line
		ui->Clients->setRowCount(0);

			while(!leaseIP.atEnd()) { // until the end of file
			LineRead = leaseIP.readLine();
			fields = LineRead.split(" ");
			if(fields.size()>0) {
				 // If this line is not empty, then fill in the fields value
				ui->Clients->setRowCount(ui->Clients->rowCount()+1);
				vertical << QString("%1").arg(f);
				f++;
				ui->Clients->setVerticalHeaderLabels(vertical);

				item = new QTableWidgetItem(fields.at(2));
				ui->Clients->setItem(i,0,item);

				timeconnect = fields.at(0).toInt();
				date = new QDateTime;
				date->setTime_t(timeconnect);
				item = new QTableWidgetItem(date->toString("hh:mm:ss"));
				ui->Clients->setItem(i,1,item);

				item = new QTableWidgetItem(fields.at(1));
				ui->Clients->setItem(i,2,item);

				timeconnect = fields.at(0).toInt();

				switch(GlobalSettings->value("DHCP/IP_time", 2).toInt()) {
					case(0): timeconnect += 10*60; break;
					case(1): timeconnect += 30*60; break;
					case(2): timeconnect += 60*60; break;
					case(3): timeconnect += 120*60; break;
					case(4): timeconnect += 6*60*60; break;
					case(5): timeconnect += 12*60*60; break;
				}

				date = new QDateTime;
				date->setTime_t(timeconnect);
				item = new QTableWidgetItem(date->toString("hh:mm:ss"));
				ui->Clients->setItem(i,3,item);

				i++;
			} // if the string is empty, well ... Clears the list and go further
			fields.clear();
		}
	} // Could not open file
	leaseIP.close();
	delete GlobalSettings;
}

void MainWindow::TrafficUpdate() {
	QString tempt_QT;
	QSettings *GlobalSettings = new QSettings("/root/.WiFiSoftAP/WiFiSoftAP.conf",QSettings::NativeFormat);
	QString line;
	QStringList Input;
	QStringList Elements;
	int i = 0;
	tempt_QT = GlobalSettings->value("AP/Iface", "wlan0").toString().toLocal8Bit();
	ui->TrafficTable->horizontalHeader()->setVisible(true);

	QFile file_to_open("/proc/net/dev");
	file_to_open.open(QIODevice::ReadOnly);

	if(file_to_open.isOpen()) {

		QTextStream in(&file_to_open);
		line = in.read(102400);
		line.replace("  ", " ");
		line.replace("  ", " ");
		line.replace("  ", " ");
		line.replace("  ", " ");
		line.replace("  ", " ");
		line.replace("  ", " ");
		line.replace("  ", " ");
		line.replace("  ", " ");

		Input = line.split("\n");
		line.clear();

		for(i=0;i<Input.count();i++) {
			// check, so that the line does not start with a space
			if(Input.at(i).startsWith(" "))
				line = Input.at(i).mid(1, Input.at(i).length()-1);
			else
				line = Input.at(i);

			// check, whether this line to our interface
			if(line.startsWith(tempt_QT)) {
				// Yes relates
				Elements = line.split(" ");

				// Swap traffic
				GlobalTrafficInOld = GlobalTrafficIn;
				GlobalTrafficOutOld = GlobalTrafficOut;

				// Installing new traffic
				GlobalTrafficIn = Elements.at(1).toLongLong();
				GlobalTrafficOut = Elements.at(9).toLongLong();
			}
		}
	} else { qDebug() <<"error!"; }

	file_to_open.close();


	SetTrafficInformation();

	delete GlobalSettings;
}

void MainWindow::SetTrafficInformation() {
    // This function is designed to calculate the traffic
    // Firstly, it sets the value in the traffic
    // Secondly, it calculates the average speed
    // Thirdly, in the future will write in logs traffic per session
    // Fourth, will draw graphic, or will not, do not know yet ...
    ////////////////////////////////////////////////////////////////////
    // We believe traffic
    ////////////////////////////////////////////////////////////////////
    QString Temp;
    QTableWidgetItem *item = 0;
    QString a;
    int s=0, len=0;
    ////////////////////////////////////////////////////////////////////
    // call functions
    ////////////////////////////////////////////////////////////////////
    GetSI(0); // Incoming traffic
    GetSI(1); // Outgoing traffic

    GetSI(2); // Download speed
    GetSI(3); // outgoing speed

    GetSI(4); // Incoming maximum speed
    GetSI(5); // Outgoing maximum speed
    ////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////
    // 0
    ////////////////////////////////////////////////////////////////////
    a = QString("%1").arg(GlobalTrafficInFloat);
    s = a.lastIndexOf(".");
    len = a.length()-2;
    if(s <= len) { len = a.lastIndexOf(".")+3; a = a.mid(0, len); }
    a.append(" ");
    a.append(InSI);
    item = new QTableWidgetItem(a);
    ui->TrafficTable->setItem(0,0,item);
    a.clear();
    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////
    // 1
    ////////////////////////////////////////////////////////////////////
    a = QString("%1").arg(GlobalTrafficOutFloat);
    s = a.lastIndexOf(".");
    len = a.length()-2;
    if(s <= len) { len = a.lastIndexOf(".")+3; a = a.mid(0, len); }
    a.append(" ");
    a.append(OutSI);
    item = new QTableWidgetItem(a);
    ui->TrafficTable->setItem(0,1,item);
    a.clear();
    ////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////
    // 2
    ////////////////////////////////////////////////////////////////////
    a = QString("%1").arg(GlobalTrafficInSpeed);
    s = a.lastIndexOf(".");
    len = a.length()-2;
    if(s <= len) { len = a.lastIndexOf(".")+3; a = a.mid(0, len); }
    a.append(" ");
    a.append(InSISpeed);
    a.append(tr("/S"));
    item = new QTableWidgetItem(a);
    ui->TrafficTable->setItem(1,0,item);
    a.clear();
    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////
    // 3
    ////////////////////////////////////////////////////////////////////
    a = QString("%1").arg(GlobalTrafficOutSpeed);
    s = a.lastIndexOf(".");
    len = a.length()-2;
    if(s <= len) { len = a.lastIndexOf(".")+3; a = a.mid(0, len); }
    a.append(" ");
    a.append(OutSISpeed);
    a.append(tr("/S"));
    item = new QTableWidgetItem(a);
    ui->TrafficTable->setItem(1,1,item);
    a.clear();
    ////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////
    // 4
    ////////////////////////////////////////////////////////////////////
    a = QString("%1").arg(GlobalTrafficInMaxSpeed);
    s = a.lastIndexOf(".");
    len = a.length()-2;
    if(s <= len) { len = a.lastIndexOf(".")+3; a = a.mid(0, len); }
    a.append(" ");
    a.append(InSIMaxSSpeed);
    a.append(tr("/S"));
    item = new QTableWidgetItem(a);
    ui->TrafficTable->setItem(2,0,item);
    a.clear();
    ////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////
    // 5
    ////////////////////////////////////////////////////////////////////
    a = QString("%1").arg(GlobalTrafficOutMaxSpeed);
    s = a.lastIndexOf(".");
    len = a.length()-2;
    if(s <= len) { len = a.lastIndexOf(".")+3; a = a.mid(0, len); }
    a.append(" ");
    a.append(OutSIMaxSpeed);
    a.append(tr("/S"));
    item = new QTableWidgetItem(a);
    ui->TrafficTable->setItem(2,1,item);
    a.clear();
    ////////////////////////////////////////////////////////////////////



    ////////////////////////////////////////////////////////////////////
    // End this function
    ////////////////////////////////////////////////////////////////////
}

void MainWindow::GetSI(int N) {
    // This feature set units and number
    float PP = 0;
    float Test = 0;
    QString Label;

    if(N==0) {
    // We have to deal with incoming traffic
    PP = GlobalTrafficIn;
    }  else if(N==1) {
    // We are dealing with outgoing traffic
    PP = GlobalTrafficOut;
    }  else if(N==2) {
    // We are dealing with incoming speed
    PP = GlobalTrafficIn - GlobalTrafficInOld;
    GlobalTrafficInSpeedINT = GlobalTrafficIn - GlobalTrafficInOld;
    }  else if(N==3) {
    // We are dealing with outgoing speed
    PP = GlobalTrafficOut - GlobalTrafficOutOld;
    GlobalTrafficOutSpeedINT = GlobalTrafficOut - GlobalTrafficOutOld;
    }  else if(N==4) {
    // We are dealing with a maximum speed of incoming
    // If the current speed is greater than the maximum, then we do the current maximum
        if(GlobalTrafficInMaxSpeedINT<=GlobalTrafficInSpeedINT && GlobalTrafficInOld>1) { GlobalTrafficInMaxSpeedINT = GlobalTrafficInSpeedINT; }
        PP = GlobalTrafficInMaxSpeedINT;
    } else if(N==5) {
    // We are dealing with a maximum speed of outgoing
    // If the current speed is greater than the maximum, then we do the current maximum
        if(GlobalTrafficOutMaxSpeedINT<=GlobalTrafficOutSpeedINT && GlobalTrafficOutOld>1) { GlobalTrafficOutMaxSpeedINT = GlobalTrafficOutSpeedINT; }
        PP = GlobalTrafficOutMaxSpeedINT;
    }

    ////////////////////////////////////////////////////
    // We get the symbols and values
    ////////////////////////////////////////////////////
    Label = tr("Byte");

    Test = PP / 1024.0;
    if(Test>=0.98) {
        // The following units
        Label = tr("KB");
        PP = Test;
    }

    Test = PP / (1024.0);
    if(Test>=0.98) {
        // The following units
        Label = tr("MB");
        PP = Test;
    }

    Test = PP / (1024.0);
    if(Test>=0.98) {
        // The following units
        Label = tr("GB");
        PP = Test;
    }

    Test = PP / (1024.0);
    if(Test>=0.98) {
        // The following units
        Label = tr("TB");
        PP = Test;
    }

    //////////////////////////////////////////////////
    // Returns the value back into the global variables
    //////////////////////////////////////////////////
    if(N==0) {
    // We have to deal with incoming traffic
    GlobalTrafficInFloat = PP;
    InSI = Label; }

    else if(N==1) {
    // We are dealing with outgoing traffic
        GlobalTrafficOutFloat = PP;
        OutSI = Label;
    }
    else if(N==2) {
    // We are dealing with incoming speed
    GlobalTrafficInSpeed = PP;
    InSISpeed = Label;
    }
    else if(N==3) {
    // We are dealing with outgoing speed
    GlobalTrafficOutSpeed = PP;
    OutSISpeed = Label;
    }
    else if(N==4) {
    //We are dealing with a maximum speed of incoming
    GlobalTrafficInMaxSpeed = PP;
    InSIMaxSSpeed = Label;
    }
    else if(N==5) {
    // We are dealing with a maximum speed of outgoing
    GlobalTrafficOutMaxSpeed = PP;
    OutSIMaxSpeed = Label;
    }
}

void MainWindow::on_Activate_clicked()
{
// Start activation APs

    DialogIndicator ActForm;
    ActForm.SetDo(0); // activatable
    ActForm.setWindowTitle(tr("Working..."));
    connect(&ActForm, SIGNAL(to_Mainwindow()), this, SLOT(FromIndicator()));
    connect(&ActForm, SIGNAL(toLogInMainWindow(QString)), this, SLOT(LogSystemAppend(QString)));
    ui->Disactive->setEnabled(false);
    ui->Activate->setEnabled(false);
    ActForm.exec();
}

void MainWindow::on_Disactive_clicked()
{
// Start the deactivation of the access point

    DialogIndicator ActForm;
    ActForm.SetDo(1); // deactivation
    ActForm.setWindowTitle(tr("Working..."));
    connect(&ActForm, SIGNAL(to_Mainwindow()), this, SLOT(FromIndicator()));
    connect(&ActForm, SIGNAL(toLogInMainWindow(QString)), this, SLOT(LogSystemAppend(QString)));
    ui->Disactive->setEnabled(false);
    ui->Activate->setEnabled(false);

    ActForm.exec();
}

//void MainWindow::on_AboutW_activated()
void MainWindow::on_AboutW_triggered()
{
   // Show Window "About the program"
   AboutWindow about;
   about.exec();
}

//void MainWindow::on_OptW_activated()
void MainWindow::on_OptW_triggered()
{
    OptionsWindow OptW;
    connect(&OptW, SIGNAL(toMainLog(QString)), this, SLOT(LogSystemAppend(QString)));
    OptW.exec();
}

void MainWindow::console( char cmd[], char *ret) {

    char buf[BUFSIZ];
         FILE *ptr;

         if ((ptr = popen(cmd, "r")) != NULL)
              while (fgets(buf, BUFSIZ, ptr) != NULL)
                  strcat(ret, buf);
                 pclose(ptr);
}

void MainWindow::checkStatus() {
    int a[4] = {0,0,0,0};
    int tempI = 0, i=0, tempI2 = 0;
    QDateTime TimeNow;
    QString dateUNIXNow;
    QSettings *GlobalSettings = new QSettings("/root/.WiFiSoftAP/WiFiSoftAP.conf",QSettings::NativeFormat); // Creation of new object
    bool starting = true;
    bool test;
    QString temp_QT;
    qDebug() << "Testing AP... ";
    ////////////////////////////////////////////////////////////////////////////////////////////
    // Whether the point of access is activated?
    ////////////////////////////////////////////////////////////////////////////////////////////
    // The point of access is activated, if
// 1. status  Arrangements
    ////////////////////////////////////////////////////////////////////////////////////////////
    CheckProcess IFace(1, GlobalSettings->value("AP/Iface", "wlan0").toString());
    if(!IFace.init()) { starting=false; a[0]=0; qDebug() << "DEVICE - " << "OFF"; }
    else { qDebug() << "DEVICE - " << "ON"; a[0]=1; }

// 2. status DNSMASQ
 ////////////////////////////////////////////////////////////////////////////////////////////
	CheckProcess DNSMASQcheck("dnsmasq");
	test = DNSMASQcheck.init();
	if(test) { qDebug() << "DNSMASQ - " << "ON"; a[1]=1; }
	else { starting=false; a[1]=0; qDebug() << "DNSMASQ - " << "OFF"; }


// 3. status Hostapd
    ////////////////////////////////////////////////////////////////////////////////////////////
    CheckProcess HOSTAPDcheck("hostapd");
    test = HOSTAPDcheck.init();
    if(!test) { starting=false; a[2]=0; qDebug() << "HOSTAPD - " << "OFF"; }
        else { qDebug() << "HOSTAPD - " << "ON"; a[2]=1; }

// 4. status IP Forwarding
    ////////////////////////////////////////////////////////////////////////////////////////////
    CheckProcess IPForwarding(2, "");
    if(!IPForwarding.init())   { starting=false; a[3]=0; qDebug() << "IP FORWARDING - " << "OFF"; }
    else { qDebug() << "IP FORWARDING - " << "ON"; a[3]=1; }

    ////////////////////////////////////////////////////////////////////////////////////////////
    if(starting==true) {
        ui->identificator->setText(tr("<h3><font color=\"green\">It is activated</font></h3>"));
        ui->StatusIcon->setPixmap(QPixmap(":/pic/icons/dialog-ok-apply.png"));
        ui->Activate->setEnabled(false);
        ui->Disactive->setEnabled(true);
        qDebug() << "AP IS RUNNING \n";

        dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());
        LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("Program"), dateUNIXNow, tr("AP online"), QString("0")));

    } else {
        ui->identificator->setText(tr("<h3><font color=\"red\">It is disconnected</font></h3>"));
        ui->StatusIcon->setPixmap(QPixmap(":/pic/icons/dialog-close.png"));
        ui->Activate->setEnabled(true);
        ui->Disactive->setEnabled(false);
        qDebug() << "AP IS NOT RUNNING\n";
    ////////////////////////////////////////////////////////////////////////////////////////////
    // We find out what is wrong
    ////////////////////////////////////////////////////////////////////////////////////////////
        for(i=0;i<4;i++) if(a[i]==0) tempI++; // We consider how many items are not active

        temp_QT.clear(); temp_QT = tr("AP offline because ");
        if(a[0]==0) { temp_QT.append(tr("network interface is down")); tempI2++;
        if(tempI>tempI2) temp_QT.append(tr(" and ")); }
        if(a[1]==0) { temp_QT.append(tr("DNSMASQ offline")); tempI2++;
        if(tempI>tempI2) temp_QT.append(tr(" and ")); }
        if(a[2]==0) { temp_QT.append(tr("Hostapd offline")); tempI2++;
        if(tempI>tempI2) temp_QT.append(tr(" and ")); }
        if(a[3]==0) { temp_QT.append(tr("IP Forward disabled")); }
        temp_QT.append(tr("."));

        dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());
        LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("Program"), dateUNIXNow, temp_QT, QString("0")));

    }
    ////////////////////////////////////////////////////////////////////////////////////////////
    delete GlobalSettings;

}

//void MainWindow::on_EditorW_activated()
void MainWindow::on_EditorW_triggered()
{
    // Show Window "editor"
    EditorConfig editor;
    editor.exec();
}

void MainWindow::FromIndicator() {
    //sleep(1);
    QWaitCondition wc;
	QMutex mutex;
	QMutexLocker locker(&mutex);
	wc.wait(&mutex, 1000);
	
	//QTest::qSleep ( 1000 )
    checkStatus();
}

//void MainWindow::on_Log_activated()
void MainWindow::on_Log_triggered()
{
    LogSystem logsystem;
    logsystem.exec();
}

QIcon MainWindow::iconReturn(QString name) {
    QString iconPath = ":/pic/icons/";
    QIcon icon = QIcon::fromTheme(name);
    if (icon.isNull())
        icon = QIcon(QString("%1%2%3").arg(iconPath, name, ".png"));
    return icon;
}

void MainWindow::LogSystemAppend(QString Input) {
    ////////////////////////////////////////////////////////////////////////////////////////
	QDir directory;
	QString TempString;
	bool t=false;
	QDate dateNow;
	TempString  = dateNow.currentDate().toString("yyyy.MM.d");
    ////////////////////////////////////////////////////////////////////////////////////////
    // Logs are in /root/.WiFiSoftAP/logs/today's date/log.txt
    ////////////////////////////////////////////////////////////////////////////////////////
    // 1. Go to the folder /root/.WiFiSoftAP/logs/
    ////////////////////////////////////////////////////////////////////////////////////////
       directory.cd("/root");
    if(!directory.cd(".WiFiSoftAP")) {
        directory.mkdir(QString("%1").arg(".WiFiSoftAP"));
        directory.cd(".WiFiSoftAP"); }
    if(!directory.cd("logs")) {
        directory.mkdir(QString("%1").arg("logs"));
        directory.cd("logs"); }
    ////////////////////////////////////////////////////////////////////////////////////////
    // 2. See the list of files, and if there is a folder with today's date, then everything is fine,
    // If not, then it ?
    ////////////////////////////////////////////////////////////////////////////////////////

	QStringList listDir = directory.entryList(QDir::Dirs);
		foreach (QString subdir, listDir) {
			if (subdir == "." || subdir == "..") continue;
			else {
				if(subdir==TempString) t = true;
			}
		}
   if(!t) directory.mkdir(TempString);
    //////////////////////////////////////////////////////////////////////////////////////////
    //// 3. Open the file to append line
    //////////////////////////////////////////////////////////////////////////////////////////
	QFile LogFileNew(QString("%1%2%3").arg("/root/.WiFiSoftAP/logs/", TempString, "/log.txt"));
	LogFileNew.open(QIODevice::Append | QIODevice::Text | QIODevice::Unbuffered);
	QTextStream LogFileOout(&LogFileNew);
    //////////////////////////////////////////////////////////////////////////////////////////
    //// 4. Form a log and write to a file
    //////////////////////////////////////////////////////////////////////////////////////////
	QString InputNew = Input.replace("\n","");
	LogFileOout << InputNew << "\n";
    //// Close file
	LogFileNew.close();
}

void MainWindow::CheckUbuntuPrecsisNM() {
 // The function created to check if as DNS DNSMASQ settings NetworkManager
 // If so, and if the program is running in the operating system Ubuntu 12.04, it will disable it
//////////////////////////////////////////////////////////////////////////////////////////
// 1 move - check distribution:
//////////////////////////////////////////////////////////////////////////////////////////
	char temp[200];
	QString qtTemp;
	QDateTime TimeNow;
	QString dateUNIXNow;
	bool test = false;
	// qDebug() << "Testing DNS... ";

	QFile file_TEST("/etc/NetworkManager/NetworkManager.conf");
	QFile file_LSB("/etc/lsb-release");
	QFile file("/etc/NetworkManager/NetworkManager.conf");
	QFile fileTo("/etc/NetworkManager/NetworkManager.conf.new");
	QString Line;

	if(file_LSB.open(QFile::ReadOnly)) {
		while(!file_LSB.atEnd()) {
			qtTemp = file_LSB.readLine();
			if(qtTemp.startsWith("DISTRIB_CODENAME") && qtTemp.endsWith("precise\n"))
				test = true;
		}
		file_LSB.close();
		qtTemp.clear();
	}

	if(test && file_TEST.exists()){
		  //  qDebug() << "This is Ubuntu 12.04.";
		//////////////////////////////////////////////////////////////////////////////////////////
		// 2 move - check, Do not vyklyuchin earlier DNS
		//////////////////////////////////////////////////////////////////////////////////////////
		if(file_TEST.open(QFile::ReadOnly)) {
				test = false;
				while(!file_TEST.atEnd()) {
					qtTemp= file_TEST.readLine();
					if(qtTemp.startsWith("dns=dnsmasq"))
						test = true;
				}
			file_TEST.close();
			qtTemp.clear();
		}

		if(test && file_TEST.exists()) {
		//////////////////////////////////////////////////////////////////////////////////////////
		// 3 move - Prompts the user for permission to shut down
		// DNSMASQ as a local DNS
		//////////////////////////////////////////////////////////////////////////////////////////

		  //  qDebug() << "DNS used by NM";
			QMessageBox msgBox;
			msgBox.setText(tr("DNSMAQS уже используется"));
			msgBox.setIcon(QMessageBox::Warning);
			msgBox.setInformativeText(tr("1. Внимание! Похоже, что DNSMASQ уже используется в вашей системе, как локальный DNS. \nДля корректной работы программы необходимо отключить локальный DNS. \nПеред тем, как продолжить, пожалуйста, закройте все соединения с Интернет. \nПродолжить?"));
			msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
			msgBox.setDefaultButton(QMessageBox::Cancel);
			int ret = msgBox.exec();
			if(ret==QMessageBox::Ok) {
			//////////////////////////////////////////////////////////////////////////////////////////
			// 3 move - Turning off DNS and restart NM
			//////////////////////////////////////////////////////////////////////////////////////////
			// 1 - Turning off DNSMASQ
				//system("/etc/init.d/dnsmasq stop")
				console("/etc/init.d/dnsmasq stop", temp);
			   // qDebug() << "Stoping DNSMASQ ... " << temp ;
				strcpy(temp, "");
			// 2 - Turn off DNS
				if(file.open(QFile::ReadOnly) && fileTo.open(QFile::WriteOnly)) {
					QTextStream newFileTo(&fileTo);
				   //     qDebug() << "Files is opened";

					while(!file.atEnd()) {
						Line = file.readLine();
						if(Line.startsWith("dns=dnsmasq"))
							Line.replace("dns=dnsmasq", "#dns=dnsmasq");
						newFileTo << Line;
			  //      qDebug() << Line;
					}
					file.close();
					fileTo.close();
					// Change the file locations
					QFile::remove("/etc/NetworkManager/NetworkManager.conf");
					QFile::rename("/etc/NetworkManager/NetworkManager.conf.new", "/etc/NetworkManager/NetworkManager.conf");
					console("chmod 755 /etc/NetworkManager/NetworkManager.conf", temp);
					strcpy(temp, "");
					// 3 - restart NM
					dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());
					 LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("Program"), dateUNIXNow, tr("DNS is disable succesfully"), QString("1")));
					// console("/etc/init.d/network-manager restart", temp);
					system("/etc/init.d/network-manager restart");
				}
			} // 4
			else {
				QMessageBox msg;
				msg.setIcon(QMessageBox::Critical);
				msgBox.setText(tr("The work program can not be"));
				msgBox.setInformativeText(tr("2. To use the program must have full access to DNSMASQ."));
				msgBox.exec();
				parentWidget()->close();
			}
		} // 3
	} // 2
}

void MainWindow::CheckHostapdInitdScript() {
    //////////////////////////////////////////////////////////////////////////////////////////
    // The method eliminates the problem of start Hostapd due curved script
    //////////////////////////////////////////////////////////////////////////////////////////
    QDateTime TimeNow;
    char temp[500];
    QString dateUNIXNow;
    QFile file("/etc/init.d/hostapd");
    QFile file_TEST("/etc/init.d/hostapd");
    QFile fileTo("/etc/init.d/hostapd.new");
    QString Line;
    bool test = true;

    if(file_TEST.open(QFile::ReadOnly)) {
        while(!file_TEST.atEnd()) {
            Line = file_TEST.readLine();
            if(Line.startsWith("DAEMON_CONF=/etc/hostapd/hostapd.conf\n"))
                test = false;
        }
        Line.clear();
    }
     file_TEST.close();


   if(test) {
        if(file.open(QFile::ReadOnly) && fileTo.open(QFile::WriteOnly)) {

            QTextStream newFileTo(&fileTo);

            while(!file.atEnd()) {
                Line = file.readLine();
                if(Line.startsWith("DAEMON_CONF=\n"))
                    newFileTo << "DAEMON_CONF=/etc/hostapd/hostapd.conf\n";
                else
                newFileTo << Line;
            }

            file.close();
            fileTo.close();

            // Change the file locations
            if(QFile::remove("/etc/init.d/hostapd") && QFile::rename("/etc/init.d/hostapd.new", "/etc/init.d/hostapd")) {

                console("chmod 755 /etc/init.d/hostapd", temp);
                strcpy(temp, "");
                console("chmod +x /etc/init.d/hostapd", temp);
                strcpy(temp, "");
                dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());

                LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("Program"), dateUNIXNow, tr("Hostapd Startup Script will be edited."), QString("1")));
            } else {
                dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());
                LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("Program"), dateUNIXNow, tr("Error edit Hostapd file."), QString("2")));
            }
        } // Whether the file is opened
        else {
            dateUNIXNow = QString::number(TimeNow.currentDateTime().toTime_t());
            LogSystemAppend(QString("%1|%2|%3|%4").arg(tr("Program"), dateUNIXNow, tr("Error edit Hostapd file."), QString("2")));

            QMessageBox arg;
            arg.setText(tr("Error edit Hostapd file."));
            arg.setIcon(QMessageBox::Warning);
            arg.exec();
        } // else

    } // test
} // function

/*
void MainWindow::setVisible(bool visible)
{
    minimizeAction->setEnabled(visible);
    maximizeAction->setEnabled(!isMaximized());
    restoreAction->setEnabled(isMaximized() || !visible);
    //TODO:
    //ui->
    //QDialog::setVisible(visible);
}
*/

void MainWindow::showEvent(QShowEvent *event)
{ 
	qDebug() << "showEvent ... ";
	if (event->spontaneous()) {
        emit windowShown(true);
    }
    QMainWindow::showEvent(event);
    //emit window_loaded();
}
/*
void MainWindow::your_function()
{
    // body of function
}
*/

//system tray Icon

//close to system tray icon
void MainWindow::closeEvent(QCloseEvent *event)
{
    if (trayIcon->isVisible()) {
		//TODO: remember setting
        QMessageBox::information(this, tr("Systray"),
                                 tr("The program will keep running in the "
                                    "system tray. To terminate the program, "
                                    "choose <b>Quit</b> in the context menu "
                                    "of the system tray entry."));
        hide();
        //QT5.4 compiler error??
        event->ignore();
    }
}

//hide to system tray icon

void MainWindow::hideEvent(QHideEvent *event)
{
	qDebug() << "hideEvent ... ";
	if (event->spontaneous()) {
        emit windowShown(false);
    }
    QMainWindow::hideEvent(event);
}


void MainWindow::createActions()
{
    //following action only works on windows?
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

    //maximizeAction = new QAction(tr("Ma&ximize"), this);
    //connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));
	//connect(maximizeAction, SIGNAL(triggered()), this, SLOT(show()));

    restoreAction = new QAction(tr("&Restore"), this);
    //connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));
	connect(restoreAction, SIGNAL(triggered()), this, SLOT(show()));
	
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}

void MainWindow::createTrayIcon()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    //trayIconMenu->addAction(maximizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    //trayIcon = new QSystemTrayIcon(this);
    trayIcon = new QSystemTrayIcon(iconReturn("wifisoftap"),this);
    trayIcon->setContextMenu(trayIconMenu);

    //connect(showIconCheckBox, SIGNAL(toggled(bool)),  trayIcon, SLOT(setVisible(bool)));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
                this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
    trayIcon->show();
}

//TODO:
void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
        case QSystemTrayIcon::Trigger:
            trayIcon->showMessage("title","single click"); //後面兩個默認參數
            break;
        case QSystemTrayIcon::DoubleClick:
            trayIcon->showMessage("title","Double click"); //後面兩個默認參數
//            iconComboBox->setCurrentIndex((iconComboBox->currentIndex() + 1)
//                                          % iconComboBox->count());
            break;
        case QSystemTrayIcon::MiddleClick:
            trayIcon->showMessage("title","Middle click"); //後面兩個默認參數
//            showMessage();
            break;
        default:
            ;
    }
}
