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

Serveur::Serveur(int port)
{
    serveurt = new QTcpServer(); //Cr�ation du serveur
    
    nb = 0;
    tailleMessage = 0;
    pport = port;
    versionClient = 1090;
    if(pport==10100)
    {
        fnb(false,false,true);
        fnb(false,true,true);
    }
    //Statuts
    statuts.clear();
    statuts << "En Ligne" << "Occup�" << "Absent";
    //IP bannies
    ipban.clear();
    
    if (!serveurt->listen(QHostAddress::Any, port))
    {
        std::cout << "Erreur lors du d�marrage : " << ch(serveurt->errorString()) << std::endl;
        etat = false;
		return;
    }
    else
    {
        std::cout << "Le serveur a bien �t� d�marr� sur le port " << 
        ch(QString::number(port)) << std::endl;
        etat = true;
        QObject::connect(serveurt, SIGNAL(newConnection()), this, SLOT(nouvelleConnexion()));
    }
    
    //BDD
	/*
	 La base de donn�e est utilis�e pour stocker le message
	 de bienvenue de le code de la publicit� ; ainsi que
	 pour enregistrer les messages �chang�s (dans le but
	 de proposer un historique en ligne sur le site)
	 */
    db = QSqlDatabase::addDatabase("QMYSQL");
	db.setHostName("127.0.0.1");
	db.setDatabaseName("NOM_DE_LA_BDD");
	db.setUserName("UTILISATEUR_DE_LA_BDD");
	db.setPassword("MOT_DE_PASSE");
    bool ok = db.open();
    if(ok)
    {
		std::cout << "Connexion � la base de donn�es r�ussie !" << std::endl;
    }
    else
    {
		std::cout << "Erreur lors de la connexion � MySQL !" << std::endl;
		return;
    }
    
    //On r�cup�re quelques infos depuis la BDD (le nom du serveur, le message de bienvenue, la pub)
    QSqlQuery messBP;
    messBP.exec("SELECT nom,messageBienvenue,messagePub FROM neo_serveurs WHERE port='" + QString::number(pport) + "'");
    while(messBP.next())
    {
		messageBienvenue = messBP.value(1).toString();
		messagePub = messBP.value(2).toString();
    }
    
    msgd.clear();
}

