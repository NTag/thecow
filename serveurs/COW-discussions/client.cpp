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
#include "discussions.h"

Client::Client(QString Pseudo,QTcpSocket *Tcp)
{
    m_pseudo = Pseudo;
    m_tcp = Tcp;
    m_tailleMessage = 0;
    m_rang = "membre";
    m_ip = "";
    m_version = 1000;
    m_langage = "fr";
    m_os = "windows";
	m_statut = 0;
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
QString Client::getLangage()
{
    return m_langage;
}
void Client::putLangage(QString langage)
{
    m_langage = langage;
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
QString Client::getOs()
{
    return m_os;
}
void Client::putOs(QString osf)
{
    m_os = osf;
}

void Client::ajouter_discussion(Discussion *discussionf)
{
	m_discussions << discussionf;
}
void Client::supprimer_discussion(Discussion *discussionf)
{
	m_discussions.removeOne(discussionf);
} 
QList<Discussion*> Client::liste_discussions()
{
	return m_discussions;
}
