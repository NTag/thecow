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

class Client
{
    public:
        Client(QTcpSocket *Tcp, QHostAddress ipf);
        QTcpSocket* getTcp();
        quint16 getTaille();
        void putTaille(quint16 taillee);
		int getVersion();
		void putVersion(int versionf);
		void putLangage(QString flangage);
		QString getLangage();
		QHostAddress getIp();
		QString getOs();
        void putOs(QString osf);

    private:
        QTcpSocket *m_tcp;
        quint16 m_tailleMessage;
		QString m_langage;
		QHostAddress m_ip;
		QString m_os;
		int m_version;
};

#endif // CLIENT_H
