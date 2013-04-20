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

Client::Client(QString Pseudo,QTcpSocket *Tcp)
{
    m_pseudo = Pseudo;
    m_tcp = Tcp;
    m_tailleMessage = 0;
    m_rang = "membre";
    m_ip = "";
    m_version = 1000;
    m_statut = 0;
    
    m_opt_heureMessAff = false;
}
QString Client::getPseudo()
{
    return m_pseudo;
}
void Client::putPseudo(QString Pseudo)
{
    m_pseudo = Pseudo;
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
QString Client::getMessagep(int p)
{
    if(m_messagesp.size()<=p)
    {
		return "";
    }
    else
    {
		return m_messagesp.at(p);
    }
}
void Client::putMessagep(QString m)
{
    m_messagesp.insert(0,m);
    if(m_messagesp.size()>10)
	m_messagesp.removeAt(10);
}
QString Client::getMessagemp(int p)
{
    if(m_messagesmp.size()<=p)
    {
		return "";
    }
    else
    {
		return m_messagesp.at(p);
    }
}
void Client::putMessagemp(QString m)
{
    m_messagesmp.insert(0,m);
    if(m_messagesmp.size()>10)
	m_messagesmp.removeAt(10);
}
QString Client::getIP()
{
    return m_ip;
}
void Client::putIP(QString ipf)
{
    m_ip = ipf;
}
QString Client::getRang()
{
    return m_rang;
}
void Client::putRang(QString rangf)
{
    m_rang = rangf;
}
int Client::getVersion()
{
    return m_version;
}
void Client::putVersion(int versionf)
{
    m_version = versionf;
}
int Client::getStatut()
{
    return m_statut;
}
void Client::putStatut(int statutf)
{
    if(statutf==0 or statutf==1 or statutf==2)
	m_statut = statutf;
    else
	m_statut = 0;
}
QString Client::getLangage()
{
    return m_langage;
}
void Client::putLangage(QString langagef)
{
    m_langage = langagef;
}
QString Client::getOs()
{
    return m_os;
}
void Client::putOs(QString osf)
{
    m_os = osf;
}

//Options
bool Client::getOptHeureMessAff()
{
    return m_opt_heureMessAff;
}
void Client::putOptHeureMessAff(bool afff)
{
    m_opt_heureMessAff = afff;
}

