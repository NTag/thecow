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
#include "Fenetre.h"

Fenetre::Fenetre(QString p_pseudo,QString mdpf,iserveur* p_iserveur,QString langagef)
{
    setupUi(this);
	
    //======Smileys - by carado
    QMenu *menu = new QMenu;
	QMenu *menuS = menu->addMenu(QIcon(":/smileys/smile.png"),"Smileys");
	QAction *smiley1 = menuS->addAction(QIcon(":/smileys/smile.png"),texte1);
	QAction *smiley2 = menuS->addAction(QIcon(":/smileys/clin.png"),texte2);
	QAction *smiley3 = menuS->addAction(QIcon(":/smileys/triste.png"),texte3);
	QAction *smiley4 = menuS->addAction(QIcon(":/smileys/hihi.png"),texte4);
	QAction *smiley5 = menuS->addAction(QIcon(":/smileys/unsure.png"),texte5);
	QAction *smiley6 = menuS->addAction(QIcon(":/smileys/heureux.png"),texte6);
	QAction *smiley7 = menuS->addAction(QIcon(":/smileys/langue.png"),texte7);
	QAction *smiley8 = menuS->addAction(QIcon(":/smileys/rire.png"),texte8);
	QAction *smiley9 = menuS->addAction(QIcon(":/smileys/huh.png"),texte9);
	QAction *smiley10 = menuS->addAction(QIcon(":/smileys/siffle.png"),texte10);
	QAction *smiley11 = menuS->addAction(QIcon(":/smileys/mechant.png"),texte11);
	QAction *smiley12 = menuS->addAction(QIcon(":/smileys/blink.png"),texte12);
	boutonm->setMenu(menu);
	connect(smiley1, SIGNAL(triggered()), this, SLOT(mettreSmiley1()));
	connect(smiley2, SIGNAL(triggered()), this, SLOT(mettreSmiley2()));
	connect(smiley3, SIGNAL(triggered()), this, SLOT(mettreSmiley3()));
	connect(smiley4, SIGNAL(triggered()), this, SLOT(mettreSmiley4()));
	connect(smiley5, SIGNAL(triggered()), this, SLOT(mettreSmiley5()));
	connect(smiley6, SIGNAL(triggered()), this, SLOT(mettreSmiley6()));
	connect(smiley7, SIGNAL(triggered()), this, SLOT(mettreSmiley7()));
	connect(smiley8, SIGNAL(triggered()), this, SLOT(mettreSmiley8()));
	connect(smiley9, SIGNAL(triggered()), this, SLOT(mettreSmiley9()));
	connect(smiley10, SIGNAL(triggered()), this, SLOT(mettreSmiley10()));
	connect(smiley11, SIGNAL(triggered()), this, SLOT(mettreSmiley11()));
	connect(smiley12, SIGNAL(triggered()), this, SLOT(mettreSmiley12()));
    //==========
	
    //=====Notifications
    QMenu *m_options = new QMenu;
	
	opt_reduire = new QAction(tr("Hide COW"),this);
	opt_reduire->setToolTip(tr("Hide COW when when it's reduced"));
	opt_reduire->setCheckable(true);
	if(QSystemTrayIcon::isSystemTrayAvailable() and QSystemTrayIcon::supportsMessages() and OS!="mac") //On désactive par défaut sous Mac, car si Growl n'est pas installé les messages ne s'afficheront pas, et donc on n'est plus averti
		opt_reduire->setChecked(true);
	else
		opt_reduire->setChecked(false);

	m_options->addAction(opt_reduire);
	
	QMenu *m_o_notifications = m_options->addMenu("Notifications");
	
	//Actions
	no_clignoter = new QAction(tr("Blink in task bar"),this);
	no_clignoter->setToolTip(tr("Blink in task bar on new message recieved if COW is reduced"));
	no_clignoter->setCheckable(true);
	no_clignoter->setChecked(true);
	
	no_message = new QAction(tr("Show messages"),this);
	no_message->setToolTip(tr("Show a tooltip with the message if COW is reduced"));
	no_message->setCheckable(true);
	no_message->setChecked(true);
	
	m_o_notifications->addAction(no_clignoter);
	m_o_notifications->addAction(no_message);
	
	i_options->setMenu(m_options);
    //==========

	//COW-serveur
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));
	tailleMessage = 0;
	tentatives = 0;
	serveurPort = p_iserveur->port;
    serveurNom = p_iserveur->nom;
    serveurIP = p_iserveur->ip;
	
	//COW-discussions
	socketD = new QTcpSocket(this);
    connect(socketD, SIGNAL(readyRead()), this, SLOT(donneesRecuesD()));
    connect(socketD, SIGNAL(connected()), this, SLOT(connecteD()));
    connect(socketD, SIGNAL(disconnected()), this, SLOT(deconnecteD()));
    connect(socketD, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocketD(QAbstractSocket::SocketError)));
	socketD->connectToHost("the-cow.org", 10050);
	tailleMessageD = 0;
	tentativesD = 0;
	
    nbc = 0;
    mdp = mdpf;
    conn = false;
    pseudo = p_pseudo;
        
    e_surligne = false; //Surlignage des messages ; qui est désactivé car contenait trop de bugs et qui se faisait lorsqu'on double clic sur qqn
	
	//Pour l'historique des messages avec les touches HAUT et BAS
    messagesEnvoyes.clear();
    messagesEnvoyesP = 0;
    messagesEnvoyesS = "";
    changeCurrentIndex = true;
	ignorerp = false;
	
	
	
    //Concernant la langue
    if(langagef=="fr")
		langage = "fr";
    else if(langagef=="en")
		langage = "en";
    else
		langage = "en";
    
    i_pseudo->setText(pseudo);
    tabs_discussions->setTabText(0,serveurNom);
    
	connect(listecon, SIGNAL(clicked(QModelIndex)), this, SLOT(lclic(QModelIndex)));
    connect(listecon, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(dclic(QModelIndex)));

	//=====TRAY ICON
	affMess = true;
	trayActions();
	createTray();
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));
	connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));
	//==========
	
    //=====Options
    QSettings settings("cow", "Fenetre");
    if(settings.contains("Options/Notifications/clignoter_barre") and settings.contains("Options/Notifications/afficher_bulle") and settings.contains("Options/reduire"))
    {
		no_clignoter->setChecked(settings.value("Options/Notifications/clignoter_barre").toBool());
		no_message->setChecked(settings.value("Options/Notifications/afficher_bulle").toBool());
		no_message->setChecked(settings.value("Options/reduire").toBool());
    }
    //==========
	
	
	ldiscussions.clear(); //On efface le contenu de la liste des discussions
	ldiscussions << -1;
	
	connexion(); //On se connecte au serveur
}
void Fenetre::trayActions()
{
    minimizeAction = new QAction(tr("Mi&nimize"), this);
    connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));


    restoreAction = new QAction(tr("&Show"), this);
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));
}
void Fenetre::createTray()
{
    trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(minimizeAction);
    trayIconMenu->addAction(restoreAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    QIcon icon_vache(":/cow-v0.8-transparent.png");
    trayIcon = new QSystemTrayIcon(icon_vache,this);
    trayIcon->setContextMenu(trayIconMenu);

    trayIcon->show();
}
void Fenetre::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason==QSystemTrayIcon::DoubleClick)
    {
		this->showNormal();
		this->raise();
		this->activateWindow();
    }
}
void Fenetre::messageClicked()
{
    this->showNormal();
	this->raise();
	this->activateWindow();
}

