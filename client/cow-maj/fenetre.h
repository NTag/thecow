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
#ifndef PREFEN_H_INCLUDED
#define PREFEN_H_INCLUDED

#include <QtGui>
#include <QtNetwork>
#include <QHttp>
#include "ui_fenetre.h"
#define OS "mac"
#define VERSION 2000
#define MAC

class Fenetre : public QWidget, private Ui::cowMaj
{
    Q_OBJECT

    public:
       Fenetre(QString flangue="en");

    private:
		void telecharge(QString fichier);
		QString versionv(int v);
		const char* ch(QString texte);

    private slots:
        void donneesRecues();
        void connecte();
        void deconnecte();
        void erreurSocket(QAbstractSocket::SocketError erreur);
		void updateDataReadProgress(int bytesRead, int totalBytes);
		void httpRequestFinished(int requestId, bool error);

    private:
        int id_request;
        QHttp *page_adresse;
        QTcpSocket *socket; // Représente le serveur
        quint16 taille;
        QString serveur_ip;
        int serveur_port;
	
		bool connectee;
		QString langue;
	
		QHttp *http;
		QFile *file;
		int httpGetId;
		bool httpRequestAborted;

		QList<QString> vfichiers;
		QString code;
		int vnb_f;
		QString vch;
		int vpoids;
		int nbv;
		QString adresse;
		int bytesDeja;
		int courantf;
		int ctbytes;
	
		int versiona;

};

#endif // PREFEN_H_INCLUDED
