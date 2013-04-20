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
#ifndef DISCUSSIONS_H
#define DISCUSSIONS_H

#include <QtCore/QCoreApplication>
#include <QtNetwork>
#include <iostream>
#include "client.h"

class Discussion
{
    public:
        Discussion(int fid);
        void ajouter_client(Client *fclient);
        void supprimer_client(Client *fclient);
        bool client_existe(Client *fclient);
        QList<Client *> liste_clients();
        int nombre_clients();
        int id();

    private:
        QList<Client *> m_clients;
        int m_id;
        

};

#endif // DISCUSSIONS_H
