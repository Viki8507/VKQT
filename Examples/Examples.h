#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Examples.h"

typedef std::function<QWidget*()> DemoWidgetCreator;
class VKQTDemo : public QMainWindow
{
    Q_OBJECT

public:
    VKQTDemo(QWidget *parent = Q_NULLPTR);

private:

    void    addDemoNaviItem(const QString& naviName, DemoWidgetCreator demoCreator);
    void    initDemoNavigation();

private:
    Ui::Examples ui;
};

Q_DECLARE_METATYPE(DemoWidgetCreator)

#define RegistryDemoWidget(n, cls) extern QList<QPair<QString, DemoWidgetCreator>> DemoCreatorList; \
static void Create##cls(){ \
    DemoCreatorList.append(QPair<QString,DemoWidgetCreator>{n, []{ return new cls();}}); \
}\
Q_COREAPP_STARTUP_FUNCTION(Create##cls)
