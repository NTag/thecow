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
#include "client.h"
#include "discussions.h"
#include "fonctions.h"

using namespace std;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    cout << "COW-discussions" << endl << endl;
	cout << "Programme de chat distribué sous licence GNU/GPL par Basile Bruneau ; serveur permettant les discussions privées (par onglets sur cow-client>=1.9)" << endl << endl;
    cout << "Tentative de démarrage..." << endl;

    Serveur serveur(10050);

    return a.exec();
}
