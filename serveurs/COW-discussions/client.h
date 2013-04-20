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
#ifndef CLIENT_H
#define CLIENT_H

#include <QtCore/QCoreApplication>
#include <QtNetwork>
#include <iostream>

class Discussion;

class Client
{
    public:
        Client(QString Pseudo, QTcpSocket *Tcp);
        QString getPseudo();
        void putPseudo(QString Pseudo);
        QTcpSocket* getTcp();
        quint16 getTaille();
        void putTaille(quint16 taillee);
		QString getMessagep(int p);
		void putMessagep(QString m);
		QString getIP();
		void putIP(QString ipf);
		QString getRang();
		void putRang(QString rangf);
		int getVersion();
		void putVersion(int versionf);
		QString getLangage();
		void putLangage(QString langage);
		int getStatut();
		void putStatut(int fstatut);
		QString getOs();
		void putOs(QString os);
		void ajouter_discussion(Discussion *discussionf);
		void supprimer_discussion(Discussion *discussionf);
		QList<Discussion *> liste_discussions();

    private:
        QString m_pseudo;
        QTcpSocket *m_tcp;
        quint16 m_tailleMessage;
		QList<QString> m_messagesp;
		QString m_ip;
		QString m_rang;
		int m_version;
		int m_statut;
		QString m_langage;
		QString m_os;
		QList<Discussion *> m_discussions; //Discussions auxquelles participe le client
};

#endif // CLIENT_H
