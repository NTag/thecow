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
#include "client.h"

using namespace std;

class Serveur : public QObject
{
    Q_OBJECT

    public:
        Serveur(int portt);
        void envoyerA(QTcpSocket *Tcp, int type=1);
        const char* ch(QString texte); //Fonction qui retourne un char à partir d'un QString

    private:
		void lancer();
		QString mys(QString texte); //Fonction de sécurisation SQL
		long timestamp(); //Retourne le timestamp actuel
		QString versionv(int v);
		int Langue(QString langue); //Retourne le numéro de la QList en fonction de la langue

    private slots:
        void nouvelleConnexion();
        void donneesRecues();
        void deconnexionClient();

    private:
		int port;
        QTcpServer *serveurt;
        bool etat;
	
        quint16 tailleMessage;
        QList<QString> a;
        QList<QString> b;
        QList<int> c;
		QSqlDatabase db;
		QList<Client> clients;
		int point;
	
		int versionC; //numéro de la dernière version du client
		QString versionCch; //changements apportés
		int versionCp; //taille du téléchargement
	
		QList< QList<QString> > textes; //Traductions
};

#endif // FONCTIONS_H
