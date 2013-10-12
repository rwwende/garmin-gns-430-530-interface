/********************************************************************************
** Form generated from reading UI file 'mainWindow.ui'
**
** Created: Sat Oct 12 21:47:27 2013
**      by: Qt User Interface Compiler version 4.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QFrame>
#include <QtGui/QHBoxLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "gnsViewWidget.h"

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QVBoxLayout *verticalLayout;
    QFrame *view_box;
    GnsViewWidget *gnsViewWidget;
    QTabWidget *tabWidget;
    QWidget *tabCOM;
    QVBoxLayout *verticalLayout_2;
    QHBoxLayout *horizontalLayout_2;
    QLabel *label;
    QSpacerItem *horizontalSpacer_2;
    QDoubleSpinBox *spinCOMActive;
    QPushButton *btnCOMActiveSet;
    QHBoxLayout *horizontalLayout_3;
    QLabel *label_2;
    QSpacerItem *horizontalSpacer_3;
    QDoubleSpinBox *spinCOMStandby;
    QPushButton *btnCOMStandbySet;
    QWidget *tabNAV;
    QVBoxLayout *verticalLayout_3;
    QHBoxLayout *horizontalLayout_4;
    QLabel *label_3;
    QSpacerItem *horizontalSpacer_4;
    QDoubleSpinBox *spinNAVActive;
    QPushButton *btnNAVActiveSet;
    QHBoxLayout *horizontalLayout_5;
    QLabel *label_4;
    QSpacerItem *horizontalSpacer_5;
    QDoubleSpinBox *spinNAVStandby;
    QPushButton *btnNAVStandbySet;
    QHBoxLayout *horizontalLayout;
    QComboBox *cmbModel;
    QPushButton *btnStart;
    QPushButton *btnStop;
    QSpacerItem *horizontalSpacer;

    void setupUi(QWidget *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
        MainWindow->resize(650, 549);
        verticalLayout = new QVBoxLayout(MainWindow);
        verticalLayout->setContentsMargins(3, 3, 3, 3);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        view_box = new QFrame(MainWindow);
        view_box->setObjectName(QString::fromUtf8("view_box"));
        QSizePolicy sizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(view_box->sizePolicy().hasHeightForWidth());
        view_box->setSizePolicy(sizePolicy);
        gnsViewWidget = new GnsViewWidget(view_box);
        gnsViewWidget->setObjectName(QString::fromUtf8("gnsViewWidget"));
        gnsViewWidget->setGeometry(QRect(120, 40, 351, 121));
        QSizePolicy sizePolicy1(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(gnsViewWidget->sizePolicy().hasHeightForWidth());
        gnsViewWidget->setSizePolicy(sizePolicy1);
        gnsViewWidget->setFocusPolicy(Qt::StrongFocus);

        verticalLayout->addWidget(view_box);

        tabWidget = new QTabWidget(MainWindow);
        tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
        tabWidget->setMinimumSize(QSize(0, 100));
        tabWidget->setMaximumSize(QSize(16777215, 100));
        tabCOM = new QWidget();
        tabCOM->setObjectName(QString::fromUtf8("tabCOM"));
        verticalLayout_2 = new QVBoxLayout(tabCOM);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        horizontalLayout_2 = new QHBoxLayout();
        horizontalLayout_2->setObjectName(QString::fromUtf8("horizontalLayout_2"));
        label = new QLabel(tabCOM);
        label->setObjectName(QString::fromUtf8("label"));

        horizontalLayout_2->addWidget(label);

        horizontalSpacer_2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_2->addItem(horizontalSpacer_2);

        spinCOMActive = new QDoubleSpinBox(tabCOM);
        spinCOMActive->setObjectName(QString::fromUtf8("spinCOMActive"));
        spinCOMActive->setDecimals(3);
        spinCOMActive->setMaximum(999);
        spinCOMActive->setSingleStep(0.5);
        spinCOMActive->setValue(125.25);

        horizontalLayout_2->addWidget(spinCOMActive);

        btnCOMActiveSet = new QPushButton(tabCOM);
        btnCOMActiveSet->setObjectName(QString::fromUtf8("btnCOMActiveSet"));

        horizontalLayout_2->addWidget(btnCOMActiveSet);


        verticalLayout_2->addLayout(horizontalLayout_2);

        horizontalLayout_3 = new QHBoxLayout();
        horizontalLayout_3->setObjectName(QString::fromUtf8("horizontalLayout_3"));
        label_2 = new QLabel(tabCOM);
        label_2->setObjectName(QString::fromUtf8("label_2"));

        horizontalLayout_3->addWidget(label_2);

        horizontalSpacer_3 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_3->addItem(horizontalSpacer_3);

        spinCOMStandby = new QDoubleSpinBox(tabCOM);
        spinCOMStandby->setObjectName(QString::fromUtf8("spinCOMStandby"));
        spinCOMStandby->setDecimals(3);
        spinCOMStandby->setMaximum(999);
        spinCOMStandby->setSingleStep(0.5);
        spinCOMStandby->setValue(125.25);

        horizontalLayout_3->addWidget(spinCOMStandby);

        btnCOMStandbySet = new QPushButton(tabCOM);
        btnCOMStandbySet->setObjectName(QString::fromUtf8("btnCOMStandbySet"));

        horizontalLayout_3->addWidget(btnCOMStandbySet);


        verticalLayout_2->addLayout(horizontalLayout_3);

        tabWidget->addTab(tabCOM, QString());
        tabNAV = new QWidget();
        tabNAV->setObjectName(QString::fromUtf8("tabNAV"));
        verticalLayout_3 = new QVBoxLayout(tabNAV);
        verticalLayout_3->setObjectName(QString::fromUtf8("verticalLayout_3"));
        horizontalLayout_4 = new QHBoxLayout();
        horizontalLayout_4->setObjectName(QString::fromUtf8("horizontalLayout_4"));
        label_3 = new QLabel(tabNAV);
        label_3->setObjectName(QString::fromUtf8("label_3"));

        horizontalLayout_4->addWidget(label_3);

        horizontalSpacer_4 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_4->addItem(horizontalSpacer_4);

        spinNAVActive = new QDoubleSpinBox(tabNAV);
        spinNAVActive->setObjectName(QString::fromUtf8("spinNAVActive"));
        spinNAVActive->setDecimals(3);
        spinNAVActive->setMaximum(999);
        spinNAVActive->setSingleStep(0.5);
        spinNAVActive->setValue(125.25);

        horizontalLayout_4->addWidget(spinNAVActive);

        btnNAVActiveSet = new QPushButton(tabNAV);
        btnNAVActiveSet->setObjectName(QString::fromUtf8("btnNAVActiveSet"));

        horizontalLayout_4->addWidget(btnNAVActiveSet);


        verticalLayout_3->addLayout(horizontalLayout_4);

        horizontalLayout_5 = new QHBoxLayout();
        horizontalLayout_5->setObjectName(QString::fromUtf8("horizontalLayout_5"));
        label_4 = new QLabel(tabNAV);
        label_4->setObjectName(QString::fromUtf8("label_4"));

        horizontalLayout_5->addWidget(label_4);

        horizontalSpacer_5 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout_5->addItem(horizontalSpacer_5);

        spinNAVStandby = new QDoubleSpinBox(tabNAV);
        spinNAVStandby->setObjectName(QString::fromUtf8("spinNAVStandby"));
        spinNAVStandby->setDecimals(3);
        spinNAVStandby->setMaximum(999);
        spinNAVStandby->setSingleStep(0.5);
        spinNAVStandby->setValue(125.25);

        horizontalLayout_5->addWidget(spinNAVStandby);

        btnNAVStandbySet = new QPushButton(tabNAV);
        btnNAVStandbySet->setObjectName(QString::fromUtf8("btnNAVStandbySet"));

        horizontalLayout_5->addWidget(btnNAVStandbySet);


        verticalLayout_3->addLayout(horizontalLayout_5);

        tabWidget->addTab(tabNAV, QString());

        verticalLayout->addWidget(tabWidget);

        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
        cmbModel = new QComboBox(MainWindow);
        cmbModel->setObjectName(QString::fromUtf8("cmbModel"));

        horizontalLayout->addWidget(cmbModel);

        btnStart = new QPushButton(MainWindow);
        btnStart->setObjectName(QString::fromUtf8("btnStart"));

        horizontalLayout->addWidget(btnStart);

        btnStop = new QPushButton(MainWindow);
        btnStop->setObjectName(QString::fromUtf8("btnStop"));

        horizontalLayout->addWidget(btnStop);

        horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        horizontalLayout->addItem(horizontalSpacer);


        verticalLayout->addLayout(horizontalLayout);


        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);
        cmbModel->setCurrentIndex(1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QWidget *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "Main window GNS", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("MainWindow", "Active", 0, QApplication::UnicodeUTF8));
        btnCOMActiveSet->setText(QApplication::translate("MainWindow", "Set", 0, QApplication::UnicodeUTF8));
        label_2->setText(QApplication::translate("MainWindow", "Standby", 0, QApplication::UnicodeUTF8));
        btnCOMStandbySet->setText(QApplication::translate("MainWindow", "Set", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabCOM), QApplication::translate("MainWindow", "COM", 0, QApplication::UnicodeUTF8));
        label_3->setText(QApplication::translate("MainWindow", "Active", 0, QApplication::UnicodeUTF8));
        btnNAVActiveSet->setText(QApplication::translate("MainWindow", "Set", 0, QApplication::UnicodeUTF8));
        label_4->setText(QApplication::translate("MainWindow", "Standby", 0, QApplication::UnicodeUTF8));
        btnNAVStandbySet->setText(QApplication::translate("MainWindow", "Set", 0, QApplication::UnicodeUTF8));
        tabWidget->setTabText(tabWidget->indexOf(tabNAV), QApplication::translate("MainWindow", "NAV", 0, QApplication::UnicodeUTF8));
        cmbModel->clear();
        cmbModel->insertItems(0, QStringList()
         << QApplication::translate("MainWindow", "GNS 430 WAAS", 0, QApplication::UnicodeUTF8)
         << QApplication::translate("MainWindow", "GNS 530 WAAS", 0, QApplication::UnicodeUTF8)
        );
        btnStart->setText(QApplication::translate("MainWindow", "Start", 0, QApplication::UnicodeUTF8));
        btnStop->setText(QApplication::translate("MainWindow", "Stop", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
