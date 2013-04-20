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
#include "fenetre.h"

Fenetre::Fenetre(QString flangue)
{   
    setupUi(this);
    
    serveur_ip = "the-cow.org";
    serveur_port = 10001;
    taille = 0;
    bytesDeja = 0;
    ctbytes = 0;
    
    langue = flangue;
    
    adresse = "http://www.the-cow.org/maj/";
    connectee = false;
    
    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(donneesRecues()));
    connect(socket, SIGNAL(connected()), this, SLOT(connecte()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(deconnecte()));
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(erreurSocket(QAbstractSocket::SocketError)));
    socket->abort(); // On désactive les connexions précédentes s'il y en a
    socket->connectToHost(serveur_ip, serveur_port); // On se connecte au serveur demandé
}
void Fenetre::connecte()
{
    //On envoie la version au serveur
    QByteArray paquet;
    QDataStream out(&paquet, QIODevice::WriteOnly);
    
    out << (quint16) 0;
    out << (int)1;
    out << (int)VERSION;
    out << langue;
    out << QString(OS);

    out.device()->seek(0);
    out << (quint16) (paquet.size() - sizeof(quint16));

    socket->write(paquet); // On envoie le paquet

    connectee = true;
    
    http = new QHttp(this);
    
     connect(http, SIGNAL(requestFinished(int, bool)),
             this, SLOT(httpRequestFinished(int, bool)));
     connect(http, SIGNAL(dataReadProgress(int, int)),
             this, SLOT(updateDataReadProgress(int, int)));

    
}
// Ce slot est appelé lorsqu'on est déconnecté du serveur
void Fenetre::deconnecte()
{
    connectee = false;
}

