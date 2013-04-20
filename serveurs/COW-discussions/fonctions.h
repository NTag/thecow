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
#include "client.h"
#include "discussions.h"

class Serveur : public QObject
{
    Q_OBJECT

    public:
        Serveur(int port=10050);
		void envoyerATous(Discussion *discussion,const QString &message,int type=1,int fpoint=-1);
		void envoyerA(Discussion *discussion,const QString &message,QTcpSocket *Tcp, bool simple = false,int type=1,int fdiscussion = -1);
        const char* ch(QString texte); //Fonction qui retourne un char à partir d'un QString
		QString mys(QString texte); //Fonction de sécurisation SQL
		long timestamp(); //Retourne le timestamp actuel
		QString zero(int numero); //Ajoute un zéro si nécessaire au chiffre

    private slots:
        void nouvelleConnexion();
        void donneesRecues();
        void deconnexionClient();

    private:
        QTcpServer *serveurt;
        bool etat;
        QList<Client> clients; //clients connectés
		QList<Discussion> discussions; //discussions en cours
        int point;
        QString messagee;
        QSqlDatabase db;
		int versionClient;
		int id;
};

#endif // FONCTIONS_H