bool Fenetre::event(QEvent *event)
{
    if(event->type()==QEvent::WindowStateChange)
    {
		if (this->windowState()==Qt::WindowMinimized and trayIcon->isVisible() and opt_reduire->isChecked()) {
			if(affMess)
			{
				QMessageBox::information(this,tr("Systray"),tr("COW will keep running "
															   "in the system tray. To "
															   "show cow,double-click "
															   "on the tray icon"));
				affMess = false;
			}
			event->ignore();
			QTimer::singleShot(100, this, SLOT(hide())); //Ceci peut paraître un peu bizarre, indiquer un délai avant de cacher COW, mais essayez de l'enlever et vous obtiendrai une belle fenêtre blanche qui ne se cache pas sous Windows --'
			return false;
		}
		else
		{
			return QMainWindow::event(event);
		}
    }
    else
    {
		return QMainWindow::event(event);
    }
}

// Tentative de connexion au serveur
void Fenetre::connexion()
{
    // On annonce sur la fenêtre qu'on est en train de se connecter
    listeMessages->append("<em>" + tr("Connexion attempt...") + "</em>");
    boutons(false,true);

    socket->abort(); // On désactive les connexions précédentes s'il y en a
    socket->connectToHost(serveurIP, serveurPort); // On se connecte au serveur demandé
    tentatives++;
}

