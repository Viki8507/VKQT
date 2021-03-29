#include "CompleterTextEdit.h"
#include <QCompleter>
#include <QAbstractItemView>

VKQT::CompleterTextEdit::CompleterTextEdit(QWidget *parent)
    : QTextEdit(parent)
{
    connect(this, &QTextEdit::textChanged, this, [this]{
        if (_pCompleter)
        {
            if (toPlainText().isEmpty())
            {
                _pCompleter->popup()->hide();
            }
            else if (this->hasFocus())
            {
                _pCompleter->setCompletionPrefix(this->toPlainText());
                _pCompleter->complete();
            }
        }
    });
}

VKQT::CompleterTextEdit::~CompleterTextEdit()
{
}

void VKQT::CompleterTextEdit::setCompleter(QCompleter* completer)
{
    if (_pCompleter)
        _pCompleter->disconnect(this);

    _pCompleter = completer;
    if (_pCompleter)
    {
        _pCompleter->setWidget(this);
        connect(_pCompleter, QOverload<const QString&>::of(&QCompleter::activated), this, [this](const QString &text){
            this->setPlainText(text);
            this->moveCursor(QTextCursor::End);
            _pCompleter->popup()->hide();
        });
    }
}

QCompleter* VKQT::CompleterTextEdit::completer()
{
    return _pCompleter;
}
