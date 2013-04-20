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
#include "fonctions.h"

Serveur::Serveur(int p)
{
    serveurt = new QTcpServer(); //Création du serveur
    
    tailleMessage = 0;
    vnb_f_win = 0;
    vnb_f_gnu = 0;
	vnb_f_mac = 0;
    
    port = p;
    db = QSqlDatabase::addDatabase("QMYSQL");
      db.setHostName("127.0.0.1");
      db.setDatabaseName("cow");
      db.setUserName("root");
      db.setPassword("bb93ncis");
      bool ok = db.open();
    if(ok)
    {
      cout << "Connexion à la base de données réussie !" << endl;
    }
    else
    {
      cout << "Erreur lors de la connexion à MySQL !" << endl;
      return;
    }
    cout << "Récupération des infos" << endl;
    

    
    QSqlQuery query;
    query.exec("SELECT version,changements,taille,code_maj FROM programmes WHERE valide='1' AND os='windows' ORDER BY version DESC LIMIT 0,1");
    while (query.next())
    {
		vch_win = query.value(1).toString();
		vpoids_win = query.value(2).toInt();
		v_win = query.value(0).toInt();
		vcode_win = query.value(3).toString();
    }
    
    QSqlQuery queryv;
    queryv.exec("SELECT fichier FROM programmes_fichiers WHERE version='" + QString::number(v_win) + "' AND os='windows' ORDER BY fichier ASC");
    while (queryv.next())
    {
		vfichiers_win << queryv.value(0).toString();
		vnb_f_win++;
    }
    
    QSqlQuery query2;
    query2.exec("SELECT version,changements,taille,code_maj FROM programmes WHERE valide='1' AND os='gnu' ORDER BY version DESC LIMIT 0,1");
    while (query2.next())
    {
		vch_gnu = query2.value(1).toString();
		vpoids_gnu = query2.value(2).toInt();
		v_gnu = query2.value(0).toInt();
		vcode_gnu = query2.value(3).toString();
    }
    
    QSqlQuery queryv2;
    queryv2.exec("SELECT fichier FROM programmes_fichiers WHERE version='" + QString::number(v_gnu) + "' AND os='gnu' ORDER BY fichier ASC");
    while (queryv2.next())
    {
		vfichiers_gnu << queryv2.value(0).toString();
		vnb_f_gnu++;
    }
	
	QSqlQuery query3;
    query3.exec("SELECT version,changements,taille,code_maj FROM programmes WHERE valide='1' AND os='mac' ORDER BY version DESC LIMIT 0,1");
    while (query3.next())
    {
		vch_mac = query3.value(1).toString();
		vpoids_mac = query3.value(2).toInt();
		v_mac = query3.value(0).toInt();
		vcode_mac = query3.value(3).toString();
    }
    
    QSqlQuery queryv3;
    queryv3.exec("SELECT fichier FROM programmes_fichiers WHERE version='" + QString::number(v_mac) + "' AND os='mac' ORDER BY fichier ASC");
    while (queryv3.next())
    {
		vfichiers_mac << queryv3.value(0).toString();
		vnb_f_mac++;
    }
	
	
     cout << "Lancement du serveur..." << endl;

    tailleMessage = 0;
    if (!serveurt->listen(QHostAddress::Any, port))
    {
        cout << "Erreur lors du démarrage : " << ch(serveurt->errorString()) << endl << endl;
        etat = false;
    }
    else
    {
        cout << "Le serveur a bien été démarré sur le port " << ch(QString::number(port)) << endl << endl;
        etat = true;
        QObject::connect(serveurt, SIGNAL(newConnection()), this, SLOT(nouvelleConnexion()));
    }
}
//Slots
void Serveur::nouvelleConnexion()
{
    QTcpSocket *nouveauClient = serveurt->nextPendingConnection();
    connect(nouveauClient, SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
    connect(nouveauClient, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    cout << "Connexion d'un client" << endl;
}

void Serveur::deconnexionClient()
{
    cout << "Déconnexion d'un client" << endl << endl;
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0) // Si par hasard on n'a pas trouvé le client à l'origine du signal, on arrête la méthode
        return;
    
    socket->deleteLater();
}
void Serveur::donneesRecues()
{
    // On détermine quel client envoie le message (recherche du QTcpSocket du client)
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0) // Si par hasard on n'a pas trouvé le client à l'origine du signal, on arrête la méthode
        return;

    // Si tout va bien, on continue : on récupère le message
    QDataStream in(socket);

    if (tailleMessage == 0) // Si on ne connaît pas encore la taille du message, on essaie de la récupérer
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16)) // On n'a pas reçu la taille du message en entier
             return;

        in >> tailleMessage; // Si on a reçu la taille du message en entier, on la récupère
    }

    // Si on connaît la taille du message, on vérifie si on a reçu le message en entier
    if (socket->bytesAvailable() < tailleMessage) // Si on n'a pas encore tout reçu, on arrête la méthode
        return;
    
    int version;
    int type;
    QString langue;
    QString os;
    in >> type;
    in >> version;
    in >> langue;
    in >> os;
    cout << "ici" << endl;
    cout << ch(os) << endl;
    cout << ch(QString::number(version)) << endl;
    if(QString(os) != QString("windows") and QString(os) != QString("linux") and QString(os) != QString("mac"))
    {
		os = "windows";
    }
    if(type==1)
    {
		envoyerA(socket,os);
    }
    tailleMessage = 0;
}
void Serveur::envoyerA(QTcpSocket *Tcp,QString os) //Envoi d'un message à un client précis
{
    // Préparation du paquet
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) 0; // On écrit 0 au début du paquet pour réserver la place pour écrire la taille
    
    if(os=="windows")
    {
		out << v_win;
		out << vfichiers_win;
		out << vnb_f_win;
		out << vch_win;
		out << vpoids_win;
		out << vcode_win;
    }
    else if(os=="linux")
    {
		out << v_gnu;
		out << vfichiers_gnu;
		out << vnb_f_gnu;
		out << vch_gnu;
		out << vpoids_gnu;
		out << vcode_gnu;
    }
	else if(os=="mac")
    {
		out << v_mac;
		out << vfichiers_mac;
		out << vnb_f_mac;
		out << vch_mac;
		out << vpoids_mac;
		out << vcode_mac;
    }

    out.device()->seek(0); // On se replace au début du paquet
    out << (quint16) (paquet.size() - sizeof(quint16)); // On écrase le 0 qu'on avait réservé par la longueur du message


    // Envoi du paquet au client
    Tcp->write(paquet);
}

//Fonctions sans rapport
const char* Serveur::ch(QString texte)
{
    return texte.toStdString().c_str();
}