void Fenetre::on_boutonDeconnexion_clicked()
{
    socket->abort(); // On désactive les connexions
	socketD->abort();
    preFen *rec = new preFen(pseudo,langage);
    rec->show();
    this->close();
    trayIcon->hide();
    trayIcon->deleteLater();
    this->destroy(true,true);
}
// Envoi d'un message au serveur
void Fenetre::on_boutonEnvoyer_clicked()
{
    // On prépare le paquet à envoyer
    QString messageAEnvoyer = message->text();
    
    //On va vérifier si il ne s'agit pas d'une commande
    QRegExp commande_msge("^(/((m[a-z][a-z])|([a-z]s[a-z])|([a-z][a-z]g)))"); //Commande /m** ou /*s* ou /**g ; pour corriger les fautes de frappe de /msg
    QRegExp commande_version("^/(version)|(about)$");
    QRegExp commande_clear("^/clear$");
    QRegExp commande_quit("^/quit$");
    QRegExp commande_close("^/close$");
    if(commande_version.indexIn(message->text())!=-1) //Il s'agit de la commande /version
    {
		QMessageBox::about(this,tr("About COW"),tr("<strong>COW - Chat Over the World</strong><br /><br />Created and administered by <strong>Basile Bruneau</strong> (basilebruneau@gmail.com)<br />Software's version : <strong>2.0.0.0</strong><br />Library Qt's version : <strong>4.6</strong><br /><br />Website : <a title='See the website' href='http://www.the-cow.org'>http://www.the-cow.org</a><br /><br /><br />Thanks to* : dodo, ju., carado, OdoniX, iMeee, joedu12, Djipi2000, SRLKilling, Methconi and more !<br /><br /><font size='1'>*list incomplet</font>"));
    }
    else if(commande_clear.indexIn(message->text())!=-1) //Il s'agit de la commande /clear
		listeMessages->clear();
    else if(commande_quit.indexIn(message->text())!=-1) //Il s'agit de la commande /quit
		socket->abort();
    else if(commande_close.indexIn(message->text())!=-1) //Il s'agit de la commande /close
		this->close();
    else //Il ne s'agit pas d'une commande
    {
		if(commande_msge.indexIn(message->text())!=-1 and commande_msge.cap(1)!="/msg") //On demande si la commande voulue n'était pas /msg
		{
			QString commande = commande_msge.cap(1);
			int reponse = QMessageBox::question(this, tr("You entered the ") + commande + tr(" command"), tr("You entered the ") + commande + tr (" command. Did you meant /msg ?"), QMessageBox::Yes | QMessageBox::No);
    
			if (reponse == QMessageBox::Yes) //On modifie la commande pour avoir /msg
			{
				messageAEnvoyer.replace(commande_msge, "/msg");
			}
		}
		QByteArray paquet;
		QDataStream out(&paquet, QIODevice::WriteOnly);

		out << (quint16) 0;
		out << (int)1; //type
		out << messageAEnvoyer; //rmessage
		if(tabs_discussions->currentIndex()>0) //On rajoute l'ID de la conversation
		{
			out << ldiscussions.at(tabs_discussions->currentIndex());
		}
		
		out.device()->seek(0);
		out << (quint16) (paquet.size() - sizeof(quint16));

		if(tabs_discussions->currentIndex()>0)
			socketD->write(paquet); // On envoie le paquet à COW-discussions
		else
			socket->write(paquet); // On envoie le paquet à COW-serveur
    }
    messagesEnvoyes.prepend(messageAEnvoyer);
    messagesEnvoyesP = 0;
    message->clear(); // On vide la zone d'écriture du message
    message->setFocus(); // Et on remet le curseur à l'intérieur
}
void Fenetre::on_i_statut_currentIndexChanged(int statut)
{
    if(changeCurrentIndex)
    {
		QByteArray paquet;
		QDataStream out(&paquet, QIODevice::WriteOnly);

		out << (quint16) 0;
		out << (int)11; //type
		out << statut; //on envoie le nouveau statut
		out.device()->seek(0);
		out << (quint16) (paquet.size() - sizeof(quint16));

		socket->write(paquet); // On envoie le paquet
    }
    else
		changeCurrentIndex = true;
}
void Fenetre::on_tabs_discussions_tabCloseRequested(int index) //On veut fermer un onglet
{
	if(index<=0)
		return;
	
	QByteArray paquet;
	QDataStream out(&paquet, QIODevice::WriteOnly);
	
	out << (quint16) 0;
	out << (int)6; //type - indique qu'on s'en va de la conversation
	out << ldiscussions.at(index); //ID de la discussion
	out.device()->seek(0);
	out << (quint16) (paquet.size() - sizeof(quint16));
	
	socketD->write(paquet); // On envoie le paquet
	
	ldiscussions.removeAt(index);
	
	tabs_discussions->widget(index)->deleteLater();
	tabs_discussions->removeTab(index);
}
// Appuyer sur la touche Entrée a le même effet que cliquer sur le bouton "Envoyer"
void Fenetre::on_message_returnPressed()
{
    on_boutonEnvoyer_clicked();
}
void Fenetre::on_boutonCacherListes_clicked()
{
    if(listes->isVisible())
    {
		listes->hide();
		boutonCacherListes->setText(tr("Show"));
		boutonCacherListes->setToolTip(tr("Show connected people list"));
    }
    else
    {
		listes->show();
		boutonCacherListes->setText(tr("Hide"));
		boutonCacherListes->setToolTip(tr("Hide connected people list"));
    }
}
// On a reçu un paquet (ou un sous-paquet)
void Fenetre::donneesRecues()
{
    /* Même principe que lorsque le serveur reçoit un paquet :
    On essaie de récupérer la taille du message
    Une fois qu'on l'a, on attend d'avoir reçu le message entier (en se basant sur la taille annoncée tailleMessage)
    */
    QDataStream in(socket);

    if (tailleMessage == 0)
    {
        if (socket->bytesAvailable() < (int)sizeof(quint16))
             return;

        in >> tailleMessage;
    }

    if (socket->bytesAvailable() < tailleMessage)
        return;

	/*Architecture de base d'un paquet du serveur de discussion - COW-serveur
	 * quint16	taille	- taille du paquet
	 * int		type	- type définissant la suite de l'architecture
	 * QString	message	- message recu (sauf pour le type 5)
	 #TYPES
	 -1					- Déconnexion du client
	 0	QString:messageBienvenue	QString:messagePub	QStringList:listeConnectes	- Informations (le petit message de bienvenue, la publicité, la liste des connectés)
	 1					- Message uniquement
	 2	int:nbc			- Nombre de connectés (presque ou plus du tout utilisé ; on passe par type=3)
	 3	QStringList:listeConnectes	- Liste des connectés
	 5					- {non implanté} Petit message genre "3 personnes sont en train de taper"
	 6	int:statut	QStringList:listeConnectes	- Le serveur a modifié notre statut et nous indique donc le nouveau, avec la nouvelle liste des connectés
	 */
	
    // Si on arrive jusqu'à cette ligne, on peut récupérer le message entier
    int type;
    QString messageRecu = "";
    in >> type;
    if(type==-1) //Déconnexion
    {
		in >> messageRecu;
		socket->abort();
    }
    else if(type==0) //Infos
    {
		QString messageBienvenue;
		QString messagePub;
		in >> messageRecu;
		in >> messageBienvenue;
		in >> messagePub;
		listeMessages->append(messageBienvenue);
		t_Pub->setText(messagePub);
	
		QStringList listeConnectes;
        in >> listeConnectes;
	
        modele = new QStringListModel(listeConnectes);
        listecon->setModel(modele);
	
		Dnbc->setText(QString::number(listeConnectes.size()));
    }
    else if(type==1) //Message
    {
		in >> messageRecu;
    }
    else if(type==2) //Nombre de connectés
    {
		in >> messageRecu;
        in >> nbc;
        Dnbc->setText(QString::number(nbc));
    }
    else if(type==3) //Liste des connectés
    {
		QStringList listec;
		in >> messageRecu;
        QStringList listeConnectes;
        in >> listeConnectes;
	
        modele = new QStringListModel(listeConnectes);
        listecon->setModel(modele);
	
		Dnbc->setText(QString::number(listeConnectes.size()));
    }
    else if(type==5) //Petit message (=truc est en train d'écrire)
    {
		QString rmessInfos;
		in >> rmessInfos;
		//On voit qu'ensuite il n'y a pas de traitement ; ce système n'est pas encore en place
    }
    else if(type==6) //Statut modifié et envoyé par le serveur
    {
		int statut;
		QStringList listec;
		in >> messageRecu;
		in >> statut;
        QStringList listeConnectes;
        in >> listeConnectes;
	
        modele = new QStringListModel(listeConnectes);
        listecon->setModel(modele);
	
		Dnbc->setText(QString::number(listeConnectes.size()));
	
		if(statut>=0 and statut<=2)
		{
			changeCurrentIndex = false;
			i_statut->setCurrentIndex(statut);
		}
    }
    
    if(messageRecu!="")
    {
		listeMessages->append(messageRecu);
		if(QApplication::activeWindow()==0 and no_message->isChecked())
		{
			trayIcon->showMessage(tr("New message"),supprHtml(messageRecu),QSystemTrayIcon::Information,5000);
		}
		if(no_clignoter->isChecked())
		{
			QApplication::alert(this, 0);
		}
    }
    // On remet la taille du message à 0 pour pouvoir recevoir de futurs messages
    tailleMessage = 0;
    nettoyer();
}

