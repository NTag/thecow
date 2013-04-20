/*
 Copyright (C) 2010 Bruneau Basile
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License along
 with this program; if not, write to the Free Software Foundation, Inc.,
 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#include <QtSingleApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include "fenetre.h"

int main(int argc, char* argv[])
{
    //=====Empeche de lancer COW plusieurs fois
    QtSingleApplication app(argc, argv);

    if (app.isRunning())
	return 0;
    //==========
    
    QDir::setCurrent(QCoreApplication::applicationDirPath());
	
	//=====Informations
	app.setApplicationName("COW Chat Over the World");
	app.setApplicationVersion("2.0.0.0");
	app.setOrganizationDomain("the-cow.org");
	app.setOrganizationName("Basile Bruneau");
	//==========

	//=====Traductions
    QString locale = QLocale::system().name();
    QTranslator trans;
    trans.load(QString(":/translations/qt_") + locale);
    app.installTranslator(&trans);

    QTranslator translator;
    translator.load(QString(":/translations/cow-maj_") + locale.section('_', 0, 0));
    app.installTranslator(&translator);
	//==========

    Fenetre fen(locale.section('_', 0, 0));
    
    app.setActivationWindow(&fen); //Empeche de lancer COW plusieurs fois
    fen.show();

    return app.exec();
}
