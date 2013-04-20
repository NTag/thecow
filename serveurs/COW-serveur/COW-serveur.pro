# -------------------------------------------------
# Project created by QtCreator 2008-11-19T14:11:36
# -------------------------------------------------
QT += network
QT -= gui
QT += sql
TARGET = COW-serveur
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp \
    fonctions.cpp \
    client.cpp
HEADERS += fonctions.h \
    client.h
TRANSLATIONS = cow-serveur_en.ts