// Ce slot est appelé lorsque la connexion au serveur a réussi
void Fenetre::connecte()
{
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);

    out << (quint16) 0;
    out << (int)2; //(int)type
    out << (int)VERSION; //(int)rversion
    out << pseudo; //(QString)rpseudo
    out << mdp; //(QString)rmdp
    out << langage; //(QString)rlangage
    out << QString(OS); //(QString)ros;
    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet


    listeMessages->append(tr("<em>Connexion succesful!</em>"));
    boutons(true,false);
    tentatives = 0;
}

// Ce slot est appelé lorsqu'on est déconnecté du serveur
void Fenetre::deconnecte()
{
    listeMessages->append(tr("<em>Disconnected from server</em>"));
    boutons(false,false);
    nbc = 0;
    Dnbc->setText(QString::number(nbc));
}

// Ce slot est appelé lorsqu'il y a une erreur
void Fenetre::erreurSocket(QAbstractSocket::SocketError erreur)
{
    switch(erreur) // On affiche un message différent selon l'erreur qu'on nous indique
    {
        case QAbstractSocket::HostNotFoundError:
            listeMessages->append("<font color='red'><em>" + tr("ERROR: Impossible to find the server! It is possible the server is down.") + "<br />" + tr("Try again later.") + "</em></font>");
            break;
        case QAbstractSocket::ConnectionRefusedError:
            listeMessages->append("<font color='red'><em>" + tr("ERROR: The server refused the connexion. It generally means that this one is in maintenance for a moment.") + "<br />" + tr("Try to reconnect in 5 minutes.") + "</em>");
            break;
        case QAbstractSocket::RemoteHostClosedError:
            listeMessages->append("<font color='red'><em>" + tr("ERROR: Server stopped the connexion.") + "</em></font>");
            nbc = 0;
            Dnbc->setText(QString::number(nbc));
            break;
        default:
            listeMessages->append("<font color='red'><em>" + tr("ERROR") + " : " + socket->errorString() + "</em></font>");
    }

    boutons(false,false);
    nettoyer();
    if(tentatives<5)
    {
        QTimer::singleShot(5000, this, SLOT(reconnexion()));
        listeMessages->append("<em>" + tr("Connexion attempt") + " (" + QString::number(tentatives) + ") " + tr("in 5 seconds...") + "</em>");
    }
    else
    {
        listeMessages->append("<br /><br /><strong>" + tr("Impossible to connect on the server with 5 attempts. This one must be down for a moment. Tray again later.") + "</strong>");
    }
}

