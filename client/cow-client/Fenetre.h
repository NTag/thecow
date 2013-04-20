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
#ifndef HEADER_FENETRE
#define HEADER_FENETRE

#include <QtGui>
#include <QtNetwork>
#include <QSystemTrayIcon>
#include "ui_Fenetre.h"
#include "preFen.h"
#include "structures.h"

//=====Smileys - by carado
#define texte1 " :) "
#define texte2 " ;) "
#define texte3 " :( "
#define texte4 " ^^ "
#define texte5 " :euh: "
#define texte6 " :D "
#define texte7 " :p "
#define texte8 " :lol: "
#define texte9 " :oh: "
#define texte10 " :-o "
#define texte11 " :@ "
#define texte12 " oO "
//==========

//=====Infos sur la version
#define OS "mac"
#define MAC
#define VERSION 2000
//==========

class Fenetre : public QMainWindow, private Ui::Fenetre
{
    Q_OBJECT

    public:
        Fenetre(QString p_pseudo,QString mdpf,iserveur* p_iserveur,QString langagef);

    private :
        void connexion();
        void surligne(QString pseudo2);
        void nettoyer();
        void boutons(bool action = TRUE,bool etat = TRUE); //action : si on doit activer (TRUE) ou désactiver (FALSE) les boutons -- etat : si on doit activer (TRUE) ou désactiver (FALSE) le bouton déconnexion (en fait si on est déjà connécté ou si on est en cours de connexion).
		void trayActions();
		void createTray();
		QString supprHtml(QString text);

    private slots:
        void on_boutonEnvoyer_clicked();
        void on_boutonDeconnexion_clicked();
        void on_message_returnPressed();
		void on_boutonCacherListes_clicked();
		void on_i_statut_currentIndexChanged(int statut);
		void on_tabs_discussions_tabCloseRequested(int index);
	
        void donneesRecues();
        void connecte();
        void deconnecte();
        void erreurSocket(QAbstractSocket::SocketError erreur);
	
		void donneesRecuesD();
		void connecteD();
		void deconnecteD();
		void erreurSocketD(QAbstractSocket::SocketError erreur);
	
        void lclic(QModelIndex index);
		void dclic(QModelIndex index);
        void reconnexion();
		void reconnexionD();
		void keyReleaseEvent(QKeyEvent *event); //Lorsqu'on appuie sur les touches haut et bas pour voir les messages envoyés - ça ne fonctionne pas sous MacOSX, je n'ai pas cherché pourquoi
		QWidget* onglet();
		bool event(QEvent *event);
		void iconActivated(QSystemTrayIcon::ActivationReason);
		void messageClicked();
		//=====Smileys - by carado
        void mettreSmiley1();
		void mettreSmiley2();
		void mettreSmiley3();
		void mettreSmiley4();
		void mettreSmiley5();
		void mettreSmiley6();
		void mettreSmiley7();
		void mettreSmiley8();
		void mettreSmiley9();
		void mettreSmiley10();
		void mettreSmiley11();
		void mettreSmiley12();

    private:
        QTcpSocket *socket; // Représente le serveur
        quint16 tailleMessage;
	
        int nbc; //Nombre de connectes
        bool conn;
        QStringListModel *modele;
		int tentatives;
        bool e_surligne;
		QString mdp;
		QString langage;
		QStringList messagesEnvoyes;
		int messagesEnvoyesP;
		QString messagesEnvoyesS;
		bool changeCurrentIndex;
	
		//=====Tray Icon
		QAction *minimizeAction;
		QAction *maximizeAction;
		QAction *restoreAction;
		QAction *quitAction;

		QSystemTrayIcon *trayIcon;
		QMenu *trayIconMenu;
	
		bool affMess; //Afficher un message lors de la réduction dans le tray icon
		//==========
	
		//=====Options
		QAction *no_clignoter;
		QAction *no_message;
		QAction *opt_reduire; //Si COW doit se réduire ou non en trayIcon (certains gestionnaires graphiques n'ont pas de zone de notification)
		//==========

		QString pseudo;
		int serveurPort;
		QString serveurNom;
		QString serveurIP;
	
		bool ignorerp;
	
		//=====COW-discussions
		QTcpSocket *socketD; // Représente le serveur
		quint16 tailleMessageD;
		QList<int> ldiscussions;
		int tentativesD;
		//==========
	
    protected:
        void closeEvent(QCloseEvent *event);
};

#endif
