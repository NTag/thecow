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
#include "discussions.h"

using namespace std;

Discussion::Discussion(int fid)
{
    m_clients.clear();
    m_id = fid;
}
void Discussion::ajouter_client(Client *fclient)
{
    m_clients << fclient;
	fclient->ajouter_discussion(this);
}
void Discussion::supprimer_client(Client *fclient)
{
	cout << "d2.1" << endl;
	m_clients.removeOne(fclient);
	cout << "d2.2" << endl;
	fclient->supprimer_discussion(this);
	cout << "d2.3" << endl;
}
bool Discussion::client_existe(Client *fclient)
{
    return m_clients.contains(fclient);
}
QList<Client *> Discussion::liste_clients()
{
    return m_clients;
}
int Discussion::nombre_clients()
{
    return m_clients.count();
}
int Discussion::id()
{
    return m_id;
}
