/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Die Jun 18 23:04:03 CEST 2002
    copyright            : (C) 2002 by Christian Nitschkowski
    email                : segfault_ii@web.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kuniqueapplication.h>
#include <kaboutapplication.h>

#include "mxd.h"
#include "settingdialog.h"

const char* programName = I18N_NOOP( "Magic xDSL Dialer" );
static const char *description =
	I18N_NOOP("Magic xDSL Dialer\n\nA convenient tool to handle \"Roaring Penguin PPPoE\" "
		"\nextending from \"RP-PPPoEK\" project by Christian Nitschkowski.");


static KCmdLineOptions options[] =
{
	{ "setup", I18N_NOOP("Pop up setup window only."), 0},
	{ "start", I18N_NOOP("Connect xDSL directly."), 0},
	{ "stop", I18N_NOOP("Stop xDSL connection."), 0}
};

int main(int argc, char *argv[])
{
	KAboutData aboutData( "mxd", programName,
		VERSION, description, KAboutData::License_GPL,
		"(c) 2005, yunfan", 0, 0, "yunfan_zg@163.com");
	aboutData.addAuthor("yunfan", 0, "yunfan_zg@163.com");
	aboutData.addCredit("Christian Nitschkowski", I18N_NOOP("Author of \"Roaring Penguin PPPoE Kontrol\"."), "segfault_ii@web.de");
	aboutData.addCredit( "Hugo Parente Lima", I18N_NOOP("Author of \"KNetStats\"."), "hugo_pl@users.sourceforge.net" );
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options );


	KUniqueApplication a;

	QObject::connect(&a, SIGNAL(lastWindowClosed()), &a, SLOT(quit()));
	KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
	if(args->isSet("setup")){
		SettingDialog *dialog = new SettingDialog();
		a.setMainWidget(dialog);
		dialog->show();
	}else{
		MxDialer *mxd = new MxDialer();
		a.setMainWidget(mxd);
		mxd->show();
	}
	args->clear();
	return a.exec();
}
