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

class Serveur : public QObject
{
    Q_OBJECT

    public:
        Serveur(int port);
        void envoyerATous(const QString &message,int type=1,int c=0,int fpoint=-1);
		void envoyerA(const QString &message,QTcpSocket *Tcp, bool simple = false,int type=1,int fstatut=0,bool archiver=true);
		QString statut(int statutf,bool lower=false);
        const char* ch(QString texte); //Fonction qui retourne un char à partir d'un QString
		QString mys(QString texte); //Fonction de sécurisation SQL
		long timestamp(); //Retourne le timestamp actuel
		QString zero(int numero); //Ajoute un zéro si nécessaire au chiffre
        void fnb(bool plus,bool general=false, bool eff=false);

    private slots:
        void nouvelleConnexion();
        void donneesRecues();
        void deconnexionClient();

    private:
        int pport;
        QTcpServer *serveurt;
        bool etat;
        QList<Client> clients;
        int point;
        int nb;
        quint16 tailleMessage;
        QString messagee;
        QSqlDatabase db;
		QString messageBienvenue;
		QString messagePub;
		int versionClient;
		QList<QString> dernMess;
		QStringList statuts;
		QStringList ipban;
	
		QList<QStringList> msgd;
};

#endif // FONCTIONS_H