//Slots
void Serveur::nouvelleConnexion()
{
    nb++;
    QTcpSocket *nouveauClient = serveurt->nextPendingConnection();
    
    if(ipban.contains(nouveauClient->peerAddress().toString()))
    {
		envoyerA("<strong>" + tr("Vous avez �t� banni de COW, aussi vous ne pouvez plus vous connecter.") + "</strong>",nouveauClient,false,-1);
    }
    else
    {
		Client nouveau("",nouveauClient);
		nouveau.putIP(nouveauClient->peerAddress().toString());
		clients << nouveau;
		connect(nouveauClient, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
		connect(nouveauClient, SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
		std::cout << "Connexion d'un nouveau client" << " (" << ch(QString::number(nb)) << " conn�ct�s)" << std::endl;
    }
}

void Serveur::deconnexionClient()
{
    nb--;
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
    
    //On r�cup�re les variables utiles
    QString pseudos = clients[point].getPseudo();
    QString versions = QString::number(clients[point].getVersion());
    QString ips = clients[point].getIP();
    clients.removeAt(point); //On supprime le client
    envoyerATous("<em><strong>" + pseudos + "</strong> " + tr("vient de se d�connecter") + "</em>",3);
    std::cout << "D�connexion d'un client : " << ch(pseudos) << " (" << ch(QString::number(nb)) << " conn�ct�s)" << std::endl;
    
    //On incr�mente le fichier - fichier utilis� pour pouvoir afficher sur le site le nombre de connect�s et le dernier message envoy�
    fnb(false);
    if(pport==10100)
        fnb(false,true);
    
    QSqlQuery query;
      query.exec("INSERT INTO neo_statistiques VALUES('','" + QString::number(pport) + "','" + mys(pseudos) + "','" + mys(versions) + "','" + mys(ips) + "','0','" + QString::number(timestamp()) + "','" + QString::number(nb) + "')");
    if(query.lastError().text()!="")
    {
		std::cout << "Erreur : " << ch(query.lastError().text()) << std::endl;
    }
    socket->deleteLater();
}

void Serveur::donneesRecues()
{
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

        in >> tailleMessage; // Si on a re�u la taille du message en entier, on la r�cup�re
        clients[point].putTaille(tailleMessage);
        tailleMessage = 0;
    }

    // Si on conna�t la taille du message, on v�rifie si on a re�u le message en entier
    if (socket->bytesAvailable() < clients[point].getTaille()) // Si on n'a pas encore tout re�u, on arr�te la m�thode
        return;

    //On r�cup�re le type
    int type;
    in >> type;
    
    if(type==1) //Message
    {
		QString rmessage;
		in >> rmessage;
	
		if(clients[point].getPseudo()=="") //On v�rifie que les informations nous ont d�j� �t� envoy�es
		{
			envoyerA("Vous ne pouvez pas envoyer de message tant que vous n'avez pas communiquer les informations de base : pseudo, version du client, langage, os.",clients[point].getTcp(),false,1);
			clients[point].putTaille(0);
			return;
		}
	
		//Protection contre les message vides
		QString rmessagev = rmessage;
		QRegExp esp(" ");
		rmessagev.replace(esp,"");
		if(rmessagev=="")
		{
			envoyerA(tr("Les messages vides sont interdits !"),clients[point].getTcp(),false,1,0,false);
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
	    QRegExp cliens("((https?)|ftp|ssh|sftp)://([a-zA-Z0-9._/&%~,;\\?=\\#\\+-]+)");
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
	    rmessage.replace(sjj,"<img src=':/smileys/siffle.png' alt=':-o' />");
	    rmessage.replace(sj,"<img src=':/smileys/siffle.png' alt=':-o' />");
	    rmessage.replace(sk,"<img src=':/smileys/mechant.png' alt=':@' />");
	    rmessage.replace(sl,"<img src=':/smileys/blink.png' alt='oO' />");
		//==========
		
		QString MeAEn = ""; //Message A Envoyer
	
		//=====Commandes
		QRegExp commande_me("^/me (.+)$");
		QRegExp commande_away("^/away( (.+))?$");
		QRegExp commande_back("^/back$");
		QRegExp commande_msg("^/msg (\\S+) (.+)$");
        QRegExp commande_msgs("^/msgs$");
		QRegExp commande_help("^/help$");
		QRegExp commande_kick("^/kick (\\S+)( (.+))?$");
		QRegExp commande_ip("^/ip (\\S+)$");
		QRegExp commande_os("^/os (\\S+)$");
		QRegExp commande_showtime("^/showtime$");
		QRegExp commande_ips("^/ips$");
		QRegExp commande_ipban("^/ipban$");
		QRegExp commande_ban("^/ban (\\S+)( (.+))?$");
		QRegExp commande_deban("^/deban (\\S+)$");
		QRegExp commande_infos("^/infos$");
	
		if(commande_me.indexIn(rmessage)!=-1) //Il s'agit de la commande /me
		{
			MeAEn = "<em><strong>" + clients[point].getPseudo() + "</strong> " + commande_me.cap(1) + "</em>";
		}
		else if(commande_away.indexIn(rmessage)!=-1) //Il s'agit de la commande /away
		{
			if(clients[point].getStatut()==2) //Absent
			{
				envoyerA("Vous �tes d�j� absent(e) !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
			clients[point].putStatut(2);
			QString away = "est absent(e)";
			if(commande_away.cap(2)!="")
				away = "est absent(e) (" + commande_away.cap(2) + ")";

			MeAEn = "<em><strong>" + clients[point].getPseudo() + "</strong> " + away + "</em>";
			envoyerA(MeAEn,clients[point].getTcp(),true,6,clients[point].getStatut());
			envoyerATous(MeAEn,3,0,point);
			clients[point].putTaille(0);
			return;
		}
		else if(commande_back.indexIn(rmessage)!=-1) //Il s'agit de la commande /back
		{
			if(clients[point].getStatut()==0) //En Ligne
			{
				envoyerA("Vous n'�tes pas absent(e) !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
			clients[point].putStatut(0);
			MeAEn = "<em><strong>" + clients[point].getPseudo() + "</strong> est revenu(e)</em>";
			envoyerA(MeAEn,clients[point].getTcp(),true,6,clients[point].getStatut());
			envoyerATous(MeAEn,3,0,point);
			clients[point].putTaille(0);
			return;
		}
		else if(commande_msg.indexIn(rmessage)!=-1) //Il s'agit de la commande /msg
		{
			QString PseudoAEn = commande_msg.cap(1);
			QString MessageAEn = commande_msg.cap(2);
			//=====ANTI FLOOD
			if(MessageAEn==clients[point].getMessagep(0) and clients[point].getMessagep(0)==clients[point].getMessagep(1) and clients[point].getMessagep(1)==clients[point].getMessagep(2) and clients[point].getRang()!="modo" and clients[point].getRang()!="admin")
			{
				disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
				disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
				envoyerA(tr("Le flood est interdit !"),clients[point].getTcp(),false,-1);
				QString pseudos = clients[point].getPseudo();
				clients.removeAt(point);
				nb--;
				envoyerATous("<em><strong>" + pseudos + "</strong> " + tr("vient de se d�connecter") + " [FLOOD]</em>",3);
				std::cout << "D�connexion d'un client [FLOOD] : " << ch(pseudos) << " (" << ch(QString::number(nb)) << " conn�ct�s)" << std::endl;
				return;
			}
			
			clients[point].putMessagep(MessageAEn);
			//==========
	    
	    
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
					envoyerA(MeAEn,clients[pointAEn].getTcp(),true,1);
					envoyerA(MeAEn2,clients[point].getTcp(),true,1);
				}
				else
				{
					envoyerA(MeAEn,clients[pointAEn].getTcp(),true,1);
				}
				clients[point].putTaille(0);
				return;
			}
			else
			{
				bool trouvea = false;
				if(msgd.size()>0)
				{
                    for(int i = 0;i < msgd.size();i++)
                    {
                        if(msgd[i].size()>0 and msgd[i].at(0)==PseudoAEn)
                        {
                            if(msgd[i].size()<50)
                            {
                                QDateTime now2 = QDateTime::currentDateTime();
                                msgd[i] << "<font color='gray'>(" + zero(now2.date().day()) + "/" + zero(now2.date().month()) + "/" + zero(now2.date().year()) + "@" + zero(now2.time().hour()) + ":" + zero(now2.time().minute()) + ")</font> <strong>" + clients[point].getPseudo() + "</strong> : " + MessageAEn;
                            }
                            else
                            {
                                envoyerA("Cette utilisateur est d�connect� et a atteint son quota de messages diff�r�s. Votre message ne lui sera donc pas transmit. D�sol�.",clients[point].getTcp(),false,1);
                                clients[point].putTaille(0);
                                return;
                            }
                            trouvea = true;
                        }
                    }
				}

                if(!trouvea)
                {
                    QStringList tmsgdp;
                    tmsgdp.clear();
                    tmsgdp << PseudoAEn;
                    QDateTime now2 = QDateTime::currentDateTime();
                    tmsgdp << "<font color='gray'>(" + zero(now2.date().day()) + "/" + zero(now2.date().month()) + "/" + zero(now2.date().year()) + "@" + zero(now2.time().hour()) + ":" + zero(now2.time().minute()) + ")</font> <strong>" + clients[point].getPseudo() + "</strong> : " + MessageAEn;
                    msgd << tmsgdp;
                }
                  
				envoyerA("Le message a �t� envoy� en message diff�r� ; l'utilisateur le verra leur de sa prochaine connexion",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
        else if(commande_msgs.indexIn(rmessage)!=-1) //Il s'agit de la commande /ip
        {
            if(clients[point].getRang()=="admin")
            {
                QString MeAEn = "<strong>Liste des messages diff�r�s</strong><ul>";
                if(msgd.size()>0)
                {
                    for(int i = 0;i < msgd.size();i++)
                    {
                        if(msgd[i].size()>0)
                        {
                            MeAEn += "<li>Pour : <strong>" + msgd[i].at(0) + "</strong> :";
                            for(int j = 1;j < msgd[i].size();j++)
                            {
                                MeAEn += "<br />" + msgd[i].at(j);
                            }
                            MeAEn += "</li>";
                        }
                    }
                }
                MeAEn += "</ul><br />";
                envoyerA(MeAEn,clients[point].getTcp(),true,1);
                
                clients[point].putTaille(0);
                return;
            }
            else
            {
                envoyerA("Vous n'�tes pas assez haut grad�",clients[point].getTcp(),false,1);
                clients[point].putTaille(0);
                return;
            }
        }
		else if(commande_help.indexIn(rmessage)!=-1) //Il s'agit de la commande /help
		{
			MeAEn = "<font color='gray'><hr /><strong>Commandes disponibles</strong><ul><li><strong>/me [MESSAGE]</strong> : affiche un message vous concernant<br />Exemple : /me adore COW<br />Affiche : <em><strong>pseudo</strong> adore cow</em></li><li><strong>/away [RAISON]</strong> : indique que vous �tes absent(e) pour la raison indiqu�e (qui n'est pas obligatoire, vous pouvez vous contener de <strong>/away</strong>)<br />Exemple : /away<br />Affiche : <em><strong>pseudo</strong> est absent(e)</em><br />Exemple 2 : /away pause caf� de 5 minutes !<br />Affiche : <em><strong>pseudo</strong> est absent(e) (pause caf� de 5 minutes !)</em></li><li><strong>/back</strong> : contraire � <strong>/away</strong>, indique que vous �tes revenu(e)<br />Exemple : /back<br />Affiche : <em><strong>pseudo</strong> est revenu(e)</li><li><strong>/msg [PSEUDO] [MESSAGE]</strong> : envoie un message priv� [MESSAGE] � [PSEUDO]<br />Exemple : /msg Jean tu as bien envoy� l'email ?<br />Affichage : <em>Message priv� de </em><strong>Marc</strong><em> : </em>tu as bien envoy� l'email ?</li><li><strong>/showtime</strong> : active ou d�sactive l'affichage de l'heure des messages re�us</li><li><strong>/help</strong> : affiche les commandes disponibles et leur utilisation</li></ul><hr /></font><br />";
			envoyerA(MeAEn,clients[point].getTcp(),true,1,0,false);
			clients[point].putTaille(0);
			return;
		}
		else if(commande_kick.indexIn(rmessage)!=-1) //Il s'agit de la commande /kick
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
					if(clients[pointAEn].getRang()=="admin" and clients[point].getRang()!="admin")
					{
						envoyerA("Vous n'avez pas le droit de kicker un admin !",clients[point].getTcp(),false,1);
						envoyerA(clients[point].getPseudo() + " a essay� de vous kicker !",clients[pointAEn].getTcp(),false,1);
						clients[point].putTaille(0);
						return;
					}
		    
					disconnect(clients[pointAEn].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
					disconnect(clients[pointAEn].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
					envoyerA("Vous avez �t� d�connect� par <strong>" + clients[point].getPseudo() + "</strong>. La raison donn�e est : " + RaisonKick,clients[pointAEn].getTcp(),false,-1);
					QString pseudos = clients[pointAEn].getPseudo();
					clients.removeAt(pointAEn);
					nb--;
					for (int i = 0; i < clients.size(); i++) //Boucle pour trouver le num�ro correspondant au socket
					{
						if(clients[i].getTcp()==socket)
						{
							point = i;
						}
					}
					envoyerATous("<em><strong>" + pseudos + "</strong> " + tr("vient de se d�connecter") + " [kick� par <strong>" + clients[point].getPseudo() + "</strong> : " + RaisonKick + "]</em>",3);
					clients[point].putTaille(0);
					return;
				}
				else
				{
					envoyerA("Le pseudo indiqu� ne correspond � aucun utilisateur connect� !",clients[point].getTcp(),false,1);
					clients[point].putTaille(0);
					return;
				}
			}
			else
			{
				envoyerA("Vous n'�tes pas assez haut grad� pour kicker !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_ip.indexIn(rmessage)!=-1) //Il s'agit de la commande /ip
		{
			if(clients[point].getRang()=="admin" or clients[point].getRang()=="modo")
			{
				QString PseudoIP = commande_ip.cap(1);
				int pointAEn = -1;
				for (int i = 0; i < clients.size(); i++)
				{
					if(clients[i].getPseudo()==PseudoIP or clients[i].getPseudo()==(PseudoIP + "[ADMIN]") or clients[i].getPseudo()==(PseudoIP + "[MODO]") or clients[i].getPseudo()==(PseudoIP + "[B�TA]"))
					{
						pointAEn = i;
					}
				}
				if(pointAEn>-1)
				{
					if(clients[pointAEn].getRang()=="admin" and clients[point].getRang()!="admin")
					{
						envoyerA("Vous n'avez pas le droit de conna�tre l'ip d'un admin !",clients[point].getTcp(),false,1);
						envoyerA(clients[point].getPseudo() + " a essay� de conna�tre votre ip !",clients[pointAEn].getTcp(),false,1);
						clients[point].putTaille(0);
						return;
					}
		    
					envoyerA("L'adresse IP de "+clients[pointAEn].getPseudo()+" est : <a title='Avoir des informations sur cette IP' href='http://www.wolframalpha.com/input/?i="+clients[pointAEn].getIP()+"'>"+clients[pointAEn].getIP()+"</a>",clients[point].getTcp(),false,1);
		    
					clients[point].putTaille(0);
					return;
				}
				else
				{
					envoyerA("Le pseudo indiqu� ne correspond � aucun utilisateur connect� !",clients[point].getTcp(),false,1);
					clients[point].putTaille(0);
					return;
				}
			}
			else
			{
				envoyerA("Vous n'�tes pas assez haut grad� pour conna�tre l'IP d'un connect� !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_ips.indexIn(rmessage)!=-1) //Il s'agit de la commande /ip
		{
			if(clients[point].getRang()=="admin")
			{
				QString MeAEn = "<strong>IPs des connect�s</strong><ul>";
				for (int i = 0;i<clients.size();i++)
				{
					MeAEn += "<li><strong>" + clients[i].getPseudo() + "</strong> : <a title='Avoir des informations sur cette IP' href='http://www.wolframalpha.com/input/?i="+clients[i].getIP()+"'>"+clients[i].getIP()+"</a></li>";
				}
				MeAEn += "</ul><br />";
				envoyerA(MeAEn,clients[point].getTcp(),true,1);
		
				clients[point].putTaille(0);
				return;
			}
			else
			{
				envoyerA("Vous n'�tes pas assez haut grad� pour conna�tre les IPs des connect�s",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_os.indexIn(rmessage)!=-1) //Il s'agit de la commande /ip
		{
			if(clients[point].getRang()=="admin" or clients[point].getRang()=="modo")
			{
				QString PseudoOS = commande_os.cap(1);
				int pointAEn = -1;
				for (int i = 0; i < clients.size(); i++)
				{
					if(clients[i].getPseudo()==PseudoOS or clients[i].getPseudo()==(PseudoOS + "[ADMIN]") or clients[i].getPseudo()==(PseudoOS + "[MODO]") or clients[i].getPseudo()==(PseudoOS + "[B�TA]"))
					{
						pointAEn = i;
					}
				}
				if(pointAEn>-1)
				{
					if(clients[pointAEn].getRang()=="admin" and clients[point].getRang()!="admin")
				{
					envoyerA("Vous n'avez pas le droit de conna�tre l'os d'un admin !",clients[point].getTcp(),false,1);
					envoyerA(clients[point].getPseudo() + " a essay� de conna�tre votre os !",clients[pointAEn].getTcp(),false,1);
					clients[point].putTaille(0);
					return;
				}
		    
					envoyerA("L'OS de "+clients[pointAEn].getPseudo()+" est : "+clients[pointAEn].getOs(),clients[point].getTcp(),false,1);
		    
					clients[point].putTaille(0);
					return;
				}
				else
				{
					envoyerA("Le pseudo indiqu� ne correspond � aucun utilisateur connect� !",clients[point].getTcp(),false,1);
					clients[point].putTaille(0);
					return;
				}
			}
			else
			{
				envoyerA("Vous n'�tes pas assez haut grad� pour conna�tre l'OS d'un connect� !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_showtime.indexIn(rmessage)!=-1) //Il s'agit de la commande /back
		{
			if(clients[point].getOptHeureMessAff()) //Affichage de l'heure
			{
				clients[point].putOptHeureMessAff(false);
				envoyerA("<font color='gray'><em>Affichage de l'heure <strong>d�sactiv�</strong></em></font>",clients[point].getTcp(),true,1);
				clients[point].putTaille(0);
				return;
			}
			else
			{
				clients[point].putOptHeureMessAff(true);
				envoyerA("<font color='gray'><em>Affichage de l'heure <strong>activ�</strong></em></font>",clients[point].getTcp(),true,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_ban.indexIn(rmessage)!=-1) //Il s'agit de la commande /ban
		{
			if(clients[point].getRang()=="admin")
			{
				QString IpBan = commande_ban.cap(1);
				QString RaisonBan = commande_ban.cap(3);
		
				if(ipban.contains(IpBan))
				{
					envoyerA("L'IP indiqu�e est d�j� bannie",clients[point].getTcp(),false,1);
					clients[point].putTaille(0);
					return;
				}
		
				ipban << IpBan;
		
				QString pseudos = "";
		
				for (int i = 0; i < clients.size(); i++)
				{
					if(clients[i].getIP()==IpBan)
					{
						disconnect(clients[i].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
						disconnect(clients[i].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
						envoyerA("Vous avez �t� d�connect� et <strong>banni</strong> par <strong>" + clients[point].getPseudo() + "</strong>. La raison donn�e est : " + RaisonBan,clients[i].getTcp(),false,-1);
						pseudos += clients[i].getPseudo() + ", ";
						clients.removeAt(i);
						nb--;
					}
				}
		
				envoyerATous("<em><strong>" + pseudos + "</strong> " + tr("vien(nen)t de se d�connecter") + " [bannis par <strong>" + clients[point].getPseudo() + "</strong> : " + RaisonBan + "]</em>",3);
				clients[point].putTaille(0);
				return;
			}
			else
			{
				envoyerA("Vous n'�tes pas assez haut grad� pour bannir !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_ipban.indexIn(rmessage)!=-1) //Il s'agit de la commande /ipban
		{
			if(clients[point].getRang()=="admin" or clients[point].getRang()=="modo")
			{
				QString MeAEn = "<strong>IPs bannies</strong><ul>";
				for (int i = 0;i < ipban.size();i++)
				{
					MeAEn += "<li><a title='Avoir des informations sur cette IP' href='http://www.wolframalpha.com/input/?i="+ipban.at(i)+"'>"+ipban.at(i)+"</a></li>";
				}
				MeAEn += "</ul><br />";
				envoyerA(MeAEn,clients[point].getTcp(),true,1);
		
				clients[point].putTaille(0);
				return;
			}
			else
			{
				envoyerA("Vous n'�tes pas assez haut grad� pour conna�tre les IPs bannies",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_deban.indexIn(rmessage)!=-1) //Il s'agit de la commande /ban
		{
			if(clients[point].getRang()=="admin")
			{
				QString IpDeBan = commande_deban.cap(1);
		
				if(!ipban.contains(IpDeBan))
				{
					envoyerA("L'IP indiqu�e n'est pas encore bannie",clients[point].getTcp(),false,1);
					clients[point].putTaille(0);
					return;
				}
		
				ipban.removeOne(IpDeBan);
		
				envoyerA("L'IP a bien �t� d�bannie",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
			else
			{
				envoyerA("Vous n'�tes pas assez haut grad� pour d�bannir !",clients[point].getTcp(),false,1);
				clients[point].putTaille(0);
				return;
			}
		}
		else if(commande_infos.indexIn(rmessage)!=-1) //Il s'agit de la commande /ip
		{
			if(clients[point].getRang()=="admin")
			{
				QString MeAEn = "<strong>Informations</strong><table><tr><th>ID</th><th>Pseudo</th><th>IP</th><th>OS</th><th>Version</th></tr>";
				for (int i = 0;i<clients.size();i++)
				{
					MeAEn += "<tr><td></td><td><strong>" + clients[i].getPseudo() + "</strong></td><td><a title='Avoir des informations sur cette IP' href='http://www.wolframalpha.com/input/?i="+clients[i].getIP()+"'>"+clients[i].getIP()+"</a></td><td>"+clients[i].getOs()+"</td><td>"+QString::number(clients[i].getVersion())+"</td></tr>";
				}
				MeAEn += "</table><br />";
				envoyerA(MeAEn,clients[point].getTcp(),true,1);
		
				clients[point].putTaille(0);
				return;
			}
			else
			{
				envoyerA("Vous n'�tes pas assez haut grad� pour avoir des infos sur les connect�s",clients[point].getTcp(),false,1);
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
					envoyerA("<em>Destruction du monde enclanch�e !</em><br /><br />>Envoie d'un email d'insultes au pr�sident des Etats-Unis...<br />>Envoie en cours...<br />...<br />...<br />...<br /><font color='red'><strong>ERROR : THEWEBMAILINUSAISWINDOWS!!IMPOSSIBLETOSENDANEMAIL!!</strong></font><br /><em>Destruction du monde annul�e</em>",clients[point].getTcp(),true,1);
				}
				else if(rmessage=="n")
				{
					envoyerA("Voil� qui est plus sage !",clients[point].getTcp(),false,1);
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
		if(((dernMess.size()>2 and dernMess.at(0)==MeAEn and dernMess.at(1)==MeAEn) or (clients[point].getMessagep(0)==MeAEn and clients[point].getMessagep(1)==MeAEn and clients[point].getMessagep(2)==MeAEn) or (dernMess.size()>8 and clients[point].getMessagep(0)==dernMess.at(0) and clients[point].getMessagep(1)==dernMess.at(1) and clients[point].getMessagep(2)==dernMess.at(2) and clients[point].getMessagep(3)==dernMess.at(3) and clients[point].getMessagep(4)==dernMess.at(4) and clients[point].getMessagep(5)==dernMess.at(5) and clients[point].getMessagep(6)==dernMess.at(6) and clients[point].getMessagep(7)==dernMess.at(7))) and (clients[point].getRang()!="admin" and clients[point].getRang()!="modo"))
		{
			disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
			disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
			envoyerA(tr("Le flood est interdit !"),clients[point].getTcp(),false,-1);
			QString pseudos = clients[point].getPseudo();
			clients.removeAt(point);
			nb--;
			envoyerATous("<em><strong>" + pseudos + "</strong> " + tr("vient de se d�connecter") + " [FLOOD]</em>",3);
			std::cout << "D�connexion d'un client [FLOOD] : " << ch(pseudos) << " (" << ch(QString::number(nb)) << " conn�ct�s)" << std::endl;
			return;
		}
	
		envoyerATous(MeAEn); //On envoie le message
	
	
		//On enregistre aussi celui-ci
		dernMess.insert(0,MeAEn);
		if(dernMess.size()>10)
			dernMess.removeAt(10);
	
		clients[point].putMessagep(MeAEn);
		std::cout << "Message de " << ch(clients[point].getPseudo()) << " : " << ch(rmessage) << std::endl;
    }
    else if(type==2) //Infos
    {
		int rversion;
		QString rpseudo;
		QString rmdp;
		QString rlangage;
		QString ros;
		in >> rversion;
		in >> rpseudo;
		in >> rmdp;
		in >> rlangage;
		in >> ros;
		clients[point].putVersion(rversion);
		clients[point].putLangage(rlangage);
		clients[point].putOs(ros);
	
		bool pseudookchar = true;
		for (int i=0;i<rpseudo.size();i++)
		{
			if(rpseudo.at(i).category()==11)
				pseudookchar = false;
		}
		if(!pseudookchar)
		{
			envoyerA("<strong>" + tr("Votre pseudo contient des caract�res non autoris�s") + "</strong>",socket,false,-1);
			disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
			disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
			clients.removeAt(point);
			nb--;
			std::cout << "D�connexion d'un client (caract�res incorrects) : " << ch(QString::number(nb)) << " connect�s" << std::endl << std::endl;
			return;
		}
	
		//=====Pseudo
		//Protection, on ne se fait pas passer pour un admin
		QRegExp sec1("\\[(ADMIN|MODO|B�TA|BETA|ADMLN|M0DO|MOD0|M0D0|B3TA|4DMIN|4DMLN)\\]",Qt::CaseInsensitive);
		rpseudo.replace(sec1,"");
		//Protection contre les espaces
		QRegExp esp(" ");
		rpseudo.replace(esp,"");

	    for (int i = 0; i < clients.size(); i++) //On refais une boucle pour parcourir tous les utilisateurs et v�rifier que personne n'utilise d�j� ce pseudo
	    {
		if(clients[i].getPseudo()==rpseudo or clients[i].getPseudo()==rpseudo + "[ADMIN]" or clients[i].getPseudo()==rpseudo + "[MODO]" or clients[i].getPseudo()==rpseudo + "[B�TA]") //Ah ! Quelqu'un l'utilise d�j� !
		{
		    rpseudo = "";
		}
	    }
		//=====Pseudo enregistr� avec mdp
		
		QSqlQuery pseudov;
		pseudov.exec("SELECT id,pseudo,rang_chat,opt_chat_heure_mess_aff AS rang FROM membres WHERE pseudo='" + mys(rpseudo) + "' AND pass!=''");
		while(pseudov.next()) //Il s'agit d'un pseudo enregistr�
		{
			bool mdpok = FALSE;
			
			QSqlQuery mdpv;
			mdpv.exec("SELECT id,pseudo,rang FROM membres WHERE pseudo='" + mys(rpseudo) + "' AND pass='" + QString(QCryptographicHash::hash(QString(rmdp).toAscii(),QCryptographicHash::Sha1).toHex()) + "'");
			while(mdpv.next()) //Le mot de passe est OK
			{
			    mdpok = TRUE;
			}
			
			if(!mdpok) //Si le mot de passe n'est pas bon
			{
			    envoyerA("<strong>" + tr("Le mot de passe est incorrect !") + "</strong>",socket,false,-1);
			    disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
			    disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
			    clients.removeAt(point);
			    nb--;
			    std::cout << "D�connexion d'un client (mdp incorrect) : " << ch(QString::number(nb)) << " connect�s" << std::endl << std::endl;
			    return;
			}
		    
		    //On s'occupe de son rang
		    clients[point].putRang(pseudov.value(2).toString()); //On enregistre son rang
		    clients[point].putOptHeureMessAff(pseudov.value(3).toBool()); //On enregistre ses options
			
		}
		
		//--------------------------------------------------
	    
	    if(rpseudo=="") //QString message est vide, cela signifie que soit le pseudo est d�j� utilis�, soit le pseudo contenait des caract�res incorrects (espace) qui ont �t� enlev�s, et il est maintenant vide
	    {
			envoyerA("<strong>" + tr("Ce pseudo est d�j� utilis� ! Veuillez en choisir un autre !") + "</strong>",socket,false,-1);
			disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
			disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
			clients.removeAt(point);
			nb--;
			std::cout << "D�connexion d'un client (pseudo d�j� utilis�) : " << ch(QString::number(nb)) << " connect�s" << std::endl << std::endl;
			return;
	    }
	    else //C'est OK
	    {
			if(rpseudo.size()>25)
			{
				envoyerA("<strong>" + tr("Ce pseudo est trop long, veuillez en choisir un autre.") + "</strong>",socket,false,-1);
				disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
				disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
				clients.removeAt(point);
				nb--;
				std::cout << "D�connexion d'un client (pseudo trop long) : " << ch(QString::number(nb)) << " connect�s" << std::endl << std::endl;
				return;
			}
			
			//=====Tout est OK : pseudo libre, mdp OK
			if(clients[point].getRang()=="admin")
				rpseudo = rpseudo + "[ADMIN]";
			else if(clients[point].getRang()=="modo")
				rpseudo = rpseudo + "[MODO]";
			else if(clients[point].getRang()=="beta")
				rpseudo = rpseudo + "[B�TA]";
			clients[point].putPseudo(rpseudo); //On enregistre le pseudo
		
			//On r�cup�re les 5 derniers messages
			QString dern = "";
			QSqlQuery query;
			query.exec("SELECT message,an,mois,jour,heure,minute,seconde FROM neo_messages WHERE port='" + QString::number(pport) + "' ORDER BY timestamp DESC LIMIT 0,5");
			while(query.next())
			{
				dern = query.value(0).toString() + " (" + tr("Le ") + zero(query.value(3).toInt()) + "/" + zero(query.value(2).toInt()) + "/" + query.value(1).toString() + " " + tr("�") + " " + zero(query.value(4).toInt()) + "h" + zero(query.value(5).toInt()) + ")<br />" + dern;
			}
			dern = "<font color='gray'><em>" + tr("5 derniers messages") + "</em><br />" + dern;
		
			QString derna = "";
                
			if(msgd.size()>0)
			{
				for(int i = 0;i < msgd.size();i++)
				{
					if(msgd[i].at(0)==clients[point].getPseudo() or (msgd[i].at(0) + "[ADMIN]")==clients[point].getPseudo() or (msgd[i].at(0) + "[MODO]")==clients[point].getPseudo() or (msgd[i].at(0) + "[B�TA]")==clients[point].getPseudo())
					{
						derna = "<strong>Messages diff�r�s :</strong><br />" + derna;
						for(int j = 1;j < msgd[i].size();j++)
						{
							derna += msgd[i].at(j) + "<br />";
						}
						msgd.removeAt(i);
					}
				}
			}
			dern = derna + dern;
			dern += "</font><em><strong>" + clients[point].getPseudo() + "</strong> " + tr("vient de se connecter") + "</em>";
			envoyerA(dern,clients[point].getTcp(),true,0); //On lui envoie les 5 derniers messages
		    
			envoyerATous("<em><strong>" + clients[point].getPseudo() + "</strong> " + tr("vient de se connecter") + "</em>",3,0,point);
			std::cout << "Pseudo : " << ch(clients[point].getPseudo()) << std::endl << std::endl;
			clients[point].putTaille(0);
		
			//On incr�mente le fichier
			fnb(true);
			if(pport==10100)
				fnb(true,true);
                
                
			QSqlQuery queryst;
			queryst.exec("INSERT INTO neo_statistiques VALUES('','" + QString::number(pport) + "','" + mys(clients[point].getPseudo()) + "','" + mys(QString::number(clients[point].getVersion())) + "','" + mys(clients[point].getIP()) + "','1','" + QString::number(timestamp()) + "','" + QString::number(nb) + "')");
			if(queryst.lastError().text()!="")
			{
				std::cout << "Erreur : " << ch(query.lastError().text()) << std::endl;
			}
	    }
	//==========
    }
    else if(type==10) //Tape un message
    {
		bool retat;
		in >> retat;
    }
    else if(type==11) //Statut
    {
		int rstatut;
		in >> rstatut;
		clients[point].putStatut(rstatut);
		envoyerATous("<em><strong>" + clients[point].getPseudo() + "</strong> est " + statut(clients[point].getStatut(),true) + "</em>",3);
    }
    
	if(point>=0 and point<clients.size())
	{
		clients[point].putTaille(0);
	}
}

void Serveur::envoyerATous(const QString &message,int type,int c,int fpoint)
{
    QDateTime now2 = QDateTime::currentDateTime();
    QString messageH = "<font color='gray'>(" + zero(now2.time().hour()) + ":" + zero(now2.time().minute()) + ")</font> " + message;
    
    // Pr�paration du paquet
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) 0; // On �crit 0 au d�but du paquet pour r�server la place pour �crire la taille
    out << type; //type
    if(type==-1) //D�connexion
    {
		out << message;
    }
    else if(type==0) //Infos
    {
		out << message;
		out << messageBienvenue;
		out << messagePub;
		QStringList listeConnectes;
        for (int i = 0; i < clients.size(); i++)
        {
			if(clients[i].getStatut()==0)
				listeConnectes << clients[i].getPseudo();
			else
				listeConnectes << clients[i].getPseudo() + " (" + statut(clients[i].getStatut()) + ")";
        }
		out << listeConnectes;
    }
    else if(type==1) //Message
    {
		out << message;
    }
    else if(type==2) //Nombre de connect�s
    {
		out << message;
        out << c;
    }
    else if(type==3) //Liste des connect�s
    {
		out << message;
        QStringList listeConnectes;
        for (int i = 0; i < clients.size(); i++)
        {
			if(clients[i].getStatut()==0)
				listeConnectes << clients[i].getPseudo();
			else
				listeConnectes << clients[i].getPseudo() + " (" + statut(clients[i].getStatut()) + ")";
        }
		out << listeConnectes;
    }
    out.device()->seek(0); // On se replace au d�but du paquet
    out << (quint16) (paquet.size() - sizeof(quint16)); // On �crase le 0 qu'on avait r�serv� par la longueur du message
    
    //Paquet avec l'heure
    QByteArray paquetH;
    QDataStream outH(&paquetH, QIODevice::WriteOnly);
    
    outH << (quint16) 0; // On �crit 0 au d�but du paquet pour r�server la place pour �crire la taille
    outH << type; //type
    if(type==-1) //D�connexion
    {
		outH << messageH;
    }
    else if(type==0) //Infos
    {
		outH << messageH;
		outH << messageBienvenue;
		outH << messagePub;
		QStringList listeConnectes;
        for (int i = 0; i < clients.size(); i++)
        {
			if(clients[i].getStatut()==0)
				listeConnectes << clients[i].getPseudo();
			else
				listeConnectes << clients[i].getPseudo() + " (" + statut(clients[i].getStatut()) + ")";
        }
		outH << listeConnectes;
    }
    else if(type==1) //Message
    {
		outH << messageH;
    }
    else if(type==2) //Nombre de connect�s
    {
		outH << messageH;
        outH << c;
    }
    else if(type==3) //Liste des connect�s
    {
		outH << messageH;
        QStringList listeConnectes;
        for (int i = 0; i < clients.size(); i++)
        {
			if(clients[i].getStatut()==0)
				listeConnectes << clients[i].getPseudo();
			else
				listeConnectes << clients[i].getPseudo() + " (" + statut(clients[i].getStatut()) + ")";
        }
		outH << listeConnectes;
    }
    outH.device()->seek(0); // On se replace au d�but du paquet
    outH << (quint16) (paquet.size() - sizeof(quint16)); // On �crase le 0 qu'on avait r�serv� par la longueur du message

    // Envoi du paquet pr�par� � tous les clients connect�s au serveur
    for (int i = 0; i < clients.size(); i++)
	{
		if(i!=fpoint)
		{
			if(clients[i].getOptHeureMessAff())
				clients[i].getTcp()->write(paquetH);
			else
				clients[i].getTcp()->write(paquet);
		}
    }
    
    //On enregistre le message dans ls bdd
    QDateTime now = QDateTime::currentDateTime();
    QDateTime st = QDateTime::QDateTime(QDate(1970,01,01),QTime(01,00,00));
    QSqlQuery query;
      query.exec("INSERT INTO neo_messages VALUES('','" + QString::number(pport) + "','','" + mys(message) + "','" + QString::number(st.secsTo(now)) + "','" + QString::number(now.date().year()) + "','" +  QString::number(now.date().month()) + "','" + QString::number(now.date().day()) + "','" + QString::number(now.time().hour()) + "','" + QString::number(now.time().minute()) + "','" + QString::number(now.time().second()) + "')");
    if(query.lastError().text()!="")
    {
		std::cout << "Erreur : " << ch(query.lastError().text()) << std::endl;
    }
    
    if(pport==10100)
    {
        QFile f("dern_mess_general");
        f.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate);
        f.write(QByteArray(QString(message + "\n" + QString::number(st.secsTo(now))).toAscii()));
        f.close();
    }
}

void Serveur::envoyerA(const QString &message, QTcpSocket *Tcp, bool simple,int type, int fstatut,bool archiver) //Envoi d'un message � un client pr�cis
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
    out << type; //Type
    if(type==-1) //D�connexion
    {
		out << messagee;
    }
    else if(type==0) //Infos
    {
		out << messagee;
		out << messageBienvenue;
		out << messagePub;
		QStringList listeConnectes;
        for (int i = 0; i < clients.size(); i++)
        {
			if(clients[i].getStatut()==0)
				listeConnectes << clients[i].getPseudo();
			else
				listeConnectes << clients[i].getPseudo() + " (" + statut(clients[i].getStatut()) + ")";
        }
		out << listeConnectes;
    }
    else if(type==1) //Message
    {
		out << messagee;
    }
    else if(type==2) //Nombre de connect�s
    {
		out << messagee;
        out << clients.size();
    }
    else if(type==3) //Liste des connect�s
    {
		out << messagee;
		QStringList listeConnectes;
        for (int i = 0; i < clients.size(); i++)
        {
			if(clients[i].getStatut()==0)
				listeConnectes << clients[i].getPseudo();
			else
				listeConnectes << clients[i].getPseudo() + " (" + statut(clients[i].getStatut()) + ")";
		}
		out << listeConnectes;
    }
    else if(type==6) //Statut modifi�
    {
		out << messagee;
		out << fstatut;
        QStringList listeConnectes;
        for (int i = 0; i < clients.size(); i++)
        {
			if(clients[i].getStatut()==0)
				listeConnectes << clients[i].getPseudo();
			else
				listeConnectes << clients[i].getPseudo() + " (" + statut(clients[i].getStatut()) + ")";
        }
		out << listeConnectes;
    }
    out.device()->seek(0); // On se replace au d�but du paquet
    out << (quint16) (paquet.size() - sizeof(quint16)); // On �crase le 0 qu'on avait r�serv� par la longueur du message


    // Envoi du paquet au client
    Tcp->write(paquet);
    
    if(archiver)
    {
		//On enregistre le message dans ls bdd
		QDateTime now = QDateTime::currentDateTime();
		QDateTime st = QDateTime::QDateTime(QDate(1970,01,01),QTime(01,00,00));
		QSqlQuery query;
		query.exec("INSERT INTO neo_messages VALUES('','" + QString::number(pport-50000) + "','','" + mys(message) + "','" + QString::number(st.secsTo(now)) + "','" + QString::number(now.date().year()) + "','" +  QString::number(now.date().month()) + "','" + QString::number(now.date().day()) + "','" + QString::number(now.time().hour()) + "','" + QString::number(now.time().minute()) + "','" + QString::number(now.time().second()) + "')");
		if(query.lastError().text()!="")
		{
			std::cout << "Erreur : " << ch(query.lastError().text()) << std::endl;
		}
    }
}

//Fonctions sans rapport
QString Serveur::statut(int statutf,bool lower)
{
    if(lower)
		return statuts.at(statutf).toLower();
    else
		return statuts.at(statutf);
}
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
void Serveur::fnb(bool plus,bool general,bool eff)
{
    QString chemin = "nb_connectes";
    if(general)
        chemin = "nb_connectes_general";
    
    //Lecture
    QFile f(chemin);
    f.open(QIODevice::Text | QIODevice::ReadOnly);
    QString contenu = QString(f.readAll());
    f.close();
    
    int nbf = contenu.toInt();
    if(plus)
        nbf++;
    else
        nbf--;
    
    if(eff)
        nbf = 0;
    if(general)
        nbf = clients.size();
    
    //Ecriture
    f.open(QIODevice::Text | QIODevice::WriteOnly | QIODevice::Truncate);
    f.write(QByteArray(QString::number(nbf).toAscii()));
    f.close();
}
