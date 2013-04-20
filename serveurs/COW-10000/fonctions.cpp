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
#include <QtSql>

Serveur::Serveur(int portt)
{
    versionC = 1000;
    versionCch = "";
    versionCp = 0;
    tailleMessage = 0;
    port = portt;
    
    //Textes
    QList<QString> fr;
    fr << "Bienvenue sur le serveur de" << "Certaines informations ne sont pas correctes !" << "Cet �tablissement existe d�j� !" << "Cet �tablissement a d�j� �t� propos�, il est actuellement en attente de validation." << "L'�tablissement a bien �t� ajout� !" << "La demande d'ajout de cet �tablissement a bien �t� prise en compte." << "Cependant, pour que celui-ci apparaisse dans la liste, il doit au pr�alable �tre valid� par un administrateur." << "Comptez entre 30 minutes et 24 heures" << "En attendant, nous vous invitons � discuter sur le serveur g�n�ral de COW." << "Coll�ge" << "Lyc�e" << "Autre";
    
    QList<QString> en;
    en << "Welcome on the server of" << "Some of theses informations are not correct!" << "This school already exists!" << "This School has already been asked, it is in state of validation." << "The school is correctly added!" << "The add of this school has been considered." << "However, for this one to be in the list, it must be validate by an administrator." << "It should take between 30 minutes and 24 hours" << "While you are waiting, we suggest you to chat on the general COW server." << "College" << "High school" << "Other";
    textes << en << fr;
    
    
    
    db = QSqlDatabase::addDatabase("QMYSQL");
      db.setHostName("SERVEUR_SQL");
      db.setDatabaseName("NOM_DE_LA_BASE_DE_DONNEES");
      db.setUserName("UTILISATEUR_DE_LA_BDD");
      db.setPassword("MOT_DE_PASSE");
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
    cout << "D�but de la r�cup�ration de la liste des serveurs" << endl;
    
	/*
	 *Toutes les fonctions relatives � SQL,
	 *les requ�tes � la base de donn�es etc,
	 *m�me les textes sont tr�s relatifs � the-cow.org
	 *et demandent pas mal d'adaptation pour un autre
	 *syst�me je pense. Lorsque j'aurai plus
	 *de temps je modifierai la source de
	 *mani�re � pouvoir l'utiliser plus facilement.
	 */
	
    //Serveurs
    QSqlQuery query;
    query.exec("SELECT nom,ip,port,ville,codep,langage FROM neo_serveurs WHERE valide='1' ORDER BY langage ASC, codep ASC,nom ASC");
    while (query.next())
    {
		QString nom = "Etablissement";
		if(query.value(0).toString()=="COW")
		{
			nom = "COW - Serveur g�n�ral";
		}
		else
		{
			nom = query.value(5).toString() + " : " + query.value(0).toString() + " [" + query.value(3).toString() + "-" + query.value(4).toString() + "]";
		}
		QString ip = query.value(1).toString();
		int por = query.value(2).toInt();

		QProcess::startDetached("./COW-serveur " + QString::number(por) + " " + query.value(5).toString() + " >> logs.log"); //On d�marre tous les serveurs

		cout << "[OK] Serveur \"" << ch(nom) << "\" lanc� sur le port " << ch(QString::number(por)) << endl;
    }
    
    cout << "Erreur : " << ch(query.lastError().text()) << endl;
	cout << endl << "Liste enregistr�e et serveurs lanc�s !" << endl << endl;
     
    //On r�cup�re les infos sur la derni�re version du client qui est sortie
    QSqlQuery vCi;
    vCi.exec("SELECT version,changements,taille FROM programmes WHERE valide='1' ORDER BY version DESC LIMIT 0,1");
    while (vCi.next())
    {
		versionC = vCi.value(0).toInt();
		versionCch = vCi.value(1).toString();
		versionCp = vCi.value(2).toInt();
    }
     cout << "Lancement du serveur..." << endl;
     lancer();
}
//Fonction qui lance le serveur (appel�e une fois la liste r�cup�r�e depuis la bdd)
void Serveur::lancer()
{
    serveurt = new QTcpServer();
    tailleMessage = 0;
    if (!serveurt->listen(QHostAddress::Any, port))
    {
        cout << "Erreur lors du d�marrage : " << ch(serveurt->errorString()) << endl << endl;
        etat = false;
    }
    else
    {
        cout << "Le serveur a bien �t� d�marr� sur le port " << ch(QString::number(port)) << endl << endl;
        etat = true;
        QObject::connect(serveurt, SIGNAL(newConnection()), this, SLOT(nouvelleConnexion()));
    }
}
//Slots
void Serveur::nouvelleConnexion()
{
    QTcpSocket *nouveauClient = serveurt->nextPendingConnection();
    connect(nouveauClient, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(nouveauClient, SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
    Client nouveau(nouveauClient,nouveauClient->peerAddress());
    clients << nouveau;
    cout << "Connexion d'un client" << endl;
}

void Serveur::deconnexionClient()
{
    cout << "D�connexion d'un client" << endl << endl;
    QTcpSocket *socket = qobject_cast<QTcpSocket *>(sender());
    if (socket == 0) // Si par hasard on n'a pas trouv� le client � l'origine du signal, on arr�te la m�thode
        return;

    int point = 0;
    for (int i = 0; i < clients.size(); i++) //Boucle pour trouver le num�ro correspondant au socket
    {
        if(clients[i].getTcp()==socket)
        {
            point = i;
        }
    }
    
    disconnect(clients[point].getTcp(), SIGNAL(disconnected()), this, SLOT(deconnexionClient()));
    disconnect(clients[point].getTcp(), SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    clients.removeAt(point);
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
    
    for (int i = 0; i < clients.size(); i++)
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

	/*Ce syst�me d'utiliser clients[point], de
	 *passer par la QList des clients en fait,
	 *est tr�s archa�que, il vaudrait mieux
	 *utiliser un pointeur sur le client.
	 *Etant press� par le temps je n'ai pas
	 *encore apport� cette modification.
	 */
    clients[point].putTaille(0);
    
    int type=1;
    in >> type;
    cout << "Recu ; Type : " << ch(QString::number(type)) << endl;
    if(type==1) //Infos
    {
		int rversion;
		QString rlangage="en";
		QString ros;
		in >> rversion;
		in >> rlangage;
		in >> ros;
		clients[point].putLangage(rlangage);
		clients[point].putOs(ros);
		clients[point].putVersion(rversion);
		if(rversion<versionC)
		{
			envoyerA(socket,20);
		}
		else
		{
			envoyerA(socket,1);
		}
    }
    else if(type==2) //On veut v�rifier si le pseudo est OK
    {
		bool pseudoe = false;
		QString rpseudo = "";
		in >> rpseudo;
		QRegExp sec1("\\[(ADMIN|MODO|B�TA|BETA)\\]",Qt::CaseInsensitive);
		QRegExp rx("<");
		QRegExp rx2(">");
		QRegExp esp(" ");
		rpseudo.replace(esp,"");
		rpseudo.replace(rx,"&lt;");
		rpseudo.replace(rx2,"&gt;");
		rpseudo.replace(sec1,"");
		QSqlQuery pseudov;
		pseudov.exec("SELECT pseudo FROM membres WHERE pseudo='" + mys(rpseudo) + "'");
		while(pseudov.next())
		{
			pseudoe = true;
		}
		if(pseudoe)
			envoyerA(socket,10);
		else
			envoyerA(socket,11); //Ajouter la v�rification de la disponibilit� du pseudo !
    }
    else if(type==3) //V�rification du mdp
    {
		bool pseudoe = false;
		QString rpseudo = "";
		QString rmdp;
		in >> rpseudo;
		in >> rmdp;
	
		//On crypte le mdp
		rmdp = rmdp; //Ajouter un grain de sel
		QByteArray rmdpb = QCryptographicHash::hash(rmdp.toUtf8(), QCryptographicHash::Sha1); //On peut choisir un autre algorythme
		QString rmdpc;
		rmdpc = rmdpb.toHex();
    
		QRegExp sec1("\\[(ADMIN|MODO|B�TA|BETA)\\]",Qt::CaseInsensitive);
		QRegExp rx("<");
		QRegExp rx2(">");
		QRegExp esp(" ");
		rpseudo.replace(esp,"");
		rpseudo.replace(rx,"&lt;");
		rpseudo.replace(rx2,"&gt;");
		rpseudo.replace(sec1,"");
		QSqlQuery pseudov;
		pseudov.exec("SELECT pseudo FROM membres WHERE pseudo='" + mys(rpseudo) + "' AND pass='" + mys(rmdpc) + "'");
		while(pseudov.next())
		{
			pseudoe = true;
		}
		if(pseudoe)
			envoyerA(socket,11); //Ajouter v�rification disponibilit� pseudo
		else
			envoyerA(socket,12);
	}
	else if(type==5) //Recherche �tablissement
	{
		QString achercher = "";
		in >> achercher;
		cout << ch(achercher) << endl;
		if(achercher!="")
		{
			QList<int> envoi_id;
			QList<QString> envoi_nom;
			QList<QString> envoi_ville;
			QList<QString> envoi_codep;
			QList<QString> envoi_langage;
			QList<QString> envoi_ip;
			QList<int> envoi_port;
			int i = 0;

			QSqlQuery cherche;
			cherche.exec("SELECT id,nom,ville,codep,langage,port FROM neo_serveurs WHERE valide=1 AND nom LIKE '%" + mys(achercher) + "%' ORDER BY codep DESC LIMIT 0,10");
			while (cherche.next())
			{
				i++;
				envoi_id << cherche.value(0).toInt();
				envoi_nom << cherche.value(1).toString();
				envoi_ville << cherche.value(2).toString();
				envoi_codep << cherche.value(3).toString();
				envoi_langage << cherche.value(4).toString();
				envoi_ip << QString("the-cow.org");
				envoi_port << cherche.value(5).toInt();
			}
			if(i>0)
			{
				QByteArray paquet;
				QDataStream out(&paquet, QIODevice::WriteOnly);
	
				out << (quint16) 0;
				out << (int)2;
				out << envoi_id;
				out << envoi_nom;
				out << envoi_ville;
				out << envoi_codep;
				out << envoi_langage;
				out << envoi_ip;
				out << envoi_port;

				out.device()->seek(0);
				out << (quint16) (paquet.size() - sizeof(quint16));
				socket->write(paquet); // On envoie le paquet
			}
		}
    }
    else if(type==10) //On veut ajouter un �tablissement
    {
		QString langue = "en";
		int l = 0;
		langue = clients[point].getLangage();
		l = Langue(langue);
	
		cout << endl << "Informations sur l'ajout d'un �tablissement re�ues" << endl;
		QString aj_nom;
		QString aj_ville;
		int aj_codep;
		QString aj_description;
		QString aj_type;
		bool aj_dedans = true;
		QString aj_ip = clients[point].getIp().toString();
		QString aj_lang = "fr";
		in >> aj_nom;
		in >> aj_ville;
		in >> aj_codep;
		in >> aj_description;
		in >> aj_type;
		in >> aj_dedans;
		aj_lang = langue;
	
		cout << "V�rifications en cours..." << endl;
		cout << "Nom : " << ch(aj_nom) << " - Ville : " << ch(aj_ville) << " - CodeP : " << ch(QString::number(aj_codep)) << " - Description : " << ch(aj_description) << " - Type : " << ch(aj_type) << " - IP : " << ch(aj_ip) << endl; 
		//On effectues des v�rifications
		if(aj_codep==0 or aj_codep>100000 or aj_codep<0)
		{
			envoyerA(socket,3);
			cout << "Le code postal (" << ch(QString::number(aj_codep)) << ") n'est pas correct !" << endl << endl;
			return;
		}
		if(aj_nom=="")
		{
			envoyerA(socket,3);
			cout << "Le nom n'est pas pr�sent !" << endl << endl;
			return;
		}
		if(aj_ville=="")
		{
			envoyerA(socket,3);
			cout << "La ville n'est pas pr�sente !" << endl << endl;
			return;
		}
		if(aj_type=="" or (aj_type!=textes[l].at(9) and aj_type!=textes[l].at(10) and aj_type!=textes[l].at(11)))
		{
			envoyerA(socket,3);
			cout << "Le type est incorrect !" << endl << endl;
			return;
		}
	
		//On r�cup�re le dernier port
		int aj_port = 10099;
		QSqlQuery pp;
		pp.exec("SELECT port FROM neo_serveurs ORDER BY port DESC LIMIT 0,1");
		while(pp.next())
		{
			aj_port = pp.value(0).toInt() + 1;
		}
	
		bool aj_existed = false;
		int aj_valided=0;
		QSqlQuery query;
		query.exec("SELECT id,valide FROM neo_serveurs WHERE ip='" + aj_ip + "'");
		while (query.next())
		{
			//if(query.value(0).toInt()!=0)
			aj_existed = true;
			aj_valided = query.value(1).toInt();
		}
		if(aj_existed)
		{
			if(aj_valided==0)
			{
				envoyerA(socket,5);
			}
			else
			{
				envoyerA(socket,4);
			}
			cout << "Erreur, l'�tablissement existe d�j� !" << endl << endl;
			return;
		}
		cout << "Tout est OK, on ajoute l'�tablissement !" << endl;
		if(aj_dedans)
		{
			QSqlQuery ajouted;
			ajouted.exec("INSERT INTO neo_serveurs VALUES('','" + aj_nom + "','" + aj_ip + "','" + QString::number(aj_port) + "','" + mys(aj_lang) + "','" + aj_ville + "','" + QString::number(aj_codep) + "','" + aj_type + "','" + aj_description + "','<font color=\\'blue\\'><strong>" + textes[l].at(0) + " " + aj_nom + " !</strong></font>','<!-- Begin Ad42_Tag --><style type=\"text/css\">.adHeadline {font: bold 10pt Arial; text-decoration: underline; color: #000000;} .adText {font: normal 10pt Arial; text-decoration: none; color: #000033;}</style><script type=\"text/javascript\" src=\"http://adserver.ad42.com/printZone.aspx?idz=5461&newwin=1\"></script><p /><div><a class=\"adHeadline\" href=\"http://www.ad42.com/zone.aspx?idz=5461&ida=-1\" target=\"_blank\">Votre publicit&eacute; ici ?</a></div><!-- End Ad42_Tag -->','" + aj_ip + "','1','')");
			cout << "Erreur : " << ch(ajouted.lastError().text()) << endl;
		}
		else
		{
			QSqlQuery ajouted;
			ajouted.exec("INSERT INTO neo_serveurs VALUES('','" + QString(aj_nom) + "','','" + QString::number(aj_port) + "','" + mys(aj_lang) + "','" + QString(aj_ville) + "','" + QString::number(aj_codep) + "','" + QString(aj_type) + "','" + QString(aj_description) + "','<font color=\\'blue\\'><strong>" + textes[l].at(0) + " " + QString(aj_nom) + " !</strong></font>','<!-- Begin Ad42_Tag --><style type=\"text/css\">.adHeadline {font: bold 10pt Arial; text-decoration: underline; color: #000000;} .adText {font: normal 10pt Arial; text-decoration: none; color: #000033;}</style><script type=\"text/javascript\" src=\"http://adserver.ad42.com/printZone.aspx?idz=5461&newwin=1\"></script><p /><div><a class=\"adHeadline\" href=\"http://www.ad42.com/zone.aspx?idz=5461&ida=-1\" target=\"_blank\">Votre publicit&eacute; ici ?</a></div><!-- End Ad42_Tag -->','" + QString(aj_ip) + "','','')");
			cout << "Erreur : " << ch(ajouted.lastError().text()) << endl;
		}
		aj_nom = aj_lang + " : " + aj_nom + " [" + aj_ville + "-" + QString::number(aj_codep) + "]";
		cout << "Etablissement ajout� !" << endl << endl;
		if(aj_dedans)
		{
			QProcess::startDetached("./COW-serveur " + QString::number(aj_port) + " " + aj_lang + " >> logs.log");
			cout << "[OK] Serveur \"" << ch(aj_nom) << "\" lanc� sur le port " << ch(QString::number(aj_port)) << endl;
			envoyerA(socket,6); //On informe de l'ajout
		}
		else
		{
			envoyerA(socket,7); //On informe
		}

    }
    else //Le type ne correspond � rien
    {
	//Envoyer une erreur
    }
    clients[point].putTaille(0);
}

void Serveur::envoyerA(QTcpSocket *Tcp, int type) //Envoi d'un message � un client pr�cis
{
    //On r�cup�re le num�ro de la QList, pour avoir la langue
    QString langue;
    for(int i=0; i < clients.size(); i++)
    {
		if(clients[i].getTcp()==Tcp)
		{
			langue = clients[i].getLangage();
		}
    }
    int l = Langue(langue);
    
    // Pr�paration du paquet
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) 0; // On �crit 0 au d�but du paquet pour r�server la place pour �crire la taille
    out << type; //On ajoute le type
    if(type==1) //On cherche � envoyer un �tablissement correspondant � l'ip
    {
        //On va chercher dans la bdd si un �tablissement correspond � cette IP
	cout << "Cherche..." << endl;
	bool etab_trouve = false;
	QSqlQuery etab;
	etab.exec("SELECT id,nom,ville,codep,langage,port FROM neo_serveurs WHERE ip='" + Tcp->peerAddress().toString() + "' AND valide=1");
	while (etab.next())
	{
	    /* Structure
	    int type
	    int rid
	    QString rnom
	    QString rville
	    QString rcode
	    QString rlangage
	    QString rip (ip du serveur sur lequel se connecter)
	    int rport
	    */
	    etab_trouve = true;
	    out << etab.value(0).toInt();
	    out << QString(etab.value(1).toString());
	    out << QString(etab.value(2).toString());
	    out << QString(etab.value(3).toString());
	    out << QString(etab.value(4).toString());
	    out << QString("the-cow.org");
	    out << etab.value(5).toInt();
	}
	if(!etab_trouve)
	    return;
    }
    else if(type==3)
    {
		out << QString(textes[l].at(1));
    }
    else if(type==4)
    {
		out << QString(textes[l].at(2));
    }
    else if(type==5)
    {
		out << QString(textes[l].at(3));
    }
    else if(type==6)
    {
		out << QString(textes[l].at(4));
    }
    else if(type==7)
    {
		out << QString(textes[l].at(5) + "<br />" + textes[l].at(6) + "<br /><em>(" + textes[l].at(7) + ")</em><br /><br /><strong>" + textes[l].at(8) + "</strong>");
    }
    else if(type==20)
    {
		out << versionv(versionC);
		out << versionCch;
		out << versionCp;
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
QString Serveur::versionv(int v) //En fonction du num�ro de version, on retourne celui-ci, mais s�par� par des points
{
	if(v==0)
	{
		return "Beta";
	}
	else
	{
		if(floor(v/10)==0)
		{
			return "0.0.0." + QString::number(v);
		}
		else
		{
			if(floor(v/100)==0)
			{
				return "0.O." + QString::number(floor(v/10)) + "." + QString::number(v%10);
			}
			else
			{
				if(floor(v/1000)==0)
				{
					return "0." + QString::number(floor((v%1000)/100)) + "." + QString::number(floor((v%100)/10)) + "." + QString::number(((v%100)-floor((v%100)/10)*10));
				}
				else
				{
				    
				    return QString::number(floor(v/1000)) + QString::number(floor((v%1000)/100)) + "." + QString::number(floor((v%100)/10)) + "." + QString::number(v%10);
				}
			}
		}
	}
}
int Serveur::Langue(QString langue)
{
    if(langue=="en")
	return 0;
    else if(langue=="fr")
	return 1;
    else
	return 0;
}
	
