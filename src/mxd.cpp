/***************************************************************************
 *   Copyright (C) 2005 by yunfan   *
 *   yunfan_zg@163.com   *
 *                                                                         *
 *   Thanks for Christian Nitschkowski(RP-PPPoEK project)                  *
 *   segfault_ii@web.de                                                    *
 *   Thanks for Hugo Parente Lima (KNetStats project)                      *
 *   hugo_pl@users.sourceforge.net                                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "mxd.h"
#include <qtimer.h>
#include <qtooltip.h>
#include <qcursor.h>
#include <qpainter.h>
#include <qclipboard.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kpassivepopup.h>
#include <kapplication.h>
#include <kaboutapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include "settingdialog.h"
#include "statistics.h"

#define ICONSIZE 22
#define Icon(x) KGlobal::instance()->iconLoader()->loadIcon(x, KIcon::Toolbar)

extern const char * programName;

class MxDialerTip : public QToolTip{
public:
	MxDialerTip(QWidget *parent);
protected:
	void maybeTip(const QPoint &);
};

MxDialerTip::MxDialerTip(QWidget *parent)
	: QToolTip(parent) 
{
}

void MxDialerTip::maybeTip(const QPoint &pos)
{
	if(!parentWidget()->inherits("MxDialer"))
		return;
	QRect r( ((MxDialer *)parentWidget())->rect() );
	if(!r.isValid()) 
		return;
	tip(r, ((MxDialer *)parentWidget())->updateTip());
}


/// MxDialer class

MxDialer::MxDialer(QWidget *parent, const char *name) : KSystemTray(parent, name)
{
	QString script = "rppppoek.sh";
	mInterface = "ppp0";

	KActionCollection* actionCollection = new KActionCollection(this);

	start = new KProcess;
	*start << "sudo" << "pppoe-start";

	stat = new KProcess;
	*stat << script << "status" << mInterface;

	stop = new KProcess;
	*stop << "sudo" << "pppoe-stop";

	ipFound=false;
	//setPixmap(Icon("kppp"));

	KAction* about = KStdAction::aboutApp(this, SLOT (showAbout()), actionCollection);
	conToProv = new KAction(i18n("Connect"), Icon("connect_established"), 0, 0, 0, actionCollection, 0);
	disFromProv = new KAction(i18n("Disconnect"), Icon("connect_no"), 0, 0, 0, actionCollection, 0);
	KAction* status = new KAction(i18n("Show IP-address"), Icon("info"), 0, 0, 0, actionCollection, 0);

	showConfigure = new KAction(i18n("Settings"), Icon("configure"), 0, 0, 0, actionCollection, 0);

	connect (conToProv, SIGNAL(activated()), this, SLOT(con()));
	connect (disFromProv, SIGNAL(activated()), this, SLOT(discon()));
	connect (showConfigure, SIGNAL(activated()), this, SLOT(showConfigureDialog()));
	connect (status, SIGNAL(activated()), this, SLOT(getStatus()));

	connect (stat, SIGNAL(receivedStdout(KProcess*, char*, int)), this, SLOT(receiveIP(KProcess*, char*, int)));
	connect (start, SIGNAL(processExited(KProcess*)), this, SLOT(connected()));
	connect (stop, SIGNAL(processExited(KProcess*)), this, SLOT(disconnected()));

	contextMenu()->clear();
	contextMenu()->insertTitle(Icon("mxd"), i18n("Magic xDSL Dialer"), 0);
	conToProv->plug(contextMenu(), 1);
	disFromProv->plug(contextMenu(), 2);
	contextMenu()->insertSeparator(3);
	showConfigure->plug(contextMenu(), 4);
	//status->plug(contextMenu(), 5);
	contextMenu()->insertSeparator(6);
	about->plug(contextMenu(), 7);

	mIPAddress = "0.0.0.0";
	mStatistics = 0;
	mBRx = mBTx = mPRx = mPTx = 0;
	mbConnected = true;
	mTotalBytesRx = mTotalBytesTx = mTotalPktRx = mTotalPktTx = 0;
	mPtr = 0;
	memset(mSpeedRx, 0, sizeof(double)*HISTORY_SIZE);
	memset(mSpeedTx, 0, sizeof(double)*HISTORY_SIZE);
	memset(mSpeedPRx, 0, sizeof(double)*HISTORY_SIZE);
	memset(mSpeedPTx, 0, sizeof(double)*HISTORY_SIZE);

	setTextFormat(Qt::PlainText);
	show();

	mTimer = new QTimer(this, "timer");
	connect(mTimer, SIGNAL(timeout()), this, SLOT(updateStats(void)));

	setup();
	mStatistics = new Statistics(this);
	new MxDialerTip(this);

	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if(args->isSet("start")){
		printf("start connecting...\n");
		QTimer::singleShot(200, this, SLOT(con()));
	}
	if(args->isSet("stop")){
		printf("stop connection...\n");
		QTimer::singleShot(200, this, SLOT(discon()));
	}
	args->clear();
}

MxDialer::~MxDialer()
{
	KConfig* cfg = kapp->config();
	KConfigGroupSaver groupSaver(cfg, "General");
	if(mbConnected && cfg->readBoolEntry("StopOnExit", false)){
		stop->start(KProcess::Block);
	};
}
/** Connect */
void MxDialer::con(){
	start->start(KProcess::NotifyOnExit);
	conToProv->setText(i18n("Connect") + " (" + i18n("Trying to connect") + ")");
}
/** Disconnect */
void MxDialer::discon(){
	stop->start(KProcess::NotifyOnExit);
	disFromProv->setText(i18n("Disconnect") + " (" + i18n("Trying to disconnect") + ")");
}
/** Display the IP */
void MxDialer::receiveIP(KProcess* proc, char* buf, int len){

	/** Temp variables */
	KProcess filter;
	QString msg;
	int temp;

	ipFound=true;
	msg = buf;

	/** An IP-address is at least 6 chars long but not longer than the whole string */
	for (temp=6; temp <= len; temp++){
		/** Check if a character occurs that doesn't belong to an IP-address */
		if (((buf[temp] < 48 ) || (buf[temp] > 57)) && (buf[temp] != '.'))
			/** If we found such a character, the ip-address is finished so we can */
			break; /** here */
	}
	mIPAddress = msg.remove(temp, len);
}
/** Try to get the IP */
void MxDialer::getStatus(){
	stat->start(KProcess::Block, KProcess::Stdout);
	if (!ipFound){
		KMessageBox::error(this, i18n("Couldn't find an IP-address for %1.\nPlease make sure you're connected.").arg("ppp0"));
	}else{
		ipFound=false;
	}
}
/** Executes the about-dialog. */
void MxDialer::showAbout(){
	KAboutApplication dlg(this);
	dlg.exec();	
}
/** Called when start terminates */
void MxDialer::connected(void){
	if (start->normalExit()){
		if (start->exitStatus() == 0){
			KPassivePopup::message(programName, i18n("Connected successfully"), kapp->miniIcon(), this);
			mbConnected = true;
		}else{
			KPassivePopup::message(programName, i18n("Couldn't connect"), kapp->miniIcon(), this);
			mbConnected = false;
		}
	}
	conToProv->setText(i18n("Connect"));
	getStatus();// grab ip address
}
/** Called when stop terminates */
void MxDialer::disconnected(void){
	if (stop->normalExit()){
		if (stop->exitStatus() == 0){
			KPassivePopup::message(programName, i18n("Disconnected successfully"), kapp->miniIcon(), this);
			mbConnected = false;
		}else{
			KPassivePopup::message(programName, i18n("Couldn't disconnect"), kapp->miniIcon(), this);
			mbConnected = true;
		}
	}
	disFromProv->setText(i18n("Disconnect"));
}

