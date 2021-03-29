#include "Examples.h"
#include "VQDemoView/TimelineDemo.h"

QList<QPair<QString, DemoWidgetCreator>> DemoCreatorList;

VKQTDemo::VKQTDemo(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.listWidget, &QListWidget::currentItemChanged, this, [this](QListWidgetItem* curr, QListWidgetItem* prev){
        if (curr)
        {
            DemoWidgetCreator demoPageCreator = curr->data(Qt::UserRole).value<DemoWidgetCreator>();
            if (demoPageCreator)
            {
                QWidget* pageWidget = demoPageCreator();
                QLayoutItem* pOldPageItem = ui.verticalLayout->itemAt(0);
                ui.verticalLayout->replaceWidget(pOldPageItem->widget(), pageWidget);
                if (pOldPageItem)
                {
                    QWidget* oldWidget = pOldPageItem->widget();
                    if (oldWidget)
                        delete oldWidget;
                    delete pOldPageItem;
                }   
            }
        }
    });
    initDemoNavigation();
    ui.listWidget->setCurrentRow(0);
}

void VKQTDemo::addDemoNaviItem(const QString& naviName, DemoWidgetCreator demoCreator)
{
    QListWidgetItem* naviItem = new QListWidgetItem(naviName);
    naviItem->setData(Qt::UserRole, QVariant::fromValue<DemoWidgetCreator>(demoCreator));
    ui.listWidget->addItem(naviItem);
}

void VKQTDemo::initDemoNavigation()
{
    for (auto naviItem : DemoCreatorList)
    {
        addDemoNaviItem(naviItem.first, naviItem.second);
    }
}
