#pragma once

#include <QTextEdit>

class QCompleter;
namespace VKQT{
    class CompleterTextEdit : public QTextEdit
    {
        Q_OBJECT

    public:
        CompleterTextEdit(QWidget *parent);
        ~CompleterTextEdit();

        void    setCompleter(QCompleter* completer);
        QCompleter* completer();

    private:
        QCompleter*     _pCompleter = nullptr;
    };
}
