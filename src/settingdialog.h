/***************************************************************************
 *   Copyright (C) 2005 by yunfan                                          *
 *   yunfan_zg@163.com                                                     *
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

#ifndef SETTINGDIALOG_H
#define SETTINGDIALOG_H

#include "mxdconfigureuibase.h"
#include <qstringlist.h>

class KProcess;
class QCloseEvent;
class PPPoEConfFile;

class SettingDialog : public MxDConfigureUIBase
{
	Q_OBJECT
public:
	SettingDialog(QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
	~SettingDialog();

private:
	bool m_IsConfigChanged;
	bool m_AutoStartSet;
	bool m_AutoStopOnExitSet;

	void getEthernetCards();
	void closeEvent( QCloseEvent *e);

	void updateDialog(PPPoEConfFile *conf = NULL);
	PPPoEConfFile *getConfValues();

	void loadProgramConfig();
	void saveProgramConfig();
private slots:
	void slotDefaultClick();
	void slotSaveClick();
	void slotCancelClick();

	void slotConfigureChanged();
	void slotDnsChanged(int);
};

#endif //  SETTINGDIALOG_H
