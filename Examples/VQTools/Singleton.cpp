#include "Singleton.h"
#include <QMap>
#include <QString>

#pragma init_seg(lib)
SingletonInitList SingletonInitializer;
SingletonDeleteStack g_singleDelStack;
QMap<QString, bool> g_singleDestroyed;

void pushSingletonAction(DelSingletonAction delAction)
{
	g_singleDelStack.push(delAction);
}

void destroySingleton()
{
	DelSingletonAction delAction = nullptr;
	while (!g_singleDelStack.isEmpty() && (delAction = g_singleDelStack.pop()))
	{
		delAction();
	}
}