void MxDialer::showConfigureDialog( )
{
	SettingDialog ui;
	ui.exec();
}

void MxDialer::paintEvent( QPaintEvent* ev )
{
	QPainter paint(this);
	paint.drawPixmap(0, 0, *mCurrentIcon);
}

void MxDialer::mousePressEvent(QMouseEvent* ev)
{
	if (ev->button() == Qt::RightButton )
		contextMenu()->exec(QCursor::pos());
	else if (ev->button() == Qt::LeftButton)
		statistics();
}


void MxDialer::updateStats()
{
	// Read and parse /proc/net/dev
	FILE* fp = fopen("/proc/net/dev", "r");
	if (!fp)
	{
		mTimer->stop();
		KMessageBox::error(this, i18n("Error opening /proc/net/dev!"));
		return;
	}

	unsigned int brx, btx, prx, ptx;
	char interface[8];
	char buffer[128];

	// skip headers (why not a fseek() ?)
	fgets(buffer, sizeof(buffer), fp);
	fgets(buffer, sizeof(buffer), fp);

	bool linkok = false;
	while(fgets(buffer, sizeof(buffer), fp))
	{
		// Read statistics
		sscanf(buffer, " %[^ \t\r\n:]%*c%u%u%*u%*u%*u%*u%*u%*u%u%u%*u%*u%*u%*u%*u%*u", interface, &brx, &prx, &btx, &ptx);

		if (interface == mInterface)
		{
			QPixmap* newIcon;

			// Calcula as velocidades
			mSpeedTx[mPtr] = ((btx - mBTx)*(1000.0f/UPDATE_INTERVAL));
			mSpeedRx[mPtr] = ((brx - mBRx)*(1000.0f/UPDATE_INTERVAL));
			mSpeedPTx[mPtr] = ((ptx - mPTx)*(1000.0f/UPDATE_INTERVAL));
			mSpeedPRx[mPtr] = ((prx - mPRx)*(1000.0f/UPDATE_INTERVAL));

			if (brx == mBRx)
			{
				if (btx == mBTx )
					newIcon = &mIconNone;
				else
					newIcon = &mIconTx;
			}
			else
			{
				if (btx == mBTx )
					newIcon = &mIconRx;
				else
					newIcon = &mIconBoth;
			}

			if (newIcon != mCurrentIcon)
			{
				mCurrentIcon = newIcon;
				QWidget::update();
			}

			// Update stats
			mTotalBytesRx += brx - mBRx;
			mTotalBytesTx += btx - mBTx;
			mTotalPktRx += prx - mPRx;
			mTotalPktTx += ptx - mPTx;

			mBRx = brx;
			mBTx = btx;
			mPRx = prx;
			mPTx = ptx;

			if (!mbConnected)
			{
				mbConnected = true;
				KPassivePopup::message(programName, i18n("%1 is active").arg(mInterface), kapp->miniIcon(), this);
			}
			linkok = true;
			if (++mPtr >= HISTORY_SIZE)
				mPtr = 0;
			break;
		}
	}

	fclose(fp);

	if (!linkok && mbConnected)
	{
		mbConnected = false;
		mCurrentIcon = &mIconError;
		memset(mSpeedRx, 0, sizeof(double)*HISTORY_SIZE);
		memset(mSpeedTx, 0, sizeof(double)*HISTORY_SIZE);
		memset(mSpeedPRx, 0, sizeof(double)*HISTORY_SIZE);
		memset(mSpeedPTx, 0, sizeof(double)*HISTORY_SIZE);
		QWidget::update();
		KPassivePopup::message(programName, i18n("%1 is inactive").arg(mInterface), kapp->miniIcon(), this);
	}
	updateTip();
}