// Ce slot est appel? lorsqu'il y a une erreur
void Fenetre::erreurSocket(QAbstractSocket::SocketError erreur)
{
/*
    switch(erreur) // On affiche un message différent selon l'erreur qu'on nous indique
    {
        case QAbstractSocket::HostNotFoundError:
            //Impossible de trouver le serveur !
            break;
        case QAbstractSocket::ConnectionRefusedError:
            //Le serveur a refusé la connexion
            break;
        case QAbstractSocket::RemoteHostClosedError:
            //Le serveur a coupé la connexion
            break;
        default:
            //chargement->setText("<font color='red'><em>" + tr("ERREUR") + " : " + socket->errorString() + "</em></font>");
    }
    */

}
void Fenetre::donneesRecues()
{
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


    // Si on arrive jusqu'à cette ligne, on peut récupérer les données
    
    in >> nbv;
    in >> vfichiers;
    in >> vnb_f;
    in >> vch;
    in >> vpoids;
    in >> code;
    
    taille = 0;
    
    if(nbv<=VERSION)
    {
		QMessageBox::critical(this, tr("Error"), tr("No update is avaible.","Aucune mise à jour n'est disponible pour le moment."));
		this->close();
		return;
    }
    
    d_nbf->setText(QString::number(vnb_f));
    d_cha->setText(vch);
    d_barre->setMaximum(vpoids);
    d_nversion->setText(versionv(nbv));
    
    adresse = adresse + versionv(nbv) + "/";
    courantf = 0;
    
    telecharge(adresse + vfichiers.at(0));
}
void Fenetre::telecharge(QString fichier)
 {
	 QUrl url(fichier);
	 QFileInfo fileInfo(url.path());
	#ifdef MAC
		QString fileName = QCoreApplication::applicationDirPath() + "/../../../fichiers/" + fileInfo.fileName();
	#endif
	#ifndef MAC
		QString fileName = QCoreApplication::applicationDirPath() + "/fichiers/" + fileInfo.fileName();
	#endif
     

	file = new QFile(fileName);
     
	if (!file->open(QIODevice::WriteOnly)) {
		QMessageBox::information(this, "HTTP",
								tr("Impossible to save the file %1: %2.")
								.arg(fileName).arg(file->errorString()));
		delete file;
		file = 0;
		return;
	}

	QHttp::ConnectionMode mode = url.scheme().toLower() == "https" ? QHttp::ConnectionModeHttps : QHttp::ConnectionModeHttp;
	http->setHost(url.host(), mode, url.port() == -1 ? 0 : url.port());

	if (!url.userName().isEmpty())
		http->setUser(url.userName(), url.password());

	httpRequestAborted = false;
	
	QByteArray path = QUrl::toPercentEncoding(url.path(), "!$&'()*+,;=:@/");
	if (path.isEmpty())
		path = "/";
	httpGetId = http->get(path, file);

}
void Fenetre::updateDataReadProgress(int bytesRead, int totalBytes)
{
    if (httpRequestAborted)
	return;

    d_barre->setValue(bytesRead + bytesDeja);
    ctbytes = totalBytes;
}
 void Fenetre::httpRequestFinished(int requestId, bool error)
 {
     if (requestId != httpGetId)
         return;
     if (httpRequestAborted) {
         if (file) {
             file->close();
             file->remove();
             delete file;
             file = 0;
         }
     }

     if (requestId != httpGetId)
         return;

     file->close();

     if (error) {
         file->remove();
         QMessageBox::information(this, tr("HTTP"),
                                  tr("Error while the download : %1.")
                                  .arg(http->errorString()));
     }
     delete file;
     file = 0;
    
    bytesDeja += ctbytes;
    courantf++;
    if(courantf<vfichiers.size())
    {
		d_fa->setText(QString::number(courantf+1));
		telecharge(adresse + vfichiers.at(courantf));
    }
    else
    {
		QString remp = code;
		int reponse = QMessageBox::question(this, tr("Update"), tr("Download completed.<br />This update needs to restart COW. If you are actually chatting on COW, please close the window.<br /><br /><strong>Is COW closed?</strong>","Le téléchargement des différents fichiers est terminé.<br />Pour terminer la mise à jour, si vous chattez sur COW actuellement, vous devez fermer la fenêtre de COW. Sinon, cliquez maintenant sur OUI.<br /><br /><strong>COW est-il fermé ?</strong>"), QMessageBox::Yes | QMessageBox::No);
		if (reponse == QMessageBox::Yes)
		{
			system(ch(remp));
			#ifdef LINUX
				QProcess::startDetached(QDir::currentPath() + "/cow-client");
			#endif
			#ifdef WINOWS
				QProcess::startDetached(QDir::currentPath() + "/cow-client.exe");
			#endif
			#ifdef MAC
				QProcess::startDetached(QCoreApplication::applicationDirPath() + "/../../../COW.app/Contents/MacOS/./cow");
			#endif
		}
		else if (reponse == QMessageBox::No)
		{
			QMessageBox::critical(this, tr("Update"), tr("COW should be close to finish this installation!","Vous devez fermer COW pour terminer la mise à jour !"));
	    
			int reponsee = QMessageBox::question(this, tr("Update"), tr("If COW is actually open, please close it.<br /><br /><strong>Have you closed COW? (COW should be close to finish this installation!)</strong>","Si COW est ouvert actuellement, fermez-le.<br /><br /><strong>Avez-vous fermé COW ? (COW ne doit pas rester ouvert pour terminer la mise à jour !)</strong>"), QMessageBox::Yes | QMessageBox::No);
			if (reponsee == QMessageBox::Yes)
			{
				system(ch(remp));
			#ifdef LINUX
				QProcess::startDetached(QDir::currentPath() + "/COW");
			#endif
			#ifdef WINOWS
				QProcess::startDetached(QDir::currentPath() + "/COW.exe");
			#endif
			#ifdef MAC
				QProcess::startDetached(QCoreApplication::applicationDirPath() + "/../../../COW.app/Contents/MacOS/./cow");
			#endif
			}
			else if (reponsee == QMessageBox::No)
			{
				QMessageBox::critical(this, tr("Error"), tr("COW should be close to finish this installation!","COW ne doit pas être lancé pour terminer la mise à jour !"));
			}
		}
		this->close();
	}
     
 }
const char* Fenetre::ch(QString texte)
{
    return texte.toStdString().c_str();
}
QString Fenetre::versionv(int v) //En fonction du numéro de version, on retourne celui-ci, mais séparé par des points
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
				    
				    return QString::number(floor(v/1000)) + "." + QString::number(floor((v%1000)/100)) + "." + QString::number(floor((v%100)/10)) + "." + QString::number(v%10);
				}
			}
		}
	}
}
