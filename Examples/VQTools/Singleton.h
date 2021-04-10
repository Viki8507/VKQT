#ifndef _KOMMANDER_TOOL_SINGLETON_H_
#define _KOMMANDER_TOOL_SINGLETON_H_

#include <functional>
#include <QStack>
#include <mutex>

typedef std::function<void()> DelSingletonAction;
typedef QStack<DelSingletonAction> SingletonDeleteStack;

void pushSingletonAction(DelSingletonAction delAction);
void destroySingleton();

extern QMap<QString, bool> g_singleDestroyed;
template<class T>
class KSingleton
{
public:
	template<typename... Args>
	static T*	instance(Args&&... args)
	{
		static T* _instance = nullptr;
		static std::once_flag oc;
		std::call_once(oc, [&] {
			_instance = new T(std::forward<Args>(args)...);
			g_singleDestroyed[typeid(T).name()] = false;
			if (_instance->AutoDestroy)
			{
				pushSingletonAction([=] { 
					qDebug("Delete Singleton : %s", typeid(T).name()); 
					g_singleDestroyed[typeid(T).name()] = true;
					delete _instance; 
				});
			}
		});
		return _instance;
	}

	static bool isDestroyed()
	{
		QString clsName = typeid(T).name();
		return g_singleDestroyed.contains(clsName) && g_singleDestroyed[clsName];
	}

private:
	KSingleton() {}
	virtual ~KSingleton() {}
	KSingleton(const KSingleton& st) {}
	KSingleton &operator=(const KSingleton& st) {}
};

typedef std::function<void()> SingletonInit;
typedef QVector<SingletonInit>	SingletonInitList;
extern SingletonInitList SingletonInitializer;

#define DECLARESINGLETON(clsname) public: friend class KSingleton<clsname>; bool AutoDestroy = true;
#define DECLARESINGLETON_NOTAUTODESTROY(clsname) public: friend class KSingleton<clsname>; bool AutoDestroy = false;

#define SINGLETON(x,...) KSingleton<x>::instance(__VA_ARGS__)
#define DESTROYSINGLETON	{ destroySingleton(); }
#define SINGLETONDESTROYED(T) KSingleton<T>::isDestroyed()
#define PREINITSINGLETON(T) Global(T##_PreInit_){ \
SingletonInitializer.append([]{SINGLETON(T);});\
}

#endif


