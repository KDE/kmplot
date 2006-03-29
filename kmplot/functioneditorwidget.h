#include <kdialog.h>
#include <klocale.h>
#ifndef FUNCTIONEDITORWIDGET_H
#define FUNCTIONEDITORWIDGET_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDoubleSpinBox>
#include <QtGui/QGridLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QListWidget>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QStackedWidget>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QWidget>
#include "kcolorbutton.h"
#include "klineedit.h"
#include "knuminput.h"
#include "kpushbutton.h"

class Ui_FunctionEditorWidget
{
public:
    QVBoxLayout *vboxLayout;
    QHBoxLayout *hboxLayout;
    QListWidget *functionList;
    QVBoxLayout *vboxLayout1;
    QPushButton *createCartesian;
    QPushButton *createParametric;
    QPushButton *createPolar;
    QSpacerItem *spacerItem;
    QPushButton *deleteButton;
    QStackedWidget *stackedWidget;
    QWidget *page;
    QVBoxLayout *vboxLayout2;
    QTabWidget *tabWidget;
    QWidget *tab;
    QVBoxLayout *vboxLayout3;
    QHBoxLayout *hboxLayout1;
    QLabel *textLabel1_4;
    KLineEdit *cartesianEquation;
    QCheckBox *cartesianHideExtensions;
    QGroupBox *cartesianParameters;
    QGridLayout *gridLayout;
    QRadioButton *cartesianDisableParameters;
    QRadioButton *cartesianParametersList;
    QComboBox *listOfSliders;
    QRadioButton *cartesianParameterSlider;
    KPushButton *editParameterListButton;
    QGroupBox *groupBox;
    QGridLayout *gridLayout1;
    KLineEdit *cartesianMax;
    QCheckBox *cartesianCustomMax;
    QCheckBox *cartesianCustomMin;
    KLineEdit *cartesianMin;
    QGroupBox *groupBox_2;
    QGridLayout *gridLayout2;
    QDoubleSpinBox *cartesian_f_lineWidth;
    QLabel *textLabel1_3;
    KColorButton *cartesian_f_lineColor;
    QLabel *textLabel2;
    QWidget *tab_3;
    QVBoxLayout *vboxLayout4;
    QCheckBox *showDerivative1;
    QGroupBox *groupBox_5;
    QGridLayout *gridLayout3;
    QDoubleSpinBox *cartesian_f1_lineWidth;
    QLabel *textLabel1_3_2_2;
    QLabel *textLabel2_2_2;
    KColorButton *cartesian_f1_lineColor;
    QCheckBox *showDerivative2;
    QGroupBox *groupBox_4;
    QGridLayout *gridLayout4;
    QDoubleSpinBox *cartesian_f2_lineWidth;
    KColorButton *cartesian_f2_lineColor;
    QLabel *textLabel2_2;
    QLabel *textLabel1_3_2;
    QSpacerItem *spacerItem1;
    QWidget *tab_2;
    QVBoxLayout *vboxLayout5;
    QCheckBox *showIntegral;
    QGroupBox *groupBox_6;
    QGridLayout *gridLayout5;
    QLabel *textLabel1_5;
    KLineEdit *txtInitX;
    KLineEdit *txtInitY;
    QLabel *textLabel1_5_2;
    QGroupBox *groupBox_8;
    QHBoxLayout *hboxLayout2;
    QCheckBox *customPrecision;
    KDoubleNumInput *precision;
    QGroupBox *groupBox_7;
    QGridLayout *gridLayout6;
    QDoubleSpinBox *cartesian_F_lineWidth;
    QLabel *textLabel2_3;
    KColorButton *cartesian_F_lineColor;
    QLabel *textLabel1_3_3;
    QSpacerItem *spacerItem2;
    QWidget *page_3;
    QVBoxLayout *vboxLayout6;
    QGroupBox *groupBox_11;
    QGridLayout *gridLayout7;
    QLabel *textLabel4;
    KLineEdit *parametricX;
    KLineEdit *parametricY;
    QCheckBox *parametricHideExtensions;
    KLineEdit *parametricName;
    QHBoxLayout *hboxLayout3;
    QLabel *textLabelY;
    QLabel *textLabelYF;
    QLabel *textLabelArgY;
    QHBoxLayout *hboxLayout4;
    QLabel *textLabelX;
    QLabel *textLabelXF;
    QLabel *textLabelArgX;
    QGroupBox *groupBox_9;
    QGridLayout *gridLayout8;
    KLineEdit *parametricMin;
    KLineEdit *parametricMax;
    QCheckBox *parametricCustomMin;
    QCheckBox *parametricCustomMax;
    QGroupBox *groupBox_10;
    QGridLayout *gridLayout9;
    QDoubleSpinBox *parametricLineWidth;
    QLabel *textLabel2_4;
    KColorButton *parametricLineColor;
    QLabel *textLabel1_8;
    QSpacerItem *spacerItem3;
    QWidget *page_4;
    QVBoxLayout *vboxLayout7;
    QHBoxLayout *hboxLayout5;
    QLabel *textLabel2_6;
    KLineEdit *polarEquation;
    QCheckBox *polarHideExtensions;
    QGroupBox *groupBox_13;
    QGridLayout *gridLayout10;
    KLineEdit *polarMax;
    QCheckBox *polarCustomMin;
    QCheckBox *polarCustomMax;
    KLineEdit *polarMin;
    QGroupBox *groupBox_12;
    QGridLayout *gridLayout11;
    QDoubleSpinBox *polarLineWidth;
    QLabel *textLabel1_9;
    KColorButton *polarLineColor;
    QLabel *textLabel2_5;
    QSpacerItem *spacerItem4;
    QWidget *page_2;
    QSpacerItem *spacerItem5;