void Fenetre::donneesRecuesD()
{
    /* Même principe que lorsque le serveur reçoit un paquet :
	 On essaie de récupérer la taille du message
	 Une fois qu'on l'a, on attend d'avoir reçu le message entier (en se basant sur la taille annoncée tailleMessage)
	 */
    QDataStream in(socketD);
	
    if (tailleMessageD == 0)
    {
        if (socketD->bytesAvailable() < (int)sizeof(quint16))
			return;
		
        in >> tailleMessageD;
    }
	
    if (socketD->bytesAvailable() < tailleMessageD)
        return;
	
	
    /*Architecture de base d'un paquet du serveur de discussion - COW-discussions
	 * quint16	taille	- taille du paquet
	 * int		id		- id de la discussion
	 * QString	message	- message
	 * int		type	- type définissant la suite de l'architecture
	 #TYPES
		-1				-Déconnexion du client
		1				-Message uniquement
		3	QStringList	-Liste des connectés
	 */
	
	int id;
	QString messageRecu = "";
    int type;
    
	in >> id; //On réccupère l'ID de la discussion
	in >> messageRecu; //On récupère le message
    in >> type; //On récupère le type
	
    if(type==-1) //Déconnexion
    {
		tabs_discussions->widget(ldiscussions.indexOf(id))->findChild<QTextBrowser *>()->append(messageRecu);
		socketD->abort();
    }
	else if(type==1) //Message
	{
		tabs_discussions->widget(ldiscussions.indexOf(id))->findChild<QTextBrowser *>()->append(messageRecu);
	}
	else if(type==3) //Liste des connectés
	{
		QStringList listeco;
		in >> listeco;
		if(ldiscussions.indexOf(id)==-1)
		{
			QString nomconv = "2 personnes";
			for(int i=0;i<listeco.size();i++)
			{
				if(listeco.at(i)!=pseudo)
					nomconv = listeco.at(i);
			}
			ldiscussions << id;
			int ind = tabs_discussions->addTab(onglet(), nomconv);
		}
		tabs_discussions->widget(ldiscussions.indexOf(id))->findChild<QTextBrowser *>()->append(messageRecu);
		
        QStringListModel *modeleD = new QStringListModel(listeco);
        tabs_discussions->widget(ldiscussions.indexOf(id))->findChild<QListView *>()->setModel(modeleD);
		tabs_discussions->widget(ldiscussions.indexOf(id))->findChildren<QLabel *>().last()->setText(QString::number(listeco.size()));
	}
    
    // On remet la taille du message à 0 pour pouvoir recevoir de futurs messages
    tailleMessageD = 0;
    //nettoyer(); //Etait utilisé pour le surlignage des messages
}

