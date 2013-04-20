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
#include "preFen.h"

preFen::preFen(QString pseudoo,QString langagef)
{   
    setupUi(this);
    
    connectee = false;
    tentatives = 0;
    
    //La langue
    langage = langagef;
    
	//Informations de connexion au serveur COW-10000
    serveur_ip = "serveur-recherche.the-cow.org";
    serveur_port = 10000;
	
    taille = 0;
	envois = 0;
    
	//Options
    QSettings settings("cow", "preFen");
    t_pseudo->setText(settings.value("User/pseudo").toString()); //Pseudo sauvegardé
	if(!settings.contains("User/pass")) //Mot de passe sauvegardé
	{
		settings.setValue("User/pass", "");
		settings.setValue("User/dsauvpass", true);
	}

    if(pseudoo!="") //Un pseudo a été passé en argument, on s'est donc déjà connecté puis déconnecté
    {
		t_pseudo->setText(pseudoo); //On affiche le pseudo qu'on avait déjà choisi
		question = true; //On indique qu'on a déjà proposé la MAJ
    }
    else
		question = false; //On n'a pas encore proposé la MAJ
    
    t_serveur_liste->addItem(tr("fr : COW - Serveur general"));
    t_serveur_liste->addItem(tr("Add my school"));
    t_serveur_liste->addItem(tr("Personalised"));
    t_serveur_liste->setCurrentRow(0);
    
    liste(); //Connexion au serveur
    
    connect(t_serveur_liste, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(selection(QListWidgetItem*)));
}
void preFen::on_b_connexion_clicked()
{
    //On fait différentes vérifications
    if(t_pseudo->text()=="")
    {
        QMessageBox::warning(this, tr("Error"), tr("You have to choose a nickname!"));
        return;
    }

    //On va maintenant envoyer le mot de passe au serveur pour savoir si c'est OK
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);
    
    out << (quint16) 0;
    out << (int)2;
    out << t_pseudo->text();

    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet
}
void preFen::on_t_pseudo_returnPressed()
{
    if(b_connexion->isEnabled())
    {
	    on_b_connexion_clicked();
    }
}
void preFen::on_t_ips_returnPressed()
{
    if(b_connexion->isEnabled())
    {
	    on_b_connexion_clicked();
    }
}
void preFen::on_t_serveur_liste_itemDoubleClicked(QListWidgetItem *item)
{
    if(b_connexion->isEnabled())
    {
	    on_b_connexion_clicked();
    }
}
void preFen::liste() //Connexion au serveur COW-10000
{
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));
    socket->abort(); // On désactive les connexions précédentes s'il y en a
    socket->connectToHost(serveur_ip, serveur_port); // On se connecte au serveur demandé
    chargement->setText(tr("Connexion in progress..."));
    tentatives++;
}
void preFen::tenter() //Reconnexion au serveur COW-10000
{
    a_ten->setText("");
    socket->abort(); // On désactive les connexions précédentes s'il y en a
    socket->connectToHost(serveur_ip, serveur_port); // On se connecte au serveur demandé
    chargement->setText(tr("Connexion in progress..."));
    tentatives++;
}
void preFen::connecte()
{
    chargement->setText(tr("Connexion succesfull."));
    connectee = true;
    tentatives = 0;
    
    //On envoie les infos
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);
    
    out << (quint16) 0;
    out << (int)1;
    out << VERSION;
    out << langage;
    out << QString(OS);

    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet
	
    b_connexion->setEnabled(true);
    connect(t_serveur_recherche,SIGNAL(textEdited(QString)),this,SLOT(chercher_modif(QString)));
}
// Ce slot est appelé lorsqu'on est déconnecté du serveur
void preFen::deconnecte()
{
    chargement->setText(tr("Disconnected from server!"));
}
// Ce slot est appelé lorsqu'il y a une erreur
void preFen::erreurSocket(QAbstractSocket::SocketError erreur)
{
    switch(erreur) // On affiche un message différent selon l'erreur qu'on nous indique
    {
        case QAbstractSocket::HostNotFoundError:
            chargement->setText("<font color='red'><em>" + tr("ERROR: Impossible to find the server! It is possible the server is down.") + "<br />" + tr("Try to reconnect later.") + "</em></font>");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            chargement->setText("<font color='red'><em>" + tr("ERROR: The server refused the connexion. It generally means that this one is in maintenance for a moment.") + "<br />" + tr("Try to reconnect in 5 minutes.") + "</em>");
            break;
        case QAbstractSocket::RemoteHostClosedError:
            chargement->setText("<font color='red'><em>" + tr("ERROR: the server stopped the connexion.") + "</em></font>");
            break;
        default:
            chargement->setText("<font color='red'><em>" + tr("ERROR") + " : " + socket->errorString() + "</em></font>");
    }

    if(tentatives<5)
    {
        a_ten->setText(tr("New connexion attempt") + " (" + QString::number(tentatives) + ") " + tr("in 5 seconds..."));
        QTimer::singleShot(5000, this, SLOT(tenter()));
    }
    else
    {
        chargement->setText("<strong>" + tr("Impossible to connect on") + "<br /> " + tr("server in 5 attempts.") + "<br />" + tr("It is possible that this one is") + "<br />" + tr("down.") + "<br />" + tr("Try again later.") + "</strong>");
    }
}
void preFen::chercher_modif(QString achercher)
{
    envois++;
    QTimer::singleShot(600, this, SLOT(chercher_etab()));
}
void preFen::chercher_etab()
{
    QString achercher = t_serveur_recherche->text();
    envois--;

    if(envois==0)
    {
		envoie_achercher = achercher;
		QByteArray paquet;
		QDataStream out(&paquet, QIODevice::WriteOnly);
	
		out << (quint16) 0;
		out << (int)5;
		out << achercher;

		out.device()->seek(0);
		out << (quint16) (paquet.size() - sizeof(quint16));
		socket->write(paquet); // On envoie le paquet
    }
}
void preFen::donneesRecues()
{
    chargement->setText(tr("Waiting to receive data..."));
    /* Même principe que lorsque le serveur reçoit un paquet :
    On essaie de récupérer la taille du message
    Une fois qu'on l'a, on attend d'avoir reçu le message entier (en se basant sur la taille annoncée tailleMessage)
    */
    QDataStream in(socket);

    if (taille == 0)
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16))
             return;

        in >> taille;
    }

    if (socket->bytesAvailable() < taille)
        return;


	/*Architecture de base d'un paquet du serveur de discussion - COW-10000
	 *	quint16	taille	- taille du paquet
	 *	int		type	- type définissant la suite de l'architecture
	 #TYPES
	 1								- Un établissement qui correspond à notre IP a été trouvé
		int				etab_id		- ID de l'établissement
		QString			etab_nom	- Nom de l'établissement
		QString			etab_ville	- Ville de l'établissement
		QString			etab_codep	- Code postal de l'établissement
		QString			etab_langage- Langage de l'établissement
		QString			etab_ip		- IP du serveur
		QString			etab_port	- Port du serveur
	 2								- Liste des établissements correspond à notre recherche
		QList<int>		etab_id		- IDs des établissements
		QList<QString>	etab_nom	- Noms des étlissements
		QList<QString>	etab_ville	- Villes des l'établissements
		QList<QString>	etab_codep	- Codes postaux de établissements
		QList<QString>	etab_langage- Langages de établissements
		QList<QString>	etab_ip		- IPs des serveurs
		QList<QString>	etab_port	- Ports des serveurs
	 3								- Erreur lors de l'ajout de l'établissement ; données incorrectes
		QString			messErr		- Message d'erreur
	 4								- Erreur lors de l'ajout de l'établissement ; établissement identique déjà existant
		QString			messErr		- Message d'erreur
	 5								- Erreur lors de l'ajout de l'établissement ; établissement identique déjà en attente de validation
		QString			messErr		- Message d'erreur
	 6								- Information sur l'ajout de l'établissement; celui-ci a été correctement ajouté
		QString			messErr		- Message d'information
	 7								- Information sur l'ajout de l'établissement; celui-ci est en attente de validation
		QString			messErr		- Message d'information
	 10								- Le pseudo indiqué nécessite un mot de passe
	 11								- Le compte indiqué ne nécessite pas de mot de passe, ou celui indiqué est correct > connexion possible
	 12								- Le mot de passe est incorrect
	 13								- {non implanté} Le pseudo indiqué est déjà utilisé
	 20								- Une mise à jour est disponible
		QString			nversionn	- Numéro de la nouvelle version
		QString			nversionch	- Changements apportés par cette nouvelle version (xHTML)
		int				nversionp	- Poids de cette nouvelle version
	 */
	
    // Si on arrive jusqu'à  cette ligne, on peut récupérer le message entier
    int type;
    in >> type;
     //On peut maintenant traiter les données
    if(type==1) //Un établissement correspondant à l'IP a été trouvé
    {
		int etab_id; //ID de l'établissement
		QString etab_nom;
		QString etab_ville;
		QString etab_codep;
		QString etab_langage;
		QString etab_ip; //IP du serveur
		int etab_port;
	
		in >> etab_id;
		in >> etab_nom;
		in >> etab_ville;
		in >> etab_codep;
		in >> etab_langage;
		in >> etab_ip;
		in >> etab_port;
		//On stocke ces infos dans la liste des établissements
		etabs_id.clear();
		etabs_nom.clear();
		etabs_ville.clear();
		etabs_codep.clear();
		etabs_langage.clear();
		etabs_ip.clear();
		etabs_port.clear();
		
		etabs_id << etab_id;
		etabs_nom << etab_nom;
		etabs_ville << etab_ville;
		etabs_codep << etab_codep;
		etabs_langage << etab_langage;
		etabs_ip << etab_ip;
		etabs_port << etab_port;
	
		t_serveur_liste->clear();
		etab_actuel_complet = etab_nom + " [" + etab_ville + "-" + etab_codep + "]";
	
		//La liste étant effacée, on rajoute les entrées d'origine + l'établissement trouvé
		t_serveur_liste->addItem(tr("fr : COW - Serveur general"));
		t_serveur_liste->addItem(etab_actuel_complet);
		t_serveur_liste->addItem(tr("Add my school"));
		t_serveur_liste->addItem(tr("Personalised"));
	
		t_serveur_liste->setCurrentRow(1); //L'établissement trouvé se trouve en 2e
    }
    else if(type==2) //Envoi de la liste des établissements correspondant à la recherche
    {	
		t_serveur_liste->clear();
	
		etabs_id.clear();
		etabs_nom.clear();
		etabs_ville.clear();
		etabs_codep.clear();
		etabs_langage.clear();
		etabs_ip.clear();
		etabs_port.clear();
	
		in >> etabs_id;
		in >> etabs_nom;
		in >> etabs_ville;
		in >> etabs_codep;
		in >> etabs_langage;
		in >> etabs_ip;
		in >> etabs_port;

		t_serveur_liste->addItem(tr("fr : COW - Serveur general"));
	
		for(int i = 0;i < etabs_nom.size();i++)
		{
			t_serveur_liste->addItem(QString(etabs_nom.at(i) + " [" + etabs_ville.at(i) + "-" + etabs_codep.at(i) + "]"));
		}
		t_serveur_liste->addItem(tr("Add my school"));
		t_serveur_liste->addItem(tr("Personalised"));

		t_serveur_liste->setCurrentRow(0);
    }
    else if(type==3)
    {
		QString messErr = tr("Error : the error is inknown");
		in >> messErr;
		QMessageBox::warning(this, tr("Error while adding the school"), messErr);
    }
    else if(type==4)
    {
		QString messErr = tr("Error : the error is inknown");
		in >> messErr;
		QMessageBox::warning(this, tr("Error while adding the school"), messErr);
		ajouter_etab->setMaximumHeight(0);
		t_serveur_liste->setCurrentRow(0);
    }
    else if(type==5)
    {
		QString messErr = tr("Error : the error is inknown");
		in >> messErr;
		QMessageBox::warning(this, tr("Error while adding the school"), messErr);
		ajouter_etab->setMaximumHeight(0);
		t_serveur_liste->setCurrentRow(0);
    }
    else if(type==6)
    {
		QString messErr = tr("Error : the error is inknown");
		in >> messErr;
		QMessageBox::warning(this, tr("Information"), messErr);
		ajouter_etab->setMaximumHeight(0);
    }
    else if(type==7)
    {
		QString messErr = tr("Error : the error is inknown");
		in >> messErr;
		QMessageBox::warning(this, tr("Information"), messErr);
		ajouter_etab->setMaximumHeight(0);
		t_serveur_liste->setCurrentRow(0);
    }
    else if(type==10) //On a envoyé le pseudo demandé, le serveur indique que celui-ci nécessite un mot de passe
    {
		QSettings settings("cow", "preFen");
		if(settings.value("User/pseudo").toString()==t_pseudo->text() and settings.value("User/pass").toString()!="") //Mot de passe sauvegardé
		{
			mdp = settings.value("User/pass").toString();
			QByteArray paquet;
            QDataStream out(&paquet, QIODevice::WriteOnly);
            
            out << (quint16) 0;
            out << (int)3;
            out << t_pseudo->text();
            out << settings.value("User/pass").toString();
			
            out.device()->seek(0);
            out << (quint16) (paquet.size() - sizeof(quint16));
            socket->write(paquet); // On envoie le paquet
		}
		else
		{
			if(settings.value("User/pseudo").toString()!=t_pseudo->text())
			{
				settings.setValue("User/pass","");
			}
			
			bool ok = false;
			mdp = QInputDialog::getText(this, tr("Password"), tr("The account is registered. Password:"), QLineEdit::Password, QString(), &ok);
			if (ok && !mdp.isEmpty())
			{
				//On envoie le mdp au serveur
				QByteArray paquet;
				QDataStream out(&paquet, QIODevice::WriteOnly);
            
				out << (quint16) 0;
				out << (int)3;
				out << t_pseudo->text();
				out << mdp;
    
				out.device()->seek(0);
				out << (quint16) (paquet.size() - sizeof(quint16));
				socket->write(paquet); // On envoie le paquet
			}
			else
			{
				QMessageBox::critical(this, tr("Password"), tr("There is no password!"));
			}
		}
    }
    else if(type==11) //mdp ok OR pas de mdp AND pseudo dispo
    {
		//Sauvegarde du mot de passe
		QSettings settings("cow", "preFen");
		if(mdp!="" and settings.value("User/dsauvpass").toBool()==true and mdp!=settings.value("User/pass").toString()) //On propose de sauvegarder le mdp si il y en a un, qu'on n'a pas dis qu'on ne voulait pas sauver les mdp et qu'il s'agit d'un mdp non stocké
		{
			QMessageBox *sauvmdp = new QMessageBox(QMessageBox::Question,tr("Save the password?"),tr("Would you like that COW save your password? You will don't need to tape this at the next lauch."),QMessageBox::Yes|QMessageBox::No);
			sauvmdp->addButton(tr("No and ask me never more"),QMessageBox::DestructiveRole);
			int repsauv = sauvmdp->exec();
			if(repsauv==QMessageBox::Yes)
			{
				settings.setValue("User/pass",mdp);
			}
			else if(repsauv==QMessageBox::No)
			{
				settings.setValue("User/pass","");
				settings.setValue("User/dsauvpass",true);
			}
			else
			{
				settings.setValue("User/pass","");
				settings.setValue("User/dsauvpass",false);
			}
			
		}
		
		connexionF(mdp);
    }
    else if(type==12) //Le mdp est incorrect
    {
        bool ok = false;
		QSettings settings("cow", "preFen");
		settings.setValue("User/pass","");
		mdp = QInputDialog::getText(this, tr("Password"), tr("The password is incorrect. Password:"), QLineEdit::Password, QString(), &ok);
		if (ok && !mdp.isEmpty())
		{
			//On envoie le mdp au serveur
			QByteArray paquet;
            QDataStream out(&paquet, QIODevice::WriteOnly);
            
            out << (quint16) 0;
            out << (int)3;
            out << t_pseudo->text();
            out << mdp;
    
            out.device()->seek(0);
            out << (quint16) (paquet.size() - sizeof(quint16));
            socket->write(paquet); // On envoie le paquet
		}
		else
		{
			QMessageBox::critical(this, tr("Password"), tr("There is no password!"));
		}
    }
    else if(type==13) //Le pseudo est déjà utilisé
    {
		/* Ce type est renvoyé lorsque le pseudo indiqué est déjà utilisé par quelqu'un de connecté.
		 Ce système n'est pas encore inclus sur le serveur COW-10000 ; tel que le système
		 de serveurs est consu il faudrait mettre en place une communication, une connexion TCP
		 entre le serveur COW-10000 et tous les serveurs COW-serveur.
		 Ce type devient obsolète si on met en place un système déconnectant l'utilisateur
		 déjà connecté avec le même pseudo.
		 */
        QMessageBox::critical(this, tr("Nickname"), tr("This nickname is already in use. Please choose an other one."));
    }
    else if(type==20) //MAJ disponible
    {
		QString nversion;
		QString nversionch;
		int nversionp;
		in >> nversion;
		in >> nversionch;
		in >> nversionp;
		if(!question)
		{
			int reponse = QMessageBox::question(this, tr("Update"), tr("An update") + " (" + nversion + ") " + tr("is avaible!") + "<br />" + tr("The news are:") +  "<br />" + nversionch + "<br /><br />" + tr("The size is ") + QString::number(floor(nversionp/1000)) + " ko. " + tr("This update is automatic.") + "<br /><br /><strong>" + tr("Do you want to dowload and install this update?") + "</strong>", QMessageBox::Yes | QMessageBox::No);

			if(reponse==QMessageBox::Yes) //On ca donc lancer le programme de mise à jour - lancement différent suivant chaque OS
			{
			#ifdef LINUX
				QProcess::startDetached("update");
			#endif
			#ifdef WINDOWS
				QProcess::startDetached("update.exe");
			#endif
			#ifdef MAC
				QProcess::startDetached(QCoreApplication::applicationDirPath() + "/../../../update.app/Contents/MacOS/./maj");
			#endif
			}
			else if(reponse==QMessageBox::No)
			{
				QMessageBox::critical(this, tr("Update"), tr("You will be able to do this update the next time you launch COW."));
			}
		}
		question = true;
    }
    
    taille = 0;
    chargement->setText("");
}
void preFen::selection(QListWidgetItem *nomf)
{
    QString nom = nomf->text();
    if(nom==tr("Personalised"))
    {
        t_ips->setMaximumHeight(16777215);
        t_ports->setMaximumHeight(16777215);
		ajouter_etab->setMaximumHeight(0);
    }
    else if(nom==tr("Add my school"))
    {
		ajouter_etab->setMaximumHeight(16777215);
        t_ips->setMaximumHeight(0);
        t_ports->setMaximumHeight(0);
    }
    else
    {
        t_ips->setMaximumHeight(0);
        t_ports->setMaximumHeight(0);
		ajouter_etab->setMaximumHeight(0);
    }
}
void preFen::on_aj_envoyer_clicked()
{

    if(aj_codep->text().toInt()==0 or aj_codep->text().toInt()>100000 or aj_codep->text().toInt()<0)
    {
		QMessageBox::warning(this, tr("Error"), tr("The ZIP code is not correct!"));
        return;
    }
    if(aj_nom->text()=="")
    {
		QMessageBox::warning(this, tr("Error"), tr("You have not entered any name!"));
        return;
    }
    if(aj_ville->text()=="")
    {
		QMessageBox::warning(this, tr("Error"), tr("You have not entered any city!"));
        return;
    }
    if(aj_type->currentText()=="" or (aj_type->currentText()!=tr("College") and aj_type->currentText()!=tr("High school") and aj_type->currentText()!=tr("Other")))
    {
		QMessageBox::warning(this, tr("Error"), tr("The type is not correct!"));
        return;
    }
    
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    QString oaj_nom = aj_nom->text();
    QString oaj_ville = aj_ville->text();
    int oaj_codep =  aj_codep->text().toInt();
    QString oaj_description = aj_description->text();
    QString oaj_type = aj_type->currentText();
    bool oaj_dedans = aj_dedans->isChecked();
    
    out << (quint16) 0;
    out << 10;
    out << oaj_nom; //On envoie le nom
    out << oaj_ville; //On envoie la ville
    out << oaj_codep; //On envoie le code postal
    out << oaj_description; //On envoie la description
    out << oaj_type; //On envoie le type
    out << oaj_dedans; //On envoie le bolléen, si on est dans l'établissement en ce moment même, ou bien si non

    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet
}
void preFen::connexionF(QString mdp) //Fermeture de preFen et ouverture de Fenetre
{
    iserveur serveur;
    if(t_serveur_liste->currentItem()->text()==tr("fr : COW - Serveur general"))
    {
        serveur.nom = "COW";
        serveur.ip = "the-cow.org";
        serveur.port = 10100;
    }
    else if(t_serveur_liste->currentItem()->text()==tr("Personalised"))
    {
        serveur.nom = tr("External Server");
        serveur.ip = t_ips->text();
        serveur.port = t_ports->value();
    }
    else if(t_serveur_liste->currentItem()->text()==tr("Add my school"))
    {
        return;
    }
    else
    {
		int i = t_serveur_liste->currentRow()-1;
		serveur.nom = etabs_nom.at(i);
		serveur.ip = etabs_ip.at(i);
		serveur.port = etabs_port.at(i);
    }
	
    socket->abort(); //On se déconnecte du serveur
    //On stocke le pseudo pour le prochain lancement
    QSettings settings("cow", "preFen");
    settings.setValue("User/pseudo", t_pseudo->text());

    
    Fenetre *fenetre = new Fenetre(t_pseudo->text(),mdp,&serveur,langage);
    fenetre->show();
    this->close();
    this->destroy(true,true);
}

