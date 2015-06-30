#ifndef _CheckProcess_
#define _CheckProcess_

#include <stdio.h>
#include <QString>
#include <QStringList>
#include <QFile>
#include <QDir>
#include <QDebug>
#include <QRegExp>

class CheckProcess {
	private:
		int pidproc;
		int Doing;
		QString name;
		bool Status;
		bool debugThisClass;

	public:
		CheckProcess() { pidproc = -1; Status = false; Doing = -1; }
		CheckProcess(int a)
		{   pidproc = a; Status = false;  Doing = -1;  }
		CheckProcess(QString t)
		{   name = t; Status = false; pidproc = -1;  Doing = -1; }
		CheckProcess(int b, QString c)
		{ name = c;  Doing = b; Status = false; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
		void GetWhichPID() {
			QString temp = QString::number(pidproc);
			Status = false;
			QString fileLine;
			QRegExp nameReg(name);
			QRegExp exitState("Name");

			QFile StateFile(QString("/proc/%1/status").arg(temp));

			if(debugThisClass) qDebug() << "FILE : " << QString("/proc/%1/status").arg(temp);

			if(StateFile.open(QFile::ReadOnly)) { // 0
				if(debugThisClass) qDebug() << "open file";

			 // Now we get a line dividing it, reject spaces and see the process name
			// if the name coincided with the one stored in our class, then we return true

				while(StateFile.atEnd()) { // 1.0
					fileLine = StateFile.readLine(1024);

					if(fileLine.contains(nameReg)>0)
						Status = true;

					if(fileLine.contains(exitState)>0)
						break;
				} // 1.0

			} // 0
			StateFile.close();
		} // GetWhichPID
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
		void getPIDWhichName() {
			QString path;
			QString PIDString;
		/////////////////////////////////////////////////////
			if(name == "hostapd") {
				path = "/var/run/hostapd.pid";
				if(debugThisClass) qDebug() << "check Hostapd";
			}
			else  {
				path = "/var/run/dnsmasq/dnsmasq.pid";
				if(debugThisClass) qDebug() << "check DNSMAQ";
			}
		///////////////////////////////////////////////////
			QFile FileToRead(path);
			if(FileToRead.open(QFile::ReadOnly)) {
			PIDString = FileToRead.readAll();
			pidproc = PIDString.toInt();
			}
			else {
				Status = false;
				pidproc = -1;
			}
			FileToRead.close();
			if(debugThisClass) qDebug() << "PID  = " << pidproc;
		}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool getStatusIPForwarding() {
	// Simple and CURVE !!! !!! method that returns the status of the forwarding system
			bool t = false;
			int p = 0;
			QString test;

			QFile ipforwarding("/proc/sys/net/ipv4/ip_forward");
			if(ipforwarding.open(QFile::ReadOnly)) {

				test = ipforwarding.readAll();
				p = test.toInt();
				if(p==1) t = true;

			}

			ipforwarding.close();
			return t;
		}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool getStatusInterface() {
	// The method returns the status of web interface at any given time
			QFile PriceInterface("/proc/net/dev_mcast");
			QString line;
			QStringList listArray;
			QStringList listArrayLine;
			int i, l;

			if(PriceInterface.open(QFile::ReadOnly)) {
				QTextStream in(&PriceInterface);
				line = in.read(102400);
				listArray = line.split("\n");
				for(l=0;l<listArray.count();l++) {
					line = listArray.at(l);
					listArrayLine = line.split(" ");
					for(i=0;i<listArrayLine.count();i++)  {
						if(listArrayLine.at(i)==name) {
							   Status = true;
							   break;
						}
					}
					if(Status)
							break;
				}
			}
			PriceInterface.close();
			return Status;
		}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////
		bool init() {
		// ИInitializing the class - check, we have (name or PID) and get the status of
			// Doing > -1 is the name, то We do:
			// Doing  = 1 - check interface
			// Doing = 2 - check IP forward
			// Doing == -1, then see if there is a PID, then we get the status on it
			// if there is a name, but there is no PID, then we get PID pid-file and check the status there
			debugThisClass = false;
			Status = false;

			if(Doing> -1) {
				if(Doing==1) {
					  Status = getStatusInterface();
					  if(debugThisClass) qDebug() << "check interface...";
				}
				if(Doing==2) {
					Status = getStatusIPForwarding();
					if(debugThisClass) qDebug() << "check IPForwarding...";
				}
			}
			else {
					if(pidproc>0) {
						GetWhichPID();
						if(debugThisClass) qDebug() << "We get the status of the по PID... ";
						}
						else if(name.length() > 0) {
								getPIDWhichName();
								if(debugThisClass) qDebug() << "We get the PID name (" << name << ")";
								if(pidproc>0) { GetWhichPID();
								 if(debugThisClass) qDebug() << "And now we get the status of PID";
								}
					}
			}

			return Status;
		}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////

};
#endif