// Ce slot est appelé lorsque la connexion au serveur a réussi - on va maintenant lui envoyé les informations de base
void Fenetre::connecteD()
{
	tentativesD = 0;
	
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);
	
    out << (quint16) 0;
    out << (int)2; //(int)type
    out << (int)VERSION; //(int)rversion
    out << pseudo; //(QString)rpseudo
    out << mdp; //(QString)rmdp
    out << langage; //(QString)rlangage
    out << QString(OS); //(QString)ros;
    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));
	
    socketD->write(paquet); // On envoie le paquet
}

// Ce slot est appelé lorsqu'on est déconnecté du serveur
void Fenetre::deconnecteD()
{
    listeMessages->append("<em>" + tr("Disconnected from the server for private discussion. You will not using the private discussion. Try in 2 minutes to close and restart COW.") + "</em>"); //Vous avez été déconnecté du serveur de la discussion privée. Aussi vous ne pourrez plus discuter de façon privée. Essayez dans 2 minutes de fermer puis relancer COW.
}

// Ce slot est appelé lorsqu'il y a une erreur
void Fenetre::erreurSocketD(QAbstractSocket::SocketError erreur)
{
    if(tentativesD<5)
    {
        QTimer::singleShot(5000, this, SLOT(reconnexionD()));
    }
    else
    {
		QString messageE = tr("Impossible to connect on the private discussion server in 5 attempts. You will not being able to use the private discussion. Sorry.") + "<br /><br />"; //Impossible de se connecter au serveur permettant la discussion privée en 5 tentatives. Vous ne pourrez donc pas discuter de façon privée avec vos amis, désolé.
		
		switch(erreur) // On affiche un message différent selon l'erreur qu'on nous indique
		{
			case QAbstractSocket::HostNotFoundError:
				messageE += tr("ERROR: Impossible to find the server! It is possible the server is down.");
				break;
			case QAbstractSocket::ConnectionRefusedError:
				messageE += tr("ERROR: The server refused the connexion. It generally means that this one is in maintenance for a moment.");
				break;
			case QAbstractSocket::RemoteHostClosedError:
				messageE += tr("ERROR: Server stopped the connexion.");
				break;
			default:
				messageE += tr("ERROR") + " : " + socket->errorString();
		}
		
		QMessageBox::critical(this,"Impossible to connect to private discussion",messageE);
    }
}

