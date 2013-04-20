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
#include <QtCore/QCoreApplication>
#include <QtNetwork>
#include <iostream>
#include "fonctions.h"
#include "client.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    std::cout << "COW-serveur" << std::endl << std::endl;
	std::cout << "Programme de chat distribué sous licence GNU/GPL par Basile Bruneau ; Programme central : serveur de chat, échange de message" << std::endl;
    std::cout << "Tentative de démarrage..." << std::endl;
    
	QString fich = QString("cow-serveur_") + QString(argv[2]);
	QTranslator translator;
	translator.load(fich);
	a.installTranslator(&translator);
    
    Serveur serveur(atoi(argv[1]));

    return a.exec();
}
