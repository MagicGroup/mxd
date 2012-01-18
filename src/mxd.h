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


#ifndef MXDIRALER_H
#define MXDIRALER_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <qpixmap.h>
#include <kapp.h>
#include <ksystemtray.h>
#include <kprocess.h>
#include <kdialog.h>
#include <kaction.h>

/**
 * @short A KSystemTray based Main class
 * @author yunfan <yunfan_zg@163.com>
 * @version 0.2
 */

#define HISTORY_SIZE 5
#define UPDATE_INTERVAL 300

class QTimer;
class QMouseEvent;
class QPaintEvent;
class Statistics;

class MxDialer : public KSystemTray
{
	Q_OBJECT
public:
	/** construtor */
	MxDialer(QWidget* parent=0, const char *name=0);
	/** destructor */
	~MxDialer();

	///	The current monitored network interface
	inline const QString& interface() const;
	///	The current Update Interval in miliseconds
	inline int updateInterval() const;

	/// Total of bytes receiveds
	inline unsigned int totalBytesRx() const;
	/// Total of bytes transmitted
	inline unsigned int totalBytesTx() const;
	/// Total of packets receiveds
	inline unsigned int totalPktRx() const;
	/// Total of packets transmitted
	inline unsigned int totalPktTx() const;
	/// RX Speed in bytes per second
	inline double byteSpeedRx() const;
	/// TX Speed in bytes per second
	inline double byteSpeedTx() const;
	/// RX Speed in packets per second
	inline double pktSpeedRx() const;
	/// TX Speed in packets per second
	inline double pktSpeedTx() const;

	const QString updateTip();
private:
	KProcess* start;
	KProcess* stat;
	KProcess* stop;
	bool ipFound;
	QString mIPAddress;
	KDialog* aboutDialog;
	KAction* conToProv;
	KAction* disFromProv;
	KAction* showConfigure;

	Statistics* mStatistics;
	QString mInterface;

	QPixmap* mCurrentIcon;
	QPixmap mIconError, mIconNone, mIconTx, mIconRx, mIconBoth;
	QTimer* mTimer;
	unsigned int mBRx, mBTx, mPRx, mPTx;
	unsigned int mTotalBytesRx, mTotalBytesTx, mTotalPktRx, mTotalPktTx;
	double mSpeedRx[HISTORY_SIZE], mSpeedTx[HISTORY_SIZE];
	double mSpeedPRx[HISTORY_SIZE], mSpeedPTx[HISTORY_SIZE];

	int mPtr;

	/// is connected?
	bool mbConnected;
	void setup();
	inline double calcSpeed(const double* field) const;
protected:
	void mousePressEvent( QMouseEvent* ev );
	void paintEvent( QPaintEvent* ev );

private slots: // Private slots
	/** Disconnect */
	void discon();
	/** Connect */
	void con();
	/** Show configure dialog */
	void showConfigureDialog();
	/** Display the IP */
	void receiveIP(KProcess* proc, char* buf, int len);
	/** Try to get the IP */
	void getStatus();
	/** Show the about-dialog */
	void showAbout();
	/** Called when start terminates */
	void connected(void);
	/** Called when stop terminates */
	void disconnected(void);

	/// Called by the timer to update statistics
	void updateStats();
	/// Display the statistics dialog box
	void statistics();
	friend class Statistics;
};

double MxDialer::calcSpeed(const double* field) const
{
	double total = 0.0;
	for (int i = 0; i < HISTORY_SIZE; ++i)
		total += field[i];
	return total/HISTORY_SIZE;
}

const QString& MxDialer::interface() const
{
	return mInterface;
}

int MxDialer::updateInterval() const
{
	return UPDATE_INTERVAL;
}

unsigned int MxDialer::totalBytesRx() const
{
	return mTotalBytesRx;
}

unsigned int MxDialer::totalBytesTx() const
{
	return mTotalBytesTx;
}

unsigned int MxDialer::totalPktRx() const
{
	return mTotalPktRx;
}

unsigned int MxDialer::totalPktTx() const
{
	return mTotalPktTx;
}

double MxDialer::byteSpeedRx() const
{
	return calcSpeed(mSpeedRx);
}

double MxDialer::byteSpeedTx() const
{
	return calcSpeed(mSpeedTx);
}

double MxDialer::pktSpeedRx() const
{
	return calcSpeed(mSpeedPRx);
}

double MxDialer::pktSpeedTx() const
{
	return calcSpeed(mSpeedPTx);
}


#endif  // MXDIRALER_H