void Fenetre::reconnexion()
{
    connexion();
}
void Fenetre::reconnexionD()
{
	tentativesD++;
	socketD->abort();
    socketD->connectToHost("the-cow.org", 10050);
}
//Les 2 fonctions suivantes allaient avec des signaux mais désactivés car ces fonctions bugs la plupart du temps
void Fenetre::surligne(QString pseudo2)
{
    QRegExp rx("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">" + pseudo2 + "</span>");
    QString ta = listeMessages->toHtml();

    ta.replace(rx,"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; background-color:white\"><span style=\" font-weight:600;\">" + pseudo2 + "</span>");

    listeMessages->clear();
    listeMessages->append(ta);

    e_surligne = true;
}
void Fenetre::nettoyer()
{
    if(e_surligne)
    {
        QRegExp rx("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; background-color:#ffffff;\"><span style=\" font-weight:600;\">");
        QString ta = listeMessages->toHtml();
	
        ta.replace(rx,"<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\"><span style=\" font-weight:600;\">");
	
	listeMessages->clear();
	listeMessages->append(ta);

        e_surligne = false;
    }
}
//Pour enlever les balises xHTML dans les messages qu'on va afficher dans la zone de notification
QString Fenetre::supprHtml(QString text)
{
    QRegExp rx1("(</?strong>)|(<font color='(.{0,10})'>)|(</font>)|(</?em>)|(<br />)|(<a title='.+' href='.+'>)|(</a>)");
    text.replace(rx1,"");
    QRegExp rx2("(<img src='.{0,20}' alt='(.{0,30})' />)");
    text.replace(rx2,"\\2");
	text.replace("&gt;",">");
	text.replace("&lt;","<");
    return text;
}
void Fenetre::lclic(QModelIndex index)
{
    QVariant elementSelectionne = modele->data(index, Qt::DisplayRole);
    //nettoyer();
    //surligne(elementSelectionne.toString());
}
void Fenetre::dclic(QModelIndex index)
{
	/* Ancien code avant d'avoir les discussions par onglets
    QVariant elementSelectionne = modele->data(index, Qt::DisplayRole);
    message->setText("/msg " + elementSelectionne.toString() + " ");
    message->setFocus();
	 */
	//On envoie la demande de nouvelle conversation au serveur
	QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);
	
    out << (quint16) 0;
    out << (int)3; //(int)type
    out << (QString)modele->data(index, Qt::DisplayRole).toString();
    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));
	
    socketD->write(paquet); // On envoie le paquet
}
void Fenetre::boutons(bool action,bool etat)
{
    if(action)
    {
	boutonDeconnexion->setText(tr("Disconnexion"));
	boutonDeconnexion->setEnabled(true);

    }
    else
    {
	boutonDeconnexion->setText(tr("Reconnexion"));
	boutonDeconnexion->setEnabled(true);

    }

    if(etat)
    {
        boutonDeconnexion->setEnabled(false);
    }
}
void Fenetre::keyReleaseEvent(QKeyEvent *event)
{
    if(message->hasFocus())
    {
		if(event->key()==Qt::Key_Up or event->key()==Qt::Key_Down)
		{
			if(messagesEnvoyesP==0 and (messagesEnvoyes.size()==0 or message->text()!=messagesEnvoyesS))
				messagesEnvoyesS = message->text();
		
			QStringList temp = messagesEnvoyes;
			temp.prepend(messagesEnvoyesS);

			if(event->key()==Qt::Key_Up)
				messagesEnvoyesP++;
			if(event->key()==Qt::Key_Down)
				messagesEnvoyesP--;
	    
			if(messagesEnvoyesP>=temp.size())
				messagesEnvoyesP = temp.size()-1;
			if(messagesEnvoyesP<0)
				messagesEnvoyesP = 0;
	    
			if(temp.size()>0)
				message->setText(temp.at(messagesEnvoyesP));
		}
    }
}
void Fenetre::closeEvent(QCloseEvent *event)
{
   //On stocke le pseudo pour le prochain lancement
    QSettings settings("cow", "Fenetre");
    settings.setValue("Options/Notifications/clignoter_barre", no_clignoter->isChecked());
    settings.setValue("Options/Notifications/afficher_bulle", no_message->isChecked());
    settings.setValue("Options/reduire", opt_reduire->isChecked());
    event->accept();
}
QWidget* Fenetre::onglet()
{
	QWidget *odiscussion;
    QHBoxLayout *horizontalLayout_3;
    QTextBrowser *listeMessages;
    QWidget *listes;
    QVBoxLayout *verticalLayout_6;
    QVBoxLayout *verticalLayout_4;
    QHBoxLayout *horizontalLayout_7;
    QLabel *la_nbc;
    QLabel *Dnbc;
    QListView *listecon;
	
	tab_etablissement = new QWidget();
	tab_etablissement->setObjectName(QString::fromUtf8("tab_etablissement"));
	tab_etablissement->setStyleSheet(QString::fromUtf8(""));
	horizontalLayout_8 = new QHBoxLayout(tab_etablissement);
	horizontalLayout_8->setObjectName(QString::fromUtf8("horizontalLayout_8"));
	horizontalLayout_8->setContentsMargins(0, 6, 0, 0);
	odiscussion = new QWidget(tab_etablissement);
	odiscussion->setObjectName(QString::fromUtf8("odiscussion"));
	horizontalLayout_3 = new QHBoxLayout(odiscussion);
	horizontalLayout_3->setContentsMargins(0, 0, 0, 0);
	horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
	listeMessages = new QTextBrowser(odiscussion);
	listeMessages->setObjectName(QString::fromUtf8("listeMessages"));
	listeMessages->setStyleSheet(QString::fromUtf8("background-image: url(\":/px.png\");\n"
												   "border: 2px solid black;"));
	listeMessages->setOpenExternalLinks(true);
	
	horizontalLayout_3->addWidget(listeMessages);
	
	listes = new QWidget(odiscussion);
	listes->setObjectName(QString::fromUtf8("listes"));
	listes->setMaximumSize(QSize(200, 16777215));
	listes->setAutoFillBackground(false);
	verticalLayout_6 = new QVBoxLayout(listes);
	verticalLayout_6->setContentsMargins(0, 0, 0, 0);
	verticalLayout_6->setObjectName(QString::fromUtf8("verticalLayout_6"));
	verticalLayout_4 = new QVBoxLayout();
	verticalLayout_4->setObjectName(QString::fromUtf8("verticalLayout_4"));
	horizontalLayout_7 = new QHBoxLayout();
	horizontalLayout_7->setObjectName(QString::fromUtf8("horizontalLayout_7"));
	la_nbc = new QLabel(listes);
	la_nbc->setObjectName(QString::fromUtf8("la_nbc"));
	la_nbc->setMaximumSize(QSize(180, 16777215));
	
	horizontalLayout_7->addWidget(la_nbc);
	
	Dnbc = new QLabel(listes);
	Dnbc->setObjectName(QString::fromUtf8("Dnbc"));
	Dnbc->setMaximumSize(QSize(10, 16777215));
	
	horizontalLayout_7->addWidget(Dnbc);
	
	
	verticalLayout_4->addLayout(horizontalLayout_7);
	
	listecon = new QListView(listes);
	listecon->setObjectName(QString::fromUtf8("listecon"));
	listecon->setEnabled(true);
	listecon->setMinimumSize(QSize(100, 50));
	listecon->setMaximumSize(QSize(200, 16777215));
	listecon->setStyleSheet(QString::fromUtf8("QListView {\n"
											  "     show-decoration-selected: 1; /* make the selection span the entire width of the view */\n"
											  "background-image: url(\":/px.png\");\n"
											  "border: 2px solid black;\n"
											  " }\n"
											  "\n"
											  " QListView::item:alternate {\n"
											  "     background: #EEEEEE;\n"
											  " }\n"
											  "\n"
											  " QListView::item:selected {\n"
											  "     border: 1px solid #6a6ea9;\n"
											  " }\n"
											  "\n"
											  " QListView::item:selected:!active {\n"
											  "     background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
											  "                                 stop: 0 #ABAFE5, stop: 1 #8588B2);\n"
											  " }\n"
											  "\n"
											  " QListView::item:selected:active {\n"
											  "     background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
											  "                                 stop: 0 #6a6ea9, stop: 1 #888dd9);\n"
											  " }\n"
											  "\n"
											  " QListView::item:hover {\n"
											  "     background: qlineargradient(x1: 0, y1: 0, x2: 0, y2: 1,\n"
											  "                                 stop: 0 #FAFBFE, stop: 1 #DCDEF1);\n"
											  " }"));
	listecon->setEditTriggers(QAbstractItemView::NoEditTriggers);
	listecon->setDragEnabled(false);
	listecon->setProperty("isWrapping", QVariant(false));
	
	verticalLayout_4->addWidget(listecon);
	
	
	verticalLayout_6->addLayout(verticalLayout_4);
	
	
	horizontalLayout_3->addWidget(listes);

	horizontalLayout_8->addWidget(odiscussion);
	
	la_nbc->setText(QApplication::translate("Fenetre", "Current connected poeple:", 0, QApplication::UnicodeUTF8));
	Dnbc->setText(QString());
	
	connect(listecon, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(dclic(QModelIndex)));
	
	return tab_etablissement;
}

//=====Smileys - by carado
void Fenetre::mettreSmiley1() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte1);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte1).size());
}
void Fenetre::mettreSmiley2() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte2);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte2).size());
}
void Fenetre::mettreSmiley3() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte3);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte3).size());
}
void Fenetre::mettreSmiley4() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte4);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte4).size());
}
void Fenetre::mettreSmiley5() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte5);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte5).size());
}
void Fenetre::mettreSmiley6() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte6);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte6).size());
}
void Fenetre::mettreSmiley7() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte7);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte7).size());
}
void Fenetre::mettreSmiley8() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte8);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte8).size());
}
void Fenetre::mettreSmiley9() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte9);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte9).size());
}
void Fenetre::mettreSmiley10() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte10);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte10).size());
}
void Fenetre::mettreSmiley11() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte11);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte11).size());
}
void Fenetre::mettreSmiley12() {
    QString mt = message->text();
    int pos = message->cursorPosition();
    mt.insert(pos,texte12);
    message->setText(mt);
    message->setFocus();
    message->setCursorPosition(pos + QString(texte12).size());
}

