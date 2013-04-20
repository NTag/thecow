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
#ifndef FONCTIONS_H
#define FONCTIONS_H
#include <QtCore/QCoreApplication>
#include <QtNetwork>
#include <QtSql>
#include <iostream>
#include <math.h>

using namespace std;

class Serveur : public QObject
{
    Q_OBJECT

    public:
        Serveur(int p);
        void envoyerA(QTcpSocket *Tcp,QString os);
        const char* ch(QString texte); //Fonction qui retourne un char à partir d'un QString

    private slots:
        void nouvelleConnexion();
        void deconnexionClient();
		void donneesRecues();

    private:
		int port;
        QTcpServer *serveurt;
        bool etat;
        quint16 tailleMessage;
		QSqlDatabase db;
		//Windows
		QList<QString> vfichiers_win;
		int vnb_f_win;
		QString vch_win;
		int vpoids_win;
		int v_win;
		QString vcode_win;
		//GNU
		QList<QString> vfichiers_gnu;
		int vnb_f_gnu;
		QString vch_gnu;
		int vpoids_gnu;
		int v_gnu;
		QString vcode_gnu;
		//MacOSX
		QList<QString> vfichiers_mac;
		int vnb_f_mac;
		QString vch_mac;
		int vpoids_mac;
		int v_mac;
		QString vcode_mac;
};

#endif // FONCTIONS_H
