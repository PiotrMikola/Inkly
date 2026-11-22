/********************************************************************************
** Form generated from reading UI file 'Inkly.ui'
**
** Created by: Qt User Interface Compiler version 6.12.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_INKLY_H
#define UI_INKLY_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_InklyClass
{
public:
    QMenuBar *menuBar;
    QToolBar *mainToolBar;
    QWidget *centralWidget;
    QStatusBar *statusBar;

    void setupUi(QMainWindow *InklyClass)
    {
        if (InklyClass->objectName().isEmpty())
            InklyClass->setObjectName("InklyClass");
        InklyClass->resize(600, 400);
        menuBar = new QMenuBar(InklyClass);
        menuBar->setObjectName("menuBar");
        InklyClass->setMenuBar(menuBar);
        mainToolBar = new QToolBar(InklyClass);
        mainToolBar->setObjectName("mainToolBar");
        InklyClass->addToolBar(mainToolBar);
        centralWidget = new QWidget(InklyClass);
        centralWidget->setObjectName("centralWidget");
        InklyClass->setCentralWidget(centralWidget);
        statusBar = new QStatusBar(InklyClass);
        statusBar->setObjectName("statusBar");
        InklyClass->setStatusBar(statusBar);

        retranslateUi(InklyClass);

        QMetaObject::connectSlotsByName(InklyClass);
    } // setupUi

    void retranslateUi(QMainWindow *InklyClass)
    {
        InklyClass->setWindowTitle(QCoreApplication::translate("InklyClass", "Inkly", nullptr));
    } // retranslateUi

};

namespace Ui {
    class InklyClass: public Ui_InklyClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_INKLY_H
