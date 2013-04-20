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
#ifndef PREFEN_H_INCLUDED
#define PREFEN_H_INCLUDED

#include <QtGui>
#include <QtNetwork>
#include "ui_preFen.h"
#include "Fenetre.h"
#include "structures.h"

//=====Infos sur la version
#define OS "mac"
#define MAC
#define VERSION 2000
//==========

class preFen : public QMainWindow, private Ui::preFen
{
    Q_OBJECT

    public:
        preFen(QString pseudoo="",QString langagef="en");

    private:
        void liste();
        void maj_liste();

    private slots:
        void on_b_connexion_clicked();
        void donneesRecues();
        void connecte();
        void deconnecte();
        void erreurSocket(QAbstractSocket::SocketError erreur);
        void tenter();
        void selection(QListWidgetItem *nomf);
		void on_t_serveur_liste_itemDoubleClicked(QListWidgetItem *item);
		void on_aj_envoyer_clicked();
		void on_t_pseudo_returnPressed();
		void on_t_ips_returnPressed();
		void connexionF(QString mdp="");
		void chercher_modif(QString achercher);
		void chercher_etab();

    private:
        QTcpSocket *socket; // Représente le serveur
        quint16 taille;
	
        QString serveur_ip;
        int serveur_port;
	
        bool connectee;
        int tentatives;

		QString langage;
		bool question;

		int envois;
		QString envoie_achercher;
		QString etab_actuel_complet;
		QList<int> etabs_id;
		QList<QString> etabs_nom;
		QList<QString> etabs_ville;
		QList<QString> etabs_codep;
		QList<QString> etabs_langage;
		QList<QString> etabs_ip;
		QList<int> etabs_port;
	
		QString mdp;

};

#endif // PREFEN_H_INCLUDED