void MxDialer::statistics()
{
	if (mStatistics->isShown())
		mStatistics->accept();
	else
		mStatistics->show();
}

void MxDialer::setup()
{
	// Load Icons
	KIconLoader* loader = kapp->iconLoader();
	mIconError = loader->loadIcon("status_error.png", KIcon::Panel, ICONSIZE);
	mIconNone = loader->loadIcon("status_none.png", KIcon::Panel, ICONSIZE);
	mIconTx = loader->loadIcon("status_tx.png", KIcon::Panel, ICONSIZE);
	mIconRx =loader->loadIcon("status_rx.png", KIcon::Panel, ICONSIZE);
	mIconBoth = loader->loadIcon("status_both.png", KIcon::Panel, ICONSIZE);
	mCurrentIcon = mbConnected ? &mIconNone : &mIconError;

	mTimer->start(UPDATE_INTERVAL);
	updateStats();
	QWidget::update();
}

const QString MxDialer::updateTip( )
{
	QString tip = mInterface + " ";
	tip += mbConnected?i18n("Connected"):i18n("Disconnected");
	tip += "<br>";
	QString rx = Statistics::byteFormat( byteSpeedRx(), 1, " B" )+"/s";
	QString tx = Statistics::byteFormat( byteSpeedTx(), 1, " B" )+"/s";
	tip += (i18n("Upload speed:") + rx + "<br>");
	tip += (i18n("Download speed:") + tx);
	return tip;
}

