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
#include "client.h"

Client::Client(QTcpSocket *Tcp,QHostAddress ipf)
{
    m_tcp = Tcp;
    m_tailleMessage = 0;
    m_langage = "fr";
    m_ip = ipf;
}
QTcpSocket* Client::getTcp()
{
    return m_tcp;
}
quint16 Client::getTaille()
{
    return m_tailleMessage;
}
void Client::putTaille(quint16 taillee)
{
    m_tailleMessage = taillee;
}
int Client::getVersion()
{
    return m_version;
}
void Client::putVersion(int versionf)
{
    m_version = versionf;
}
void Client::putLangage(QString flangage)
{
    m_langage = flangage;
}
QString Client::getLangage()
{
    return m_langage;
}
QHostAddress Client::getIp()
{
    return m_ip;
}
QString Client::getOs()
{
    return m_os;
}
void Client::putOs(QString osf)
{
    m_os = osf;
}
