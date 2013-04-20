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

using namespace std;

Serveur::Serveur(int port)
{
	cout << "Lancement..." << endl;
    serveurt = new QTcpServer(); //Cr�ation du serveur

    versionClient = 1900;
    id = 1;
    
    if (!serveurt->listen(QHostAddress::Any, port))
    {
        cout << "Erreur lors du d�marrage : " << ch(serveurt->errorString()) << endl;
        etat = false;
	return;
    }
    else
    {
        cout << "Le serveur a bien �t� d�marr� sur le port " << ch(QString::number(port)) << endl;
        etat = true;
        QObject::connect(serveurt, SIGNAL(newConnection()), this, SLOT(nouvelleConnexion()));
    }
    
    //BDD
    db = QSqlDatabase::addDatabase("QMYSQL");
        db.setHostName("127.0.0.1");
        db.setDatabaseName("NOM_DE_LA_BDD");
        db.setUserName("UTILISATEUR_DE_LA_BDD");
        db.setPassword("MOT_DE_PASSE_DE_LA_BDD");
    bool ok = db.open();
    if(ok)
    {
		cout << "Connexion � la base de donn�es r�ussie !" << endl;
    }
    else
    {
		cout << "Erreur lors de la connexion � MySQL !" << endl;
		return;
    }
}

