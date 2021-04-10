#pragma once

#ifndef SIMPLEINI_H
#define SIMPLEINI_H

#include <QSettings>
#include <QString>
#include "Singleton.h"

// 单实例类宏实现方式

#define INI_BEGIN(ClassName, FileName) \
class SimpleIni_##ClassName{\
    DECLARESINGLETON(SimpleIni_##ClassName)\
public:\
SimpleIni_##ClassName(){\
    _setter = new QSettings(FileName, QSettings::IniFormat);\
    QStringList keys = _setter->allKeys();\
    foreach(QString key, keys) \
        _mapVals.insert(key, _setter->value(key));\
    }\
    ~SimpleIni_##ClassName() {delete _setter;}\
public:

#define INI_PARAM(Section, Type, Name, DefaultVal)\
    Type get_##Section##_##Name() const { return _mapVals.value(#Section"/"#Name, DefaultVal).value<Type>(); }\
    void set_##Section##_##Name(const Type &Name) {\
        Type val = get_##Section##_##Name();\
        if (val == Name) return;\
        val = Name;\
        _mapVals.insert(#Section"/"#Name, Name);\
        setValue(#Section"/"#Name, Name);\
}

#define INI_END \
    void setValue(const QString &name, const QVariant &val) { _setter->setValue(name, val); _setter->sync(); }\
    QVariant value(const QString &name, const QVariant &defaultVal) { return _setter->value(name, defaultVal); }\
private:\
    QSettings* _setter;\
    QMap<QString, QVariant> _mapVals;\
};

#define SIMPLE_INI(ClassName) (*(SINGLETON(SimpleIni_##ClassName)))

#endif // SIMPLEINI_H

#if 0 //Example
/* CustomIni.h */
INI_BEGIN(Custom, "D:/XXX.ini")
INI_PARAM(VK, QString, ProductName, "")
INI_END
#define CustomINI SIMPLE_INI(Custom)
/* 使用 */
void TestIni()
{
    QString productName = CustomINI.getProductName();
    CustomINI.setProductName("xxxx");
}
#endif
