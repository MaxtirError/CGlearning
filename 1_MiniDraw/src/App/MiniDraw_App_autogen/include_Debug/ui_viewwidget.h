/********************************************************************************
** Form generated from reading UI file 'viewwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_VIEWWIDGET_H
#define UI_VIEWWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ViewWidget
{
public:

    void setupUi(QWidget *ViewWidget)
    {
        if (ViewWidget->objectName().isEmpty())
            ViewWidget->setObjectName(QString::fromUtf8("ViewWidget"));
        ViewWidget->resize(400, 300);

        retranslateUi(ViewWidget);

        QMetaObject::connectSlotsByName(ViewWidget);
    } // setupUi

    void retranslateUi(QWidget *ViewWidget)
    {
        ViewWidget->setWindowTitle(QCoreApplication::translate("ViewWidget", "ViewWidget", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ViewWidget: public Ui_ViewWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_VIEWWIDGET_H