//Slots
void Serveur::nouvelleConnexion()
{
    QTcpSocket *nouveauClient = serveurt->nextPendingConnection();
    Client nouveau("",nouveauClient);
    clients << nouveau;
    connect(nouveauClient, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(nouveauClient, SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
    cout << "Connexion d'un nouveau client" << endl;
}

void Serveur::deconnexionClient()
{
	cout << "deco" << endl;
    // On d�termine quel client se d�connecte
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0) // Si par hasard on n'a pas trouv� le client � l'origine du signal, on arr�te la m�thode
        return;

    for (int i = 0; i < clients.size(); i++) //Boucle pour trouver le num�ro correspondant au socket
    {
        if(clients[i].getTcp()==socket)
        {
            point = i;
        }
    }
    
	QList<Discussion *> ldis = clients[point].liste_discussions();
	for(int j=0;j<ldis.size();j++)
	{
		cout << "d.1" << endl;
		Discussion *discussionsuppr = ldis[j];
		cout << "d.2" << endl;
		discussionsuppr->supprimer_client(&clients[point]); //On supprime le client de la conversation
		cout << "d.3" << endl;
		envoyerATous(discussionsuppr, "<em>" + clients[point].getPseudo() + " a quitt� la conversation</em>", 3);
		
		if(discussionsuppr->nombre_clients()==0)
		{
			for(int k=0;k<discussions.size();k++)
			{
				if(discussions[k].id()==discussionsuppr->id()) //Si la conversation est vide on supprime
				{
					discussions.removeAt(k);
					return;
				}
			}
		}
	}
	
	clients.removeAt(point); //On supprime le client
	
	cout << "D�connexion d'un client" << endl;
	
    socket->deleteLater();
}

void Serveur::donneesRecues()
{
	cout << "donnees" << endl;
    // On d�termine quel client envoie le message (recherche du QTcpSocket du client)
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0) // Si par hasard on n'a pas trouv� le client � l'origine du signal, on arr�te la m�thode
        return;

    // Si tout va bien, on continue : on r�cup�re le message
    QDataStream in(socket);

    for (int i = 0; i < clients.size(); i++) //Boucle pour trouver le num�ro correspondant au socket
    {
        if(clients[i].getTcp()==socket)
        {
            point = i;
        }
    }

    if (clients[point].getTaille() == 0) // Si on ne conna�t pas encore la taille du message, on essaie de la r�cup�rer
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16)) // On n'a pas re�u la taille du message en entier
             return;

        quint16 tailleMessage;
        in >> tailleMessage; // Si on a re�u la taille du message en entier, on la r�cup�re
        clients[point].putTaille(tailleMessage);
        //tailleMessage = 0;
        ~tailleMessage;
    }

    // Si on conna�t la taille du message, on v�rifie si on a re�u le message en entier
    if (socket->bytesAvailable() < clients[point].getTaille()) // Si on n'a pas encore tout re�u, on arr�te la m�thode
        return;

    int type;
    in >> type; //On r�cup�re le type
	
	if(type==1) //Message : message, discussion
	{
		QString rmessage;
		int rdiscussion;
		in >> rmessage;
		in >> rdiscussion;
		
		if(clients[point].getPseudo()=="") //On v�rifie que les informations nous ont d�j� �t� envoy�es
		{
			envoyerA(0,"Vous ne pouvez pas envoyer de message tant que vous n'avez pas communiquer les informations de base : pseudo, version du client, langage, os.",clients[point].getTcp(),false,1);
			clients[point].putTaille(0);
			return;
		}
		Discussion *idiscussion = new Discussion(-1);
		for(int i=0;i<discussions.size();i++)
		{
			if(discussions[i].id()==rdiscussion)
				idiscussion = &discussions[i];
		}
		
		if(idiscussion->id()==-1 or (idiscussion->id()>-1 and !idiscussion->client_existe(&clients[point])))
		{
			envoyerA(0,"Vous n'apartenez pas � la discussion indiqu�e, ou elle n'existe pas ou plus.",clients[point].getTcp(),false,-1,rdiscussion);
			clients[point].putTaille(0);
			return;
		}	
			
		//Protection contre les message vides
		QString rmessagev = rmessage;
		QRegExp esp(" ");
		rmessagev.replace(esp,"");
		if(rmessagev=="")
		{
			envoyerA(idiscussion,tr("Les messages vides sont interdits !"),clients[point].getTcp(),false);
			clients[point].putTaille(0);
			return;
		}
		
		//On coupe les messages trop longs
		rmessage.truncate(500);
		
		//=====Mise en forme
	    //S�curit�
	    //On remplace les caract�res xHTML par leur correspondance
	    QRegExp rx("<");
	    QRegExp rx2(">");
	    rmessage.replace(rx,"&lt;");
	    rmessage.replace(rx2,"&gt;");
	    
	    //Code
	    QRegExp cliens("((https?)|ftp|ssh|sftp)://([a-zA-Z0-9._/&%\\?=\\#\\+-]+)");
	    rmessage.replace(cliens,"<a title='Cliquez sur ce lien � vos risques et p�rils' href='\\1://\\3'>\\1://\\3</a>");
		
	    //Smileys
	    QRegExp sa(":\\)");
	    QRegExp sb(";\\)");
	    QRegExp sc(":\\(");
	    QRegExp sd("\\^\\^");
	    QRegExp se(":euh:");
	    QRegExp sf(":D");
	    QRegExp sg(":p");
	    QRegExp sh(":lol:");
	    QRegExp si(":oh:");
	    QRegExp sj("putain");
	    QRegExp sjj(":-o");
	    QRegExp sk(":@");
	    QRegExp sl("oO");
	    rmessage.replace(sa,"<img src=':/smileys/smile.png' alt=':)' />");
	    rmessage.replace(sb,"<img src=':/smileys/clin.png' alt=';)' />");
	    rmessage.replace(sc,"<img src=':/smileys/triste.png' alt=':(' />");
	    rmessage.replace(sd,"<img src=':/smileys/hihi.png' alt='^^' />");
	    rmessage.replace(se,"<img src=':/smileys/unsure.png' alt=':euh:' />");
	    rmessage.replace(sf,"<img src=':/smileys/heureux.png' alt=':D' />");
	    rmessage.replace(sg,"<img src=':/smileys/langue.png' alt=':p' />");
	    rmessage.replace(sh,"<img src=':/smileys/rire.png' alt=':lol:' />");
	    rmessage.replace(si,"<img src=':/smileys/huh.png' alt=':oh:' />");
	    rmessage.replace(sj,"<img src=':/smileys/siffle.png' alt=':-o' />");
	    rmessage.replace(sjj,"<img src=':/smileys/siffle.png' alt=':-o' />");
	    rmessage.replace(sk,"<img src=':/smileys/mechant.png' alt=':@' />");
	    rmessage.replace(sl,"<img src=':/smileys/blink.png' alt='oO' />");
		//==========
		
		QString MeAEn = ""; //Message A Envoyer
		
		//=====Commandes
		QRegExp commande_me("^/me (.+)$");
		QRegExp commande_away("^/away( (.+))?$");
		QRegExp commande_back("^/back$");
		QRegExp commande_msg("^/msg (\\S+) (.+)$");
		QRegExp commande_help("^/help$");
		QRegExp commande_kick("^/kick (\\S+)( (.+))?$");
		//QRegExp commande_ban("^/kick (\\S+)( (.+))?$");
		
		if(commande_me.indexIn(rmessage)!=-1) //Il s'agit de la commande /me
		{
			MeAEn = "<em><strong>" + clients[point].getPseudo() + "</strong> " + commande_me.cap(1) + "</em>";
		}
		else if(commande_away.indexIn(rmessage)!=-1) //Il s'agit de la commande /away
		{
			if(clients[point].getStatut()==2) //Absent
			{
				envoyerA(idiscussion,"Vous �tes d�j� absent(e) !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
			clients[point].putStatut(2);
			QString away = "est absent(e)";
			if(commande_away.cap(2)!="")
				away = "est absent(e) (" + commande_away.cap(2) + ")";
			
			MeAEn = "<em><strong>" + clients[point].getPseudo() + "</strong> " + away + "</em>";
			envoyerA(idiscussion,MeAEn,clients[point].getTcp(),true,6,clients[point].getStatut());
			envoyerATous(idiscussion,MeAEn,3,point);
			clients[point].putTaille(0);
			return;
		}
		else if(commande_back.indexIn(rmessage)!=-1) //Il s'agit de la commande /back
		{
			if(clients[point].getStatut()==0) //En Ligne
			{
				envoyerA(idiscussion,"Vous n'�tes pas absent(e) !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
			clients[point].putStatut(0);
			MeAEn = "<em><strong>" + clients[point].getPseudo() + "</strong> est revenu(e)</em>";
			envoyerA(idiscussion,MeAEn,clients[point].getTcp(),true,6,clients[point].getStatut());
			envoyerATous(idiscussion,MeAEn,3,point);
			clients[point].putTaille(0);
			return;
		}
		else if(commande_msg.indexIn(rmessage)!=-1) //Il s'agit de la commande /msg
		{
			QString PseudoAEn = commande_msg.cap(1);
			QString MessageAEn = commande_msg.cap(2);
			//On trouve le socket qui correspond au pseudo, et on v�rifie si le pseudo existe
			int pointAEn = -1;
			for (int i = 0; i < clients.size(); i++)
			{
				if(clients[i].getPseudo()==PseudoAEn or clients[i].getPseudo()==PseudoAEn + "[ADMIN]" or clients[i].getPseudo()==PseudoAEn + "[MODO]" or clients[i].getPseudo()==PseudoAEn + "[B�TA]")
				{
					pointAEn = i;
				}
			}
			if(pointAEn>-1)
			{
				MeAEn = "<em>Message priv� de </em><strong>" + clients[point].getPseudo() + "</strong><em> : </em>" + MessageAEn;
				QString MeAEn2 = "<em>Message priv� � </em><strong>" + clients[pointAEn].getPseudo() + "</strong><em> : </em>" + MessageAEn;
				if(clients[point].getPseudo()!=clients[pointAEn].getPseudo())
				{
					envoyerA(idiscussion,MeAEn,clients[pointAEn].getTcp(),true,1);
					envoyerA(idiscussion,MeAEn2,clients[point].getTcp(),true,1);
				}
				else
				{
					envoyerA(idiscussion,MeAEn,clients[pointAEn].getTcp(),true,1);
				}
				clients[point].putTaille(0);
				return;
			}
			else
			{
				envoyerA(idiscussion,"Le pseudo indiqu� ne correspond � aucun utilisateur connect� !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_help.indexIn(rmessage)!=-1) //Il s'agit de la commande /help
		{
			MeAEn = "<font color='gray'><hr /><strong>Commandes disponibles</strong><ul><li><strong>/me [MESSAGE]</strong> : affiche un message vous concernant<br />Exemple : /me adore COW<br />Affiche : <em><strong>pseudo</strong> adore cow</em></li><li><strong>/away [RAISON]</strong> : indique que vous �tes absent(e) pour la raison indiqu�e (qui n'est pas obligatoire, vous pouvez vous contener de <strong>/away</strong>)<br />Exemple : /away<br />Affiche : <em><strong>pseudo</strong> est absent(e)</em><br />Exemple 2 : /away pause caf� de 5 minutes !<br />Affiche : <em><strong>pseudo</strong> est absent(e) (pause caf� de 5 minutes !)</em></li><li><strong>/back</strong> : contraire � <strong>/away</strong>, indique que vous �tes revenu(e)<br />Exemple : /back<br />Affiche : <em><strong>pseudo</strong> est revenu(e)</li><li><strong>/msg [PSEUDO] [MESSAGE]</strong> : envoie un message priv� [MESSAGE] � [PSEUDO]<br />Exemple : /msg Jean tu as bien envoy� l'email ?<br />Affichage : <em>Message priv� de </em><strong>Marc</strong><em> : </em>tu as bien envoy� l'email ?</li><li><strong>/help</strong> : affiche les commandes disponibles et leur utilisation</li></ul><hr /></font><br />";
			envoyerA(idiscussion,MeAEn,clients[point].getTcp(),true,1);
			clients[point].putTaille(0);
			return;
		}
		else if(commande_kick.indexIn(rmessage)!=-1) //Il s'agit de la commande /help
		{
			if(clients[point].getRang()=="admin" or clients[point].getRang()=="modo")
			{
				QString PseudoKick = commande_kick.cap(1);
				QString RaisonKick = commande_kick.cap(3);
				int pointAEn = -1;
				for (int i = 0; i < clients.size(); i++)
				{
					if(clients[i].getPseudo()==PseudoKick or clients[i].getPseudo()==(PseudoKick + "[ADMIN]") or clients[i].getPseudo()==(PseudoKick + "[MODO]") or clients[i].getPseudo()==(PseudoKick + "[B�TA]"))
					{
						pointAEn = i;
					}
				}
				if(pointAEn>-1)
				{
					disconnect(clients[pointAEn].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
					disconnect(clients[pointAEn].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
					envoyerA(idiscussion,"Vous avez �t� d�connect� par <strong>" + clients[point].getPseudo() + "</strong>. La raison donn�e est : " + RaisonKick,clients[pointAEn].getTcp(),false,-1);
					QString pseudos = clients[pointAEn].getPseudo();
					clients.removeAt(pointAEn);
					for (int i = 0; i < clients.size(); i++) //Boucle pour trouver le num�ro correspondant au socket
					{
						if(clients[i].getTcp()==socket)
						{
							point = i;
						}
					}
					envoyerATous(idiscussion,"<em><strong>" + pseudos + "</strong> " + tr("vient de se d�connecter") + " [kick� par <strong>" + clients[point].getPseudo() + "</strong> : " + RaisonKick + "]</em>",3);
					clients[point].putTaille(0);
					return;
				}
				else
				{
					envoyerA(idiscussion,"Le pseudo indiqu� ne correspond � aucun utilisateur connect� !",clients[point].getTcp(),false,1);
					clients[point].putTaille(0);
					return;
				}
			}
			else
			{
				envoyerA(idiscussion,"Vous n'�tes pas assez haut grad� pour kicker !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else //Ce n'est pas une commande
		{
			if((rmessage=="O" or rmessage=="n") and clients[point].getMessagep(0)=="detruirelemondekillworldAZERTY") //Easter Egg [/kill world]
			{
				if(rmessage=="O")
				{
					envoyerA(idiscussion,"<em>Destruction du monde enclanch�e !</em><br /><br />>Envoie d'un email d'insultes au pr�sident des Etats-Unis...<br />>Envoie en cours...<br />...<br />...<br />...<br /><font color='red'><strong>ERROR : THEWEBMAILINUSAISWINDOWS!!IMPOSSIBLETOSENDANEMAIL!!</strong></font><br /><em>Destruction du monde annul�e</em>",clients[point].getTcp(),true,1);
				}
				else if(rmessage=="n")
				{
					envoyerA(idiscussion,"Voil� qui est plus sage !",clients[point].getTcp(),false,1);
				}
				clients[point].putTaille(0);
				return;
			}
			
			//=====On arrive l� : il s'agit d'un vrai message normal
			
			//On colorie le message en fonction du rang
			if(clients[point].getRang()=="admin")
			{
				MeAEn = "<font color='red'><strong>" + clients[point].getPseudo() + "</strong> : " + rmessage + "</font>";
			}
			else if(clients[point].getRang()=="modo")
			{
				MeAEn = "<font color='blue'><strong>" + clients[point].getPseudo() + "</strong> : " + rmessage + "</font>";
			}
			else if(clients[point].getRang()=="beta")
			{
				MeAEn = "<font color='green'><strong>" + clients[point].getPseudo() + "</strong> : " + rmessage + "</font>";
			}
			else
			{
				MeAEn = "<strong>" + clients[point].getPseudo() + "</strong> : " + rmessage;
			}
		}
		
		//Contr�le anti-flood
		/* d�sactiv� car inutile dans la discussion priv�e par 2
		if(((dernMess.size()>2 and dernMess.at(0)==MeAEn and dernMess.at(1)==MeAEn) or (clients[point].getMessagep(0)==MeAEn and clients[point].getMessagep(1)==MeAEn and clients[point].getMessagep(2)==MeAEn) or (dernMess.size()>8 and clients[point].getMessagep(0)==dernMess.at(0) and clients[point].getMessagep(1)==dernMess.at(1) and clients[point].getMessagep(2)==dernMess.at(2) and clients[point].getMessagep(3)==dernMess.at(3) and clients[point].getMessagep(4)==dernMess.at(4) and clients[point].getMessagep(5)==dernMess.at(5) and clients[point].getMessagep(6)==dernMess.at(6) and clients[point].getMessagep(7)==dernMess.at(7))) and (clients[point].getRang()!="admin" and clients[point].getRang()!="modo"))
		{
			disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
			disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
			envoyerA(tr("Le flood est interdit !"),clients[point].getTcp(),false,-1);
			QString pseudos = clients[point].getPseudo();
			clients.removeAt(point);
			nb--;
			envoyerATous(idiscussion,"<em><strong>" + pseudos + "</strong> " + tr("vient de se d�connecter") + " [FLOOD]</em>",3);
			std::cout << "D�connexion d'un client [FLOOD] : " << ch(pseudos) << " (" << ch(QString::number(nb)) << " conn�ct�s)" << std::endl;
			return;
		}
		dernMess.insert(0,MeAEn);
		 */
		
		envoyerATous(idiscussion,MeAEn); //On envoie le message
		//On enregistre aussi celui-ci
		
		clients[point].putMessagep(MeAEn);
		std::cout << "Message de " << ch(clients[point].getPseudo()) << " : " << ch(rmessage) << std::endl;
    }
    else if(type==2) //Infos : pseudo, version, langage
    {
        int rversion;
        in >> rversion;
        if(rversion<1900) //La v2.0b a pour num�ro 1900 (1.9.0.0 en fait)
        {
            envoyerA(0,"<strong>" + tr("La version de COW que vous utilisez n'est pas assez r�cente pour ce serveur. Veuillez t�l�charger la derni�re version : http://www.the-cow.org") + "</strong>",socket,false,-1);
			
            disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));

            disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
			
            clients.removeAt(point);

            std::cout << "D�connexion d'un client (version<1900) " << std::endl << std::endl;
            return;
        }
        QString rpseudo;
        QString rpass;
        QString rlangage;
        QString ros;
        in >> rpseudo;
        in >> rpass;
        in >> rlangage;
        in >> ros;
        
        //On va v�rifier le pseudo
        QSqlQuery vpseudo1;

        vpseudo1.exec("SELECT pseudo,pass,rang FROM membres WHERE pseudo='" + mys(rpseudo) + "'");

        while(vpseudo1.next())
        {

            if(vpseudo1.value(1).toString()!=QString(QCryptographicHash::hash(QString(rpass).toAscii(),QCryptographicHash::Sha1).toHex()))
            {
                envoyerA(0,"<strong>" + tr("Le mot de passe est incorrect !") + "</strong>",socket,false,-1);

                disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));

                disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));

                clients.removeAt(point);

                std::cout << "D�connexion d'un client (mdp incorrect)" << std::endl << std::endl;

                return;
            }
            //On s'occupe de son rang

            clients[point].putRang(vpseudo1.value(2).toString()); //On enregistre son rang (rangs : membres;modo;admin)

        }
		
        //On v�rifie si le pseudo n'est pas d�j� pris
        for (int i = 0; i < clients.size(); i++)
        {
            if(clients[i].getPseudo()==rpseudo)
            {
				QList<Discussion *> ldis = clients[i].liste_discussions();
				for(int j=0;j<ldis.size();j++)
				{
					Discussion *discussionsuppr = ldis[j];
					discussionsuppr->supprimer_client(&clients[i]); //On supprime le client de la conversation
					envoyerATous(discussionsuppr, "<em>" + clients[i].getPseudo() + " a quitt� la conversation</em>", 3);
					
					if(discussionsuppr->nombre_clients()==0)
					{
						for(int k=0;k<discussions.size();k++)
						{
							if(discussions[k].id()==discussionsuppr->id()) //Si la conversation est vide on supprime
							{
								discussions.removeAt(k);
								return;
							}
						}
					}
				}
				
				disconnect(clients[i].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
				
                disconnect(clients[i].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
				
				envoyerA(0,"<strong>" + tr("Vous avez �t� d�connect� car vous vous �tes connect� ailleurs") + "</strong>",clients[i].getTcp(),false,-1);
				
				clients.removeAt(i); //On supprime le client
				
				cout << "D�connexion d'un client (connect� ailleurs)" << endl;
				
				return;
            }
        }
		
        //On arrive ici : le pseudo/mdp OK ; pseudo disponible
        //On enregistre toutes les infos
        clients[point].putPseudo(rpseudo);
        clients[point].putVersion(rversion);
        clients[point].putLangage(rlangage);
        clients[point].putIP(socket->peerAddress().toString());
        clients[point].putOs(ros);
    }
	else if(type==3) //Nouvelle conversation : pseudo (avec qui on veut discuter)
	{
		QString rvpseudo;
		in >> rvpseudo;
		int rvpoint = -1;
		cout <<  ch("rvpseudo : " + rvpseudo) << endl;
		for(int i=0;i<clients.size();i++)
		{
			if(clients[i].getPseudo()==rvpseudo or clients[i].getPseudo() + " (Absent)"==rvpseudo or clients[i].getPseudo() + " (Occup�)"==rvpseudo or clients[i].getPseudo() + "[ADMIN]"==rvpseudo or clients[i].getPseudo() + "[MODO]"==rvpseudo or clients[i].getPseudo() + "[B�TA]"==rvpseudo)
				rvpoint = i;
		}
		if(rvpoint==-1) //L'utilisateur est introuvable
		{
			envoyerA(0,"L'utilisateur avec qui vous d�sirez discuter est introuvable. Il est possible qu'il utilise une version de COW trop ancienne ne prenant pas en charge la discussion priv�e (&gt;2.0)",clients[point].getTcp(),false,1);
			clients[point].putTaille(0);
			return;
		}
		if(rvpoint==point)
		{
			clients[point].putTaille(0);
			return;
		}
		
		for(int j=0;j<discussions.size();j++)
		{
			if(discussions[j].nombre_clients()==2 and discussions[j].client_existe(&clients[point]) and discussions[j].client_existe(&clients[rvpoint]))
			{
				clients[point].putTaille(0);
				return;
			}
		}
		
		int nid = 1;
		if(!discussions.isEmpty())
			nid = discussions.last().id()+1;
		
		Discussion ndiscussion(nid);
		discussions << ndiscussion;
		
		discussions.last().ajouter_client(&clients[point]);
		discussions.last().ajouter_client(&clients[rvpoint]);
		
		envoyerATous(&discussions.last(), "<em>Debut d'une conversation entre " + clients[point].getPseudo() + " et " + clients[rvpoint].getPseudo() + "</em>", 3);
		
		clients[point].putTaille(0);
		return;
	}
	else if(type==6) //Quitte la conversation (sans se d�connecter)
	{
		int rdiscussion;
		in >> rdiscussion;
		
		for(int j=0;j<clients[point].liste_discussions().size();j++)
		{
			if(clients[point].liste_discussions().at(j)->id()==rdiscussion)
			{
				Discussion *discussionq = clients[point].liste_discussions().at(j);
				clients[point].liste_discussions().at(j)->supprimer_client(&clients[point]);
				envoyerATous(discussionq, "<em>" + clients[point].getPseudo() + " a quitt� la conversation</em>", 3);
				clients[point].putTaille(0);
				if(discussionq->nombre_clients()==0)
				{
					for(int k=0;k<discussions.size();k++)
					{
						if(discussions[k].id()==rdiscussion) //Si la conversation est vide on supprime
						{
							discussions.removeAt(k);
							return;
						}
					}
				}
			}
		}
		
		clients[point].putTaille(0);
		return;
	}
	clients[point].putTaille(0);


}

void Serveur::envoyerATous(Discussion *discussion,const QString &message,int type,int fpoint)
{
    // Pr�paration du paquet
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) 0; // On �crit 0 au d�but du paquet pour r�server la place pour �crire la taille
    out << discussion->id(); //On ajoute l'id de la discussion
	cout << ch("ID conv : " + QString::number(discussion->id())) << endl;
    out << message; // On ajoute le message � la suite
    out << type; //On ajoute ensuite le type, afin que le client connaisse l'architecture du paquet
    if(type==2) //On envoie le nombre de conn�ct�s
    {
        out << discussion->nombre_clients();
    }
    else if(type==3) //On envoie la liste des connnect�s
    {
        QStringList listeco;
        for (int i = 0; i < discussion->nombre_clients(); i++)
        {
			listeco << discussion->liste_clients().at(i)->getPseudo();
        }
		out << listeco;
    }
    out.device()->seek(0); // On se replace au d�but du paquet
    out << (quint16) (paquet.size() - sizeof(quint16)); // On �crase le 0 qu'on avait r�serv� par la longueur du message

    // Envoi du paquet pr�par� � tous les clients connect�s au serveur
    for (int i = 0; i < discussion->nombre_clients(); i++)
    {
	if(i!=fpoint)
	    discussion->liste_clients().at(i)->getTcp()->write(paquet);
    }
}

void Serveur::envoyerA(Discussion *discussion,const QString &message, QTcpSocket *Tcp, bool simple,int type,int fdiscussion) //Envoi d'un message � un client pr�cis
{
    // Pr�paration du paquet
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    if(simple)
    {
        messagee = message;
    }
    else
    {
        messagee = "<em><font color='red'>" + tr("Message priv� du serveur") + " : </em>" + message + "</font>";
    }
    out << (quint16) 0; // On �crit 0 au d�but du paquet pour r�server la place pour �crire la taille
	if(discussion!=0)
		out << discussion->id(); //On ajoute l'ID de la discussion
	else
		out << fdiscussion;
    out << messagee; // On ajoute le message � la suite
    out << type; //On ajoute le type
	if(type==3) //Cela signifie que l'on va envoyer la liste des utilisateurs
	{
		QStringList listeco;
        for (int i = 0; i < discussion->nombre_clients(); i++)
        {
			listeco << discussion->liste_clients().at(i)->getPseudo();
			if(!simple && clients[i].getTcp()==Tcp)
			{
				cout << "Message priv� � " << ch(clients[i].getPseudo()) << " : " << ch(message) << endl;
			}
		}
		out << listeco;
	}
    out.device()->seek(0); // On se replace au d�but du paquet
    out << (quint16) (paquet.size() - sizeof(quint16)); // On �crase le 0 qu'on avait r�serv� par la longueur du message


    // Envoi du paquet au client
    Tcp->write(paquet);
}

//Fonctions sans rapport
const char* Serveur::ch(QString texte)
{
    return texte.toStdString().c_str();
}
QString Serveur::mys(QString texte) //s�curisation SQL
{
    QRegExp antis("\\");
    texte.replace(antis,"\\\\");
    QRegExp strips("'");
    texte.replace(strips,"\\'");
    return texte;
}
long Serveur::timestamp() //Retourne le timestamp actuel
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime st = QDateTime::QDateTime(QDate(1970,01,01),QTime(01,00,00));
    return st.secsTo(now);
}
QString Serveur::zero(int numero)
{
    if(numero<10)
    {
	return "0" + QString::number(numero);
    }
    else
    {
	return QString::number(numero);
    }
}