    void setupUi(QWidget *FunctionEditorWidget)
    {
    FunctionEditorWidget->setObjectName(QString::fromUtf8("FunctionEditorWidget"));
    FunctionEditorWidget->resize(QSize(312, 754).expandedTo(FunctionEditorWidget->minimumSizeHint()));
    vboxLayout = new QVBoxLayout(FunctionEditorWidget);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(9);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    functionList = new QListWidget(FunctionEditorWidget);
    functionList->setObjectName(QString::fromUtf8("functionList"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(7));
    sizePolicy.setHorizontalStretch(255);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(functionList->sizePolicy().hasHeightForWidth());
    functionList->setSizePolicy(sizePolicy);

    hboxLayout->addWidget(functionList);

    vboxLayout1 = new QVBoxLayout();
    vboxLayout1->setSpacing(6);
    vboxLayout1->setMargin(0);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    createCartesian = new QPushButton(FunctionEditorWidget);
    createCartesian->setObjectName(QString::fromUtf8("createCartesian"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(1);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(createCartesian->sizePolicy().hasHeightForWidth());
    createCartesian->setSizePolicy(sizePolicy1);
    createCartesian->setDefault(true);

    vboxLayout1->addWidget(createCartesian);

    createParametric = new QPushButton(FunctionEditorWidget);
    createParametric->setObjectName(QString::fromUtf8("createParametric"));
    QSizePolicy sizePolicy2(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(0));
    sizePolicy2.setHorizontalStretch(1);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(createParametric->sizePolicy().hasHeightForWidth());
    createParametric->setSizePolicy(sizePolicy2);

    vboxLayout1->addWidget(createParametric);

    createPolar = new QPushButton(FunctionEditorWidget);
    createPolar->setObjectName(QString::fromUtf8("createPolar"));
    QSizePolicy sizePolicy3(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(0));
    sizePolicy3.setHorizontalStretch(1);
    sizePolicy3.setVerticalStretch(0);
    sizePolicy3.setHeightForWidth(createPolar->sizePolicy().hasHeightForWidth());
    createPolar->setSizePolicy(sizePolicy3);

    vboxLayout1->addWidget(createPolar);

    spacerItem = new QSpacerItem(20, 16, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout1->addItem(spacerItem);

    deleteButton = new QPushButton(FunctionEditorWidget);
    deleteButton->setObjectName(QString::fromUtf8("deleteButton"));
    deleteButton->setEnabled(false);
    QSizePolicy sizePolicy4(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(0));
    sizePolicy4.setHorizontalStretch(1);
    sizePolicy4.setVerticalStretch(0);
    sizePolicy4.setHeightForWidth(deleteButton->sizePolicy().hasHeightForWidth());
    deleteButton->setSizePolicy(sizePolicy4);

    vboxLayout1->addWidget(deleteButton);


    hboxLayout->addLayout(vboxLayout1);


    vboxLayout->addLayout(hboxLayout);

    stackedWidget = new QStackedWidget(FunctionEditorWidget);
    stackedWidget->setObjectName(QString::fromUtf8("stackedWidget"));
    page = new QWidget();
    page->setObjectName(QString::fromUtf8("page"));
    vboxLayout2 = new QVBoxLayout(page);
    vboxLayout2->setSpacing(6);
    vboxLayout2->setMargin(9);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    tabWidget = new QTabWidget(page);
    tabWidget->setObjectName(QString::fromUtf8("tabWidget"));
    tab = new QWidget();
    tab->setObjectName(QString::fromUtf8("tab"));
    vboxLayout3 = new QVBoxLayout(tab);
    vboxLayout3->setSpacing(6);
    vboxLayout3->setMargin(9);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    textLabel1_4 = new QLabel(tab);
    textLabel1_4->setObjectName(QString::fromUtf8("textLabel1_4"));

    hboxLayout1->addWidget(textLabel1_4);

    cartesianEquation = new KLineEdit(tab);
    cartesianEquation->setObjectName(QString::fromUtf8("cartesianEquation"));
    QSizePolicy sizePolicy5(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy5.setHorizontalStretch(0);
    sizePolicy5.setVerticalStretch(0);
    sizePolicy5.setHeightForWidth(cartesianEquation->sizePolicy().hasHeightForWidth());
    cartesianEquation->setSizePolicy(sizePolicy5);

    hboxLayout1->addWidget(cartesianEquation);


    vboxLayout3->addLayout(hboxLayout1);

    cartesianHideExtensions = new QCheckBox(tab);
    cartesianHideExtensions->setObjectName(QString::fromUtf8("cartesianHideExtensions"));

    vboxLayout3->addWidget(cartesianHideExtensions);

    cartesianParameters = new QGroupBox(tab);
    cartesianParameters->setObjectName(QString::fromUtf8("cartesianParameters"));
    gridLayout = new QGridLayout(cartesianParameters);
    gridLayout->setSpacing(6);
    gridLayout->setMargin(9);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    cartesianDisableParameters = new QRadioButton(cartesianParameters);
    cartesianDisableParameters->setObjectName(QString::fromUtf8("cartesianDisableParameters"));

    gridLayout->addWidget(cartesianDisableParameters, 0, 0, 2, 2);

    cartesianParametersList = new QRadioButton(cartesianParameters);
    cartesianParametersList->setObjectName(QString::fromUtf8("cartesianParametersList"));

    gridLayout->addWidget(cartesianParametersList, 2, 0, 1, 1);

    listOfSliders = new QComboBox(cartesianParameters);
    listOfSliders->setObjectName(QString::fromUtf8("listOfSliders"));
    listOfSliders->setEnabled(false);

    gridLayout->addWidget(listOfSliders, 3, 1, 1, 1);

    cartesianParameterSlider = new QRadioButton(cartesianParameters);
    cartesianParameterSlider->setObjectName(QString::fromUtf8("cartesianParameterSlider"));

    gridLayout->addWidget(cartesianParameterSlider, 3, 0, 1, 1);

    editParameterListButton = new KPushButton(cartesianParameters);
    editParameterListButton->setObjectName(QString::fromUtf8("editParameterListButton"));
    editParameterListButton->setEnabled(false);

    gridLayout->addWidget(editParameterListButton, 2, 1, 1, 1);


    vboxLayout3->addWidget(cartesianParameters);

    groupBox = new QGroupBox(tab);
    groupBox->setObjectName(QString::fromUtf8("groupBox"));
    gridLayout1 = new QGridLayout(groupBox);
    gridLayout1->setSpacing(6);
    gridLayout1->setMargin(9);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    cartesianMax = new KLineEdit(groupBox);
    cartesianMax->setObjectName(QString::fromUtf8("cartesianMax"));
    cartesianMax->setEnabled(false);

    gridLayout1->addWidget(cartesianMax, 1, 1, 1, 1);

    cartesianCustomMax = new QCheckBox(groupBox);
    cartesianCustomMax->setObjectName(QString::fromUtf8("cartesianCustomMax"));

    gridLayout1->addWidget(cartesianCustomMax, 1, 0, 1, 1);

    cartesianCustomMin = new QCheckBox(groupBox);
    cartesianCustomMin->setObjectName(QString::fromUtf8("cartesianCustomMin"));

    gridLayout1->addWidget(cartesianCustomMin, 0, 0, 1, 1);

    cartesianMin = new KLineEdit(groupBox);
    cartesianMin->setObjectName(QString::fromUtf8("cartesianMin"));
    cartesianMin->setEnabled(false);

    gridLayout1->addWidget(cartesianMin, 0, 1, 1, 1);


    vboxLayout3->addWidget(groupBox);

    groupBox_2 = new QGroupBox(tab);
    groupBox_2->setObjectName(QString::fromUtf8("groupBox_2"));
    gridLayout2 = new QGridLayout(groupBox_2);
    gridLayout2->setSpacing(6);
    gridLayout2->setMargin(9);
    gridLayout2->setObjectName(QString::fromUtf8("gridLayout2"));
    cartesian_f_lineWidth = new QDoubleSpinBox(groupBox_2);
    cartesian_f_lineWidth->setObjectName(QString::fromUtf8("cartesian_f_lineWidth"));
    cartesian_f_lineWidth->setAlignment(Qt::AlignRight);
    cartesian_f_lineWidth->setDecimals(1);
    cartesian_f_lineWidth->setMinimum(0.1);
    cartesian_f_lineWidth->setSingleStep(0.1);

    gridLayout2->addWidget(cartesian_f_lineWidth, 0, 1, 1, 1);

    textLabel1_3 = new QLabel(groupBox_2);
    textLabel1_3->setObjectName(QString::fromUtf8("textLabel1_3"));

    gridLayout2->addWidget(textLabel1_3, 0, 0, 1, 1);

    cartesian_f_lineColor = new KColorButton(groupBox_2);
    cartesian_f_lineColor->setObjectName(QString::fromUtf8("cartesian_f_lineColor"));

    gridLayout2->addWidget(cartesian_f_lineColor, 1, 1, 1, 1);

    textLabel2 = new QLabel(groupBox_2);
    textLabel2->setObjectName(QString::fromUtf8("textLabel2"));

    gridLayout2->addWidget(textLabel2, 1, 0, 1, 1);


    vboxLayout3->addWidget(groupBox_2);

    tabWidget->addTab(tab, tr2i18n("Function", 0));
    tab_3 = new QWidget();
    tab_3->setObjectName(QString::fromUtf8("tab_3"));
    vboxLayout4 = new QVBoxLayout(tab_3);
    vboxLayout4->setSpacing(6);
    vboxLayout4->setMargin(9);
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    showDerivative1 = new QCheckBox(tab_3);
    showDerivative1->setObjectName(QString::fromUtf8("showDerivative1"));

    vboxLayout4->addWidget(showDerivative1);

    groupBox_5 = new QGroupBox(tab_3);
    groupBox_5->setObjectName(QString::fromUtf8("groupBox_5"));
    groupBox_5->setEnabled(false);
    gridLayout3 = new QGridLayout(groupBox_5);
    gridLayout3->setSpacing(6);
    gridLayout3->setMargin(9);
    gridLayout3->setObjectName(QString::fromUtf8("gridLayout3"));
    cartesian_f1_lineWidth = new QDoubleSpinBox(groupBox_5);
    cartesian_f1_lineWidth->setObjectName(QString::fromUtf8("cartesian_f1_lineWidth"));
    cartesian_f1_lineWidth->setAlignment(Qt::AlignRight);
    cartesian_f1_lineWidth->setDecimals(1);
    cartesian_f1_lineWidth->setMinimum(0.1);
    cartesian_f1_lineWidth->setSingleStep(0.1);

    gridLayout3->addWidget(cartesian_f1_lineWidth, 0, 1, 1, 1);

    textLabel1_3_2_2 = new QLabel(groupBox_5);
    textLabel1_3_2_2->setObjectName(QString::fromUtf8("textLabel1_3_2_2"));

    gridLayout3->addWidget(textLabel1_3_2_2, 0, 0, 1, 1);

    textLabel2_2_2 = new QLabel(groupBox_5);
    textLabel2_2_2->setObjectName(QString::fromUtf8("textLabel2_2_2"));

    gridLayout3->addWidget(textLabel2_2_2, 1, 0, 1, 1);

    cartesian_f1_lineColor = new KColorButton(groupBox_5);
    cartesian_f1_lineColor->setObjectName(QString::fromUtf8("cartesian_f1_lineColor"));

    gridLayout3->addWidget(cartesian_f1_lineColor, 1, 1, 1, 1);


    vboxLayout4->addWidget(groupBox_5);

    showDerivative2 = new QCheckBox(tab_3);
    showDerivative2->setObjectName(QString::fromUtf8("showDerivative2"));

    vboxLayout4->addWidget(showDerivative2);

    groupBox_4 = new QGroupBox(tab_3);
    groupBox_4->setObjectName(QString::fromUtf8("groupBox_4"));
    groupBox_4->setEnabled(false);
    gridLayout4 = new QGridLayout(groupBox_4);
    gridLayout4->setSpacing(6);
    gridLayout4->setMargin(9);
    gridLayout4->setObjectName(QString::fromUtf8("gridLayout4"));
    cartesian_f2_lineWidth = new QDoubleSpinBox(groupBox_4);
    cartesian_f2_lineWidth->setObjectName(QString::fromUtf8("cartesian_f2_lineWidth"));
    cartesian_f2_lineWidth->setAlignment(Qt::AlignRight);
    cartesian_f2_lineWidth->setDecimals(1);
    cartesian_f2_lineWidth->setMinimum(0.1);
    cartesian_f2_lineWidth->setSingleStep(0.1);

    gridLayout4->addWidget(cartesian_f2_lineWidth, 0, 1, 1, 1);

    cartesian_f2_lineColor = new KColorButton(groupBox_4);
    cartesian_f2_lineColor->setObjectName(QString::fromUtf8("cartesian_f2_lineColor"));

    gridLayout4->addWidget(cartesian_f2_lineColor, 1, 1, 1, 1);

    textLabel2_2 = new QLabel(groupBox_4);
    textLabel2_2->setObjectName(QString::fromUtf8("textLabel2_2"));

    gridLayout4->addWidget(textLabel2_2, 1, 0, 1, 1);

    textLabel1_3_2 = new QLabel(groupBox_4);
    textLabel1_3_2->setObjectName(QString::fromUtf8("textLabel1_3_2"));

    gridLayout4->addWidget(textLabel1_3_2, 0, 0, 1, 1);


    vboxLayout4->addWidget(groupBox_4);

    spacerItem1 = new QSpacerItem(10, 106, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout4->addItem(spacerItem1);

    tabWidget->addTab(tab_3, tr2i18n("Derivatives", 0));
    tab_2 = new QWidget();
    tab_2->setObjectName(QString::fromUtf8("tab_2"));
    vboxLayout5 = new QVBoxLayout(tab_2);
    vboxLayout5->setSpacing(6);
    vboxLayout5->setMargin(9);
    vboxLayout5->setObjectName(QString::fromUtf8("vboxLayout5"));
    showIntegral = new QCheckBox(tab_2);
    showIntegral->setObjectName(QString::fromUtf8("showIntegral"));

    vboxLayout5->addWidget(showIntegral);

    groupBox_6 = new QGroupBox(tab_2);
    groupBox_6->setObjectName(QString::fromUtf8("groupBox_6"));
    groupBox_6->setEnabled(false);
    gridLayout5 = new QGridLayout(groupBox_6);
    gridLayout5->setSpacing(6);
    gridLayout5->setMargin(9);
    gridLayout5->setObjectName(QString::fromUtf8("gridLayout5"));
    textLabel1_5 = new QLabel(groupBox_6);
    textLabel1_5->setObjectName(QString::fromUtf8("textLabel1_5"));

    gridLayout5->addWidget(textLabel1_5, 0, 0, 1, 1);

    txtInitX = new KLineEdit(groupBox_6);
    txtInitX->setObjectName(QString::fromUtf8("txtInitX"));

    gridLayout5->addWidget(txtInitX, 0, 1, 1, 1);

    txtInitY = new KLineEdit(groupBox_6);
    txtInitY->setObjectName(QString::fromUtf8("txtInitY"));

    gridLayout5->addWidget(txtInitY, 1, 1, 1, 1);

    textLabel1_5_2 = new QLabel(groupBox_6);
    textLabel1_5_2->setObjectName(QString::fromUtf8("textLabel1_5_2"));

    gridLayout5->addWidget(textLabel1_5_2, 1, 0, 1, 1);


    vboxLayout5->addWidget(groupBox_6);

    groupBox_8 = new QGroupBox(tab_2);
    groupBox_8->setObjectName(QString::fromUtf8("groupBox_8"));
    groupBox_8->setEnabled(false);
    hboxLayout2 = new QHBoxLayout(groupBox_8);
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(9);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    customPrecision = new QCheckBox(groupBox_8);
    customPrecision->setObjectName(QString::fromUtf8("customPrecision"));

    hboxLayout2->addWidget(customPrecision);

    precision = new KDoubleNumInput(groupBox_8);
    precision->setObjectName(QString::fromUtf8("precision"));
    precision->setMinimum(0.01);

    hboxLayout2->addWidget(precision);


    vboxLayout5->addWidget(groupBox_8);

    groupBox_7 = new QGroupBox(tab_2);
    groupBox_7->setObjectName(QString::fromUtf8("groupBox_7"));
    groupBox_7->setEnabled(false);
    gridLayout6 = new QGridLayout(groupBox_7);
    gridLayout6->setSpacing(6);
    gridLayout6->setMargin(9);
    gridLayout6->setObjectName(QString::fromUtf8("gridLayout6"));
    cartesian_F_lineWidth = new QDoubleSpinBox(groupBox_7);
    cartesian_F_lineWidth->setObjectName(QString::fromUtf8("cartesian_F_lineWidth"));
    cartesian_F_lineWidth->setAlignment(Qt::AlignRight);
    cartesian_F_lineWidth->setDecimals(1);
    cartesian_F_lineWidth->setMinimum(0.1);
    cartesian_F_lineWidth->setSingleStep(0.1);

    gridLayout6->addWidget(cartesian_F_lineWidth, 0, 1, 1, 1);

    textLabel2_3 = new QLabel(groupBox_7);
    textLabel2_3->setObjectName(QString::fromUtf8("textLabel2_3"));

    gridLayout6->addWidget(textLabel2_3, 1, 0, 1, 1);

    cartesian_F_lineColor = new KColorButton(groupBox_7);
    cartesian_F_lineColor->setObjectName(QString::fromUtf8("cartesian_F_lineColor"));

    gridLayout6->addWidget(cartesian_F_lineColor, 1, 1, 1, 1);

    textLabel1_3_3 = new QLabel(groupBox_7);
    textLabel1_3_3->setObjectName(QString::fromUtf8("textLabel1_3_3"));

    gridLayout6->addWidget(textLabel1_3_3, 0, 0, 1, 1);


    vboxLayout5->addWidget(groupBox_7);

    spacerItem2 = new QSpacerItem(10, 79, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout5->addItem(spacerItem2);

    tabWidget->addTab(tab_2, tr2i18n("Integral", 0));

    vboxLayout2->addWidget(tabWidget);

    stackedWidget->addWidget(page);
    page_3 = new QWidget();
    page_3->setObjectName(QString::fromUtf8("page_3"));
    vboxLayout6 = new QVBoxLayout(page_3);
    vboxLayout6->setSpacing(6);
    vboxLayout6->setMargin(9);
    vboxLayout6->setObjectName(QString::fromUtf8("vboxLayout6"));
    groupBox_11 = new QGroupBox(page_3);
    groupBox_11->setObjectName(QString::fromUtf8("groupBox_11"));
    gridLayout7 = new QGridLayout(groupBox_11);
    gridLayout7->setSpacing(6);
    gridLayout7->setMargin(9);
    gridLayout7->setObjectName(QString::fromUtf8("gridLayout7"));
    textLabel4 = new QLabel(groupBox_11);
    textLabel4->setObjectName(QString::fromUtf8("textLabel4"));
    textLabel4->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

    gridLayout7->addWidget(textLabel4, 0, 0, 1, 1);

    parametricX = new KLineEdit(groupBox_11);
    parametricX->setObjectName(QString::fromUtf8("parametricX"));
    QSizePolicy sizePolicy6(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy6.setHorizontalStretch(0);
    sizePolicy6.setVerticalStretch(0);
    sizePolicy6.setHeightForWidth(parametricX->sizePolicy().hasHeightForWidth());
    parametricX->setSizePolicy(sizePolicy6);

    gridLayout7->addWidget(parametricX, 1, 1, 1, 1);

    parametricY = new KLineEdit(groupBox_11);
    parametricY->setObjectName(QString::fromUtf8("parametricY"));
    QSizePolicy sizePolicy7(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy7.setHorizontalStretch(0);
    sizePolicy7.setVerticalStretch(0);
    sizePolicy7.setHeightForWidth(parametricY->sizePolicy().hasHeightForWidth());
    parametricY->setSizePolicy(sizePolicy7);

    gridLayout7->addWidget(parametricY, 2, 1, 1, 1);

    parametricHideExtensions = new QCheckBox(groupBox_11);
    parametricHideExtensions->setObjectName(QString::fromUtf8("parametricHideExtensions"));

    gridLayout7->addWidget(parametricHideExtensions, 3, 0, 1, 2);

    parametricName = new KLineEdit(groupBox_11);
    parametricName->setObjectName(QString::fromUtf8("parametricName"));

    gridLayout7->addWidget(parametricName, 0, 1, 1, 1);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(0);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    textLabelY = new QLabel(groupBox_11);
    textLabelY->setObjectName(QString::fromUtf8("textLabelY"));

    hboxLayout3->addWidget(textLabelY);

    textLabelYF = new QLabel(groupBox_11);
    textLabelYF->setObjectName(QString::fromUtf8("textLabelYF"));
    QFont font;
    font.setFamily(QString::fromUtf8("Sans Serif"));
    font.setPointSize(9);
    font.setBold(true);
    font.setItalic(false);
    font.setUnderline(false);
    font.setWeight(75);
    font.setStrikeOut(false);
    textLabelYF->setFont(font);

    hboxLayout3->addWidget(textLabelYF);

    textLabelArgY = new QLabel(groupBox_11);
    textLabelArgY->setObjectName(QString::fromUtf8("textLabelArgY"));

    hboxLayout3->addWidget(textLabelArgY);


    gridLayout7->addLayout(hboxLayout3, 2, 0, 1, 1);

    hboxLayout4 = new QHBoxLayout();
    hboxLayout4->setSpacing(0);
    hboxLayout4->setMargin(0);
    hboxLayout4->setObjectName(QString::fromUtf8("hboxLayout4"));
    textLabelX = new QLabel(groupBox_11);
    textLabelX->setObjectName(QString::fromUtf8("textLabelX"));

    hboxLayout4->addWidget(textLabelX);

    textLabelXF = new QLabel(groupBox_11);
    textLabelXF->setObjectName(QString::fromUtf8("textLabelXF"));
    QFont font1;
    font1.setFamily(QString::fromUtf8("Sans Serif"));
    font1.setPointSize(9);
    font1.setBold(true);
    font1.setItalic(false);
    font1.setUnderline(false);
    font1.setWeight(75);
    font1.setStrikeOut(false);
    textLabelXF->setFont(font1);

    hboxLayout4->addWidget(textLabelXF);

    textLabelArgX = new QLabel(groupBox_11);
    textLabelArgX->setObjectName(QString::fromUtf8("textLabelArgX"));

    hboxLayout4->addWidget(textLabelArgX);


    gridLayout7->addLayout(hboxLayout4, 1, 0, 1, 1);


    vboxLayout6->addWidget(groupBox_11);

    groupBox_9 = new QGroupBox(page_3);
    groupBox_9->setObjectName(QString::fromUtf8("groupBox_9"));
    gridLayout8 = new QGridLayout(groupBox_9);
    gridLayout8->setSpacing(6);
    gridLayout8->setMargin(9);
    gridLayout8->setObjectName(QString::fromUtf8("gridLayout8"));
    parametricMin = new KLineEdit(groupBox_9);
    parametricMin->setObjectName(QString::fromUtf8("parametricMin"));
    parametricMin->setEnabled(false);

    gridLayout8->addWidget(parametricMin, 0, 1, 1, 1);

    parametricMax = new KLineEdit(groupBox_9);
    parametricMax->setObjectName(QString::fromUtf8("parametricMax"));
    parametricMax->setEnabled(false);

    gridLayout8->addWidget(parametricMax, 1, 1, 1, 1);

    parametricCustomMin = new QCheckBox(groupBox_9);
    parametricCustomMin->setObjectName(QString::fromUtf8("parametricCustomMin"));

    gridLayout8->addWidget(parametricCustomMin, 0, 0, 1, 1);

    parametricCustomMax = new QCheckBox(groupBox_9);
    parametricCustomMax->setObjectName(QString::fromUtf8("parametricCustomMax"));

    gridLayout8->addWidget(parametricCustomMax, 1, 0, 1, 1);


    vboxLayout6->addWidget(groupBox_9);

    groupBox_10 = new QGroupBox(page_3);
    groupBox_10->setObjectName(QString::fromUtf8("groupBox_10"));
    gridLayout9 = new QGridLayout(groupBox_10);
    gridLayout9->setSpacing(6);
    gridLayout9->setMargin(9);
    gridLayout9->setObjectName(QString::fromUtf8("gridLayout9"));
    parametricLineWidth = new QDoubleSpinBox(groupBox_10);
    parametricLineWidth->setObjectName(QString::fromUtf8("parametricLineWidth"));
    parametricLineWidth->setAlignment(Qt::AlignRight);
    parametricLineWidth->setDecimals(1);
    parametricLineWidth->setMinimum(0.1);
    parametricLineWidth->setSingleStep(0.1);

    gridLayout9->addWidget(parametricLineWidth, 0, 1, 1, 1);

    textLabel2_4 = new QLabel(groupBox_10);
    textLabel2_4->setObjectName(QString::fromUtf8("textLabel2_4"));

    gridLayout9->addWidget(textLabel2_4, 1, 0, 1, 1);

    parametricLineColor = new KColorButton(groupBox_10);
    parametricLineColor->setObjectName(QString::fromUtf8("parametricLineColor"));

    gridLayout9->addWidget(parametricLineColor, 1, 1, 1, 1);

    textLabel1_8 = new QLabel(groupBox_10);
    textLabel1_8->setObjectName(QString::fromUtf8("textLabel1_8"));

    gridLayout9->addWidget(textLabel1_8, 0, 0, 1, 1);


    vboxLayout6->addWidget(groupBox_10);

    spacerItem3 = new QSpacerItem(10, 143, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout6->addItem(spacerItem3);

    stackedWidget->addWidget(page_3);
    page_4 = new QWidget();
    page_4->setObjectName(QString::fromUtf8("page_4"));
    vboxLayout7 = new QVBoxLayout(page_4);
    vboxLayout7->setSpacing(6);
    vboxLayout7->setMargin(9);
    vboxLayout7->setObjectName(QString::fromUtf8("vboxLayout7"));
    hboxLayout5 = new QHBoxLayout();
    hboxLayout5->setSpacing(6);
    hboxLayout5->setMargin(0);
    hboxLayout5->setObjectName(QString::fromUtf8("hboxLayout5"));
    textLabel2_6 = new QLabel(page_4);
    textLabel2_6->setObjectName(QString::fromUtf8("textLabel2_6"));

    hboxLayout5->addWidget(textLabel2_6);

    polarEquation = new KLineEdit(page_4);
    polarEquation->setObjectName(QString::fromUtf8("polarEquation"));
    QSizePolicy sizePolicy8(static_cast<QSizePolicy::Policy>(7), static_cast<QSizePolicy::Policy>(0));
    sizePolicy8.setHorizontalStretch(0);
    sizePolicy8.setVerticalStretch(0);
    sizePolicy8.setHeightForWidth(polarEquation->sizePolicy().hasHeightForWidth());
    polarEquation->setSizePolicy(sizePolicy8);

    hboxLayout5->addWidget(polarEquation);


    vboxLayout7->addLayout(hboxLayout5);

    polarHideExtensions = new QCheckBox(page_4);
    polarHideExtensions->setObjectName(QString::fromUtf8("polarHideExtensions"));

    vboxLayout7->addWidget(polarHideExtensions);

    groupBox_13 = new QGroupBox(page_4);
    groupBox_13->setObjectName(QString::fromUtf8("groupBox_13"));
    gridLayout10 = new QGridLayout(groupBox_13);
    gridLayout10->setSpacing(6);
    gridLayout10->setMargin(9);
    gridLayout10->setObjectName(QString::fromUtf8("gridLayout10"));
    polarMax = new KLineEdit(groupBox_13);
    polarMax->setObjectName(QString::fromUtf8("polarMax"));
    polarMax->setEnabled(false);

    gridLayout10->addWidget(polarMax, 1, 1, 1, 1);

    polarCustomMin = new QCheckBox(groupBox_13);
    polarCustomMin->setObjectName(QString::fromUtf8("polarCustomMin"));

    gridLayout10->addWidget(polarCustomMin, 0, 0, 1, 1);

    polarCustomMax = new QCheckBox(groupBox_13);
    polarCustomMax->setObjectName(QString::fromUtf8("polarCustomMax"));

    gridLayout10->addWidget(polarCustomMax, 1, 0, 1, 1);

    polarMin = new KLineEdit(groupBox_13);
    polarMin->setObjectName(QString::fromUtf8("polarMin"));
    polarMin->setEnabled(false);

    gridLayout10->addWidget(polarMin, 0, 1, 1, 1);


    vboxLayout7->addWidget(groupBox_13);

    groupBox_12 = new QGroupBox(page_4);
    groupBox_12->setObjectName(QString::fromUtf8("groupBox_12"));
    gridLayout11 = new QGridLayout(groupBox_12);
    gridLayout11->setSpacing(6);
    gridLayout11->setMargin(9);
    gridLayout11->setObjectName(QString::fromUtf8("gridLayout11"));
    polarLineWidth = new QDoubleSpinBox(groupBox_12);
    polarLineWidth->setObjectName(QString::fromUtf8("polarLineWidth"));
    polarLineWidth->setAlignment(Qt::AlignRight);
    polarLineWidth->setDecimals(1);
    polarLineWidth->setMinimum(0.1);
    polarLineWidth->setSingleStep(0.1);

    gridLayout11->addWidget(polarLineWidth, 0, 1, 1, 1);

    textLabel1_9 = new QLabel(groupBox_12);
    textLabel1_9->setObjectName(QString::fromUtf8("textLabel1_9"));

    gridLayout11->addWidget(textLabel1_9, 0, 0, 1, 1);

    polarLineColor = new KColorButton(groupBox_12);
    polarLineColor->setObjectName(QString::fromUtf8("polarLineColor"));

    gridLayout11->addWidget(polarLineColor, 1, 1, 1, 1);

    textLabel2_5 = new QLabel(groupBox_12);
    textLabel2_5->setObjectName(QString::fromUtf8("textLabel2_5"));

    gridLayout11->addWidget(textLabel2_5, 1, 0, 1, 1);


    vboxLayout7->addWidget(groupBox_12);

    spacerItem4 = new QSpacerItem(10, 10, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout7->addItem(spacerItem4);

    stackedWidget->addWidget(page_4);
    page_2 = new QWidget();
    page_2->setObjectName(QString::fromUtf8("page_2"));
    stackedWidget->addWidget(page_2);

    vboxLayout->addWidget(stackedWidget);

    spacerItem5 = new QSpacerItem(289, 16, QSizePolicy::Minimum, QSizePolicy::Expanding);

    vboxLayout->addItem(spacerItem5);

    textLabel1_4->setBuddy(cartesianEquation);
    textLabel1_3->setBuddy(cartesian_f1_lineWidth);
    textLabel2->setBuddy(cartesian_f1_lineColor);
    textLabel1_3_2_2->setBuddy(cartesian_f1_lineWidth);
    textLabel2_2_2->setBuddy(cartesian_f1_lineColor);
    textLabel2_2->setBuddy(cartesian_f2_lineColor);
    textLabel1_3_2->setBuddy(cartesian_f2_lineWidth);
    textLabel1_5->setBuddy(txtInitX);
    textLabel1_5_2->setBuddy(txtInitY);
    textLabel2_3->setBuddy(cartesian_F_lineColor);
    textLabel1_3_3->setBuddy(cartesian_F_lineWidth);
    textLabel4->setBuddy(parametricName);
    textLabel2_4->setBuddy(parametricLineColor);
    textLabel1_8->setBuddy(parametricLineColor);
    textLabel1_9->setBuddy(polarLineWidth);
    textLabel2_5->setBuddy(parametricLineColor);
    retranslateUi(FunctionEditorWidget);
    QObject::connect(cartesianCustomMin, SIGNAL(toggled(bool)), cartesianMin, SLOT(setEnabled(bool)));
    QObject::connect(cartesianCustomMax, SIGNAL(toggled(bool)), cartesianMax, SLOT(setEnabled(bool)));
    QObject::connect(parametricCustomMin, SIGNAL(toggled(bool)), parametricMin, SLOT(setEnabled(bool)));
    QObject::connect(parametricCustomMax, SIGNAL(toggled(bool)), parametricMax, SLOT(setEnabled(bool)));
    QObject::connect(polarCustomMin, SIGNAL(toggled(bool)), polarMin, SLOT(setEnabled(bool)));
    QObject::connect(polarCustomMax, SIGNAL(toggled(bool)), polarMax, SLOT(setEnabled(bool)));
    QObject::connect(showDerivative1, SIGNAL(toggled(bool)), groupBox_5, SLOT(setEnabled(bool)));
    QObject::connect(showDerivative2, SIGNAL(toggled(bool)), groupBox_4, SLOT(setEnabled(bool)));
    QObject::connect(parametricName, SIGNAL(textChanged(QString)), textLabelXF, SLOT(setText(QString)));
    QObject::connect(parametricName, SIGNAL(textChanged(QString)), textLabelYF, SLOT(setText(QString)));
    QObject::connect(cartesianParametersList, SIGNAL(toggled(bool)), editParameterListButton, SLOT(setEnabled(bool)));
    QObject::connect(cartesianParameterSlider, SIGNAL(toggled(bool)), listOfSliders, SLOT(setEnabled(bool)));
    QObject::connect(showIntegral, SIGNAL(toggled(bool)), groupBox_6, SLOT(setEnabled(bool)));
    QObject::connect(showIntegral, SIGNAL(toggled(bool)), groupBox_7, SLOT(setEnabled(bool)));
    QObject::connect(showIntegral, SIGNAL(toggled(bool)), groupBox_8, SLOT(setEnabled(bool)));

    stackedWidget->setCurrentIndex(0);


    QMetaObject::connectSlotsByName(FunctionEditorWidget);
    } // setupUi

    void retranslateUi(QWidget *FunctionEditorWidget)
    {
    FunctionEditorWidget->setWindowTitle(tr2i18n("Form", 0));
    createCartesian->setToolTip(tr2i18n("define a new function", 0));
    createCartesian->setWhatsThis(tr2i18n("Click here to define a new function. There are 3 types of function, explicit given functions, parametric plots and polar plots. Choose the appropriate type from the drop down list.", 0));
    createCartesian->setText(tr2i18n("New &Cartesian Plot", 0));
    createParametric->setToolTip(tr2i18n("define a new function", 0));
    createParametric->setWhatsThis(tr2i18n("Click here to define a new function. There are 3 types of function, explicit given functions, parametric plots and polar plots. Choose the appropriate type from the drop down list.", 0));
    createParametric->setText(tr2i18n("New &Parametric Plot", 0));
    createPolar->setToolTip(tr2i18n("define a new function", 0));
    createPolar->setWhatsThis(tr2i18n("Click here to define a new function. There are 3 types of function, explicit given functions, parametric plots and polar plots. Choose the appropriate type from the drop down list.", 0));
    createPolar->setText(tr2i18n("&New Polar Plot", 0));
    deleteButton->setToolTip(tr2i18n("delete the selected function", 0));
    deleteButton->setWhatsThis(tr2i18n("Click here to delete the selected function from the list.", 0));
    deleteButton->setText(tr2i18n("&Delete", 0));
    textLabel1_4->setText(tr2i18n("&Equation:", 0));
    cartesianEquation->setToolTip(tr2i18n("enter an equation, for instance f(x)=x^2", 0));
    cartesianEquation->setWhatsThis(tr2i18n("Enter an equation for the function.\n"
"Example: f(x)=x^2", 0));
    cartesianHideExtensions->setToolTip(tr2i18n("hide the plot", 0));
    cartesianHideExtensions->setWhatsThis(tr2i18n("Check this box if you want to hide the plot of the function.", 0));
    cartesianHideExtensions->setText(tr2i18n("&Hide extensions", 0));
    cartesianParameters->setTitle(tr2i18n("Parameter Values", 0));
    cartesianDisableParameters->setText(tr2i18n("Disable", 0));
    cartesianParametersList->setToolTip(tr2i18n("read parameter values from a list", 0));
    cartesianParametersList->setWhatsThis(tr2i18n("Check this to let KmPlot use parameter values given in a list. Edit this list by clicking the button on the right.", 0));
    cartesianParametersList->setText(tr2i18n("From a list:", 0));
    listOfSliders->setToolTip(tr2i18n("Select a slider", 0));
    listOfSliders->setWhatsThis(tr2i18n("Select one of the sliders to change the parameter value dynamically. The values vary from 0 (left) to 100 (right).", 0));
    cartesianParameterSlider->setToolTip(tr2i18n("change parameter value by moving a slider", 0));
    cartesianParameterSlider->setWhatsThis(tr2i18n("Check this to change the parameter value by moving a slider. Select the slider from the list box on the right. The values vary from 0 (left) to 100 (right).", 0));
    cartesianParameterSlider->setText(tr2i18n("Use a slider:", 0));
    editParameterListButton->setToolTip(tr2i18n("Edit the list of parameters", 0));
    editParameterListButton->setWhatsThis(tr2i18n("Click here to open a list of parameter values. Here you can add, remove, and change them.", 0));
    editParameterListButton->setText(tr2i18n("Edit List...", 0));
    groupBox->setTitle(tr2i18n("Custom plot range", 0));
    cartesianMax->setToolTip(tr2i18n("upper boundary of the plot range", 0));
    cartesianMax->setWhatsThis(tr2i18n("Enter the upper boundary of the plot range. Expressions like 2*pi are allowed, too.", 0));
    cartesianCustomMax->setToolTip(tr2i18n("Customize the maximum plot range", 0));
    cartesianCustomMax->setWhatsThis(tr2i18n("Check this button and enter the plot range boundarys below.", 0));
    cartesianCustomMax->setText(tr2i18n("Max:", 0));
    cartesianCustomMin->setToolTip(tr2i18n("Customize the minimum plot range", 0));
    cartesianCustomMin->setWhatsThis(tr2i18n("Check this button and enter the plot range boundarys below.", 0));
    cartesianCustomMin->setText(tr2i18n("Min:", 0));
    cartesianMin->setToolTip(tr2i18n("lower boundary of the plot range", 0));
    cartesianMin->setWhatsThis(tr2i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too.", 0));
    groupBox_2->setTitle(tr2i18n("", 0));
    cartesian_f_lineWidth->setSuffix(tr2i18n(" mm", 0));
    textLabel1_3->setText(tr2i18n("Line &width:", 0));
    cartesian_f_lineColor->setToolTip(tr2i18n("color of the plot line", 0));
    cartesian_f_lineColor->setWhatsThis(tr2i18n("Click this button to choose a color for the plot line.", 0));
    cartesian_f_lineColor->setText(tr2i18n("", 0));
    textLabel2->setText(tr2i18n("Co&lor:", 0));
    tabWidget->setTabText(tabWidget->indexOf(tab), tr2i18n("Function", 0));
    showDerivative1->setToolTip(tr2i18n("Show first derivative", 0));
    showDerivative1->setWhatsThis(tr2i18n("If this box is checked, the first derivative will be plotted, too.", 0));
    showDerivative1->setText(tr2i18n("Show &1st derivative", 0));
    groupBox_5->setTitle(tr2i18n("1st derivative", 0));
    cartesian_f1_lineWidth->setSuffix(tr2i18n(" mm", 0));
    textLabel1_3_2_2->setText(tr2i18n("&Line width:", 0));
    textLabel2_2_2->setText(tr2i18n("&Color:", 0));
    cartesian_f1_lineColor->setToolTip(tr2i18n("color of the plot line", 0));
    cartesian_f1_lineColor->setWhatsThis(tr2i18n("Click this button to choose a color for the plot line.", 0));
    cartesian_f1_lineColor->setText(tr2i18n("", 0));
    showDerivative2->setToolTip(tr2i18n("Show second derivative", 0));
    showDerivative2->setWhatsThis(tr2i18n("If this box is checked, the first derivative will be plotted, too.", 0));
    showDerivative2->setText(tr2i18n("Show &2nd derivative", 0));
    groupBox_4->setTitle(tr2i18n("2nd derivative", 0));
    cartesian_f2_lineWidth->setSuffix(tr2i18n(" mm", 0));
    cartesian_f2_lineColor->setToolTip(tr2i18n("color of the plot line", 0));
    cartesian_f2_lineColor->setWhatsThis(tr2i18n("Click this button to choose a color for the plot line.", 0));
    cartesian_f2_lineColor->setText(tr2i18n("", 0));
    textLabel2_2->setText(tr2i18n("Colo&r:", 0));
    textLabel1_3_2->setText(tr2i18n("Line &width:", 0));
    tabWidget->setTabText(tabWidget->indexOf(tab_3), tr2i18n("Derivatives", 0));
    showIntegral->setToolTip(tr2i18n("hide the plot", 0));
    showIntegral->setWhatsThis(tr2i18n("Check this box if you want to hide the plot of the function.", 0));
    showIntegral->setText(tr2i18n("Show integral", 0));
    groupBox_6->setTitle(tr2i18n("Initial Point", 0));
    textLabel1_5->setText(tr2i18n("&x-value:", 0));
    txtInitX->setToolTip(tr2i18n("Enter the initial x-point,for instance 2 or pi", 0));
    txtInitX->setWhatsThis(tr2i18n("Enter the initial x-value or expression for the integral, for example 2 or pi/2", 0));
    txtInitY->setToolTip(tr2i18n("enter the initial y-point, eg 2 or pi", 0));
    txtInitY->setWhatsThis(tr2i18n("Enter the initial y-value or expression for the integral, for example 2 or pi/2", 0));
    textLabel1_5_2->setText(tr2i18n("&y-value:", 0));
    groupBox_8->setTitle(tr2i18n("", 0));
    customPrecision->setToolTip(tr2i18n("Customize the precision", 0));
    customPrecision->setText(tr2i18n("Custom &precision:", 0));
    groupBox_7->setTitle(tr2i18n("", 0));
    cartesian_F_lineWidth->setSuffix(tr2i18n(" mm", 0));
    textLabel2_3->setText(tr2i18n("Color:", 0));
    cartesian_F_lineColor->setToolTip(tr2i18n("color of the plot line", 0));
    cartesian_F_lineColor->setWhatsThis(tr2i18n("Click this button to choose a color for the plot line.", 0));
    cartesian_F_lineColor->setText(tr2i18n("", 0));
    textLabel1_3_3->setText(tr2i18n("Line width:", 0));
    tabWidget->setTabText(tabWidget->indexOf(tab_2), tr2i18n("Integral", 0));
    groupBox_11->setTitle(tr2i18n("Definition", 0));
    textLabel4->setText(tr2i18n("Name:", 0));
    parametricX->setToolTip(tr2i18n("enter an expression", 0));
    parametricX->setWhatsThis(tr2i18n("Enter an expression for the function.\n"
"The dummy variable is t.\n"
"Example: cos(t)", 0));
    parametricY->setToolTip(tr2i18n("enter an expression", 0));
    parametricY->setWhatsThis(tr2i18n("Enter an expression for the function.\n"
"The dummy variable is t.\n"
"Example: sin(t)", 0));
    parametricHideExtensions->setToolTip(tr2i18n("hide the plot", 0));
    parametricHideExtensions->setWhatsThis(tr2i18n("Check this box if you want to hide the plot of the function.", 0));
    parametricHideExtensions->setText(tr2i18n("Hide extensions", 0));
    parametricName->setToolTip(tr2i18n("name of the function", 0));
    parametricName->setWhatsThis(tr2i18n("Enter the name of the function.\n"
"The name of a function must be unique. If you leave this line empty KmPlot will set a default name. You can change it later.", 0));
    parametricName->setText(tr2i18n("f", 0));
    textLabelY->setText(tr2i18n("y", 0));
    textLabelYF->setText(tr2i18n("f", 0));
    textLabelArgY->setText(tr2i18n("(t) =", 0));
    textLabelX->setText(tr2i18n("x", 0));
    textLabelXF->setText(tr2i18n("f", 0));
    textLabelArgX->setText(tr2i18n("(t) =", 0));
    groupBox_9->setTitle(tr2i18n("Custom plot range", 0));
    parametricMin->setToolTip(tr2i18n("lower boundary of the plot range", 0));
    parametricMin->setWhatsThis(tr2i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too.", 0));
    parametricMax->setToolTip(tr2i18n("upper boundary of the plot range", 0));
    parametricMax->setWhatsThis(tr2i18n("Enter the upper boundary of the plot range. Expressions like 2*pi are allowed, too.", 0));
    parametricCustomMin->setToolTip(tr2i18n("Customize the minimum plot range", 0));
    parametricCustomMin->setWhatsThis(tr2i18n("Check this button and enter the plot minimum range boundary below.", 0));
    parametricCustomMin->setText(tr2i18n("Min:", 0));
    parametricCustomMax->setToolTip(tr2i18n("Customize the maximum plot range", 0));
    parametricCustomMax->setWhatsThis(tr2i18n("Check this button and enter the plot maximum range boundary below.", 0));
    parametricCustomMax->setText(tr2i18n("Max:", 0));
    groupBox_10->setTitle(tr2i18n("", 0));
    parametricLineWidth->setSuffix(tr2i18n(" mm", 0));
    textLabel2_4->setText(tr2i18n("Colo&r:", 0));
    parametricLineColor->setToolTip(tr2i18n("color of the plot line", 0));
    parametricLineColor->setWhatsThis(tr2i18n("Click this button to choose a color for the plot line.", 0));
    parametricLineColor->setText(tr2i18n("", 0));
    textLabel1_8->setText(tr2i18n("Line &width:", 0));
    textLabel2_6->setText(tr2i18n("Equation:", 0));
    polarEquation->setToolTip(tr2i18n("enter an equation, for instance loop(angle)=ln(angle)", 0));
    polarEquation->setWhatsThis(tr2i18n("Enter an expression for the function. The prefix \"r\" will be added automatically.\n"
"Example: loop(angle)=ln(angle)", 0));
    polarHideExtensions->setToolTip(tr2i18n("hide the plot", 0));
    polarHideExtensions->setWhatsThis(tr2i18n("Check this box if you want to hide the plot of the function.", 0));
    polarHideExtensions->setText(tr2i18n("Hide extensions", 0));
    groupBox_13->setTitle(tr2i18n("Custom plot range", 0));
    polarMax->setToolTip(tr2i18n("upper boundary of the plot range", 0));
    polarMax->setWhatsThis(tr2i18n("Enter the upper boundary of the plot range. Expressions like 2*pi are allowed, too.", 0));
    polarCustomMin->setToolTip(tr2i18n("Customize the plot range", 0));
    polarCustomMin->setWhatsThis(tr2i18n("Check this button and enter the plot minimum range boundary below.", 0));
    polarCustomMin->setText(tr2i18n("Min:", 0));
    polarCustomMax->setToolTip(tr2i18n("Customize the maximum plot range", 0));
    polarCustomMax->setWhatsThis(tr2i18n("Check this button and enter the plot maximum range boundary below.", 0));
    polarCustomMax->setText(tr2i18n("Max:", 0));
    polarMin->setToolTip(tr2i18n("lower boundary of the plot range", 0));
    polarMin->setWhatsThis(tr2i18n("Enter the lower boundary of the plot range. Expressions like 2*pi are allowed, too.", 0));
    groupBox_12->setTitle(tr2i18n("", 0));
    polarLineWidth->setSuffix(tr2i18n(" mm", 0));
    textLabel1_9->setText(tr2i18n("Line &width:", 0));
    polarLineColor->setToolTip(tr2i18n("color of the plot line", 0));
    polarLineColor->setWhatsThis(tr2i18n("Click this button to choose a color for the plot line.", 0));
    polarLineColor->setText(tr2i18n("", 0));
    textLabel2_5->setText(tr2i18n("Colo&r:", 0));
    Q_UNUSED(FunctionEditorWidget);
    } // retranslateUi

};

namespace Ui {
    class FunctionEditorWidget: public Ui_FunctionEditorWidget {};
} // namespace Ui

#endif // FUNCTIONEDITORWIDGET_H
