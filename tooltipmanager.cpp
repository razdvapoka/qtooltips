#include "tooltipmanager.h"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QCommandLinkButton>
#include <QDateEdit>
#include <QDateTimeEdit>
#include <QDial>
#include <QFontComboBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QTimeEdit>
#include <QToolBox>
#include <QToolButton>
#include <QTreeWidget>

#include <typeinfo>

// >>> Static members initialization.

// Pointer to the only one TooltipManager instance.
TooltipManager* TooltipManager::manager = 0;

QWidget* TooltipManager::currentVictim = 0;

QString TooltipManager::tooltipStyleSheet = "color: white;"
                                            "background-color: black;"
                                            "min-height: 20;"
                                            "max-height: 20;"
                                            "min-width: 20;"
                                            "max-width: 20;";

Qt::Key TooltipManager::triggerKey = Qt::Key_Control;

// <<<


// Active manager grasps every active window.
void TooltipManager::setActive(bool active)
{
    if(active)
    {
        if(!manager)
            manager = new TooltipManager();
        connect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), manager, SLOT(focusChanged()));
    }
    else
        // it's safe even if this connection hasn't been established or manager = 0
        disconnect(qApp, SIGNAL(focusChanged(QWidget*,QWidget*)), manager, SLOT(focusChanged()));
}


// Grasps the victim and provide it with tooltips.
void TooltipManager::grasp(QWidget *victim)
{
    if(victim)
    {
        if(currentVictim)
            currentVictim->removeEventFilter(manager);

        currentVictim = victim;

        if(manager)
            manager->triggerKeyPressed = false;
        else
            manager = new TooltipManager();

        currentVictim->installEventFilter(manager);

        connect(currentVictim, SIGNAL(destroyed()), manager, SLOT(victimDestroyed()));
    }
}


// The private constructor.
TooltipManager::TooltipManager()
{
    triggerKeyPressed = false;
}


// Sets the key that shows the tooltips.
void TooltipManager::setTriggerKey(Qt::Key key)
{
    triggerKey = key;
}


// This method is called when any key on the keyboard is pressed.
// It processes the received keycode and returns:
// true - if it has done something and the default processing must be omitted.
// false - if received keycode implies no processing
//         and event must be passed further for the default processing.
bool TooltipManager::notify(int key)
{
    if(key == triggerKey)
    {
        if(triggerKeyPressed)
        {
            if(!widgetsQueue.isEmpty())
            {
                hideTooltips();
                createTooltips(false);
                showTooltips();
            }
            else
            {
                triggerKeyPressed = false;
                hideTooltips();
            }
        }
        else
        {
            if(!currentVictim)
                currentVictim = qApp->activeWindow();

            triggerKeyPressed = true;
            createTooltips(true);
            showTooltips();
            currentVictim->setFocus();
        }
    }
    else
        if(triggerKeyPressed)
        {
            triggerKeyPressed = false;
            hideTooltips();

            if(widgets.contains(key))
                act(widgets[key].first);
            else
                return false;
        }
        else
            return false;

    return true;
}


void TooltipManager::createTooltips(bool refill)
{    
    foreach(int key, widgets.keys())
        delete widgets[key].second;
    widgets.clear();    

    int firstLetterThreshold,
        lastLetterThreshold;
    setThresholds(firstLetterThreshold, lastLetterThreshold);

    int key = firstLetterThreshold;

    // get widgets
    if(refill)
        refillQueue();

    while(inBounds(key, firstLetterThreshold) && !widgetsQueue.isEmpty())
    {
        QWidget *child = widgetsQueue.dequeue();
        widgets[key] = QPair<QWidget*, QLabel*>(child, createTooltip(key, child));
        key++;
        if(key > lastLetterThreshold)
            key = FirstNumber;
    }
}


// Sets the range of letters to put into tooltips:
// [A..Z] or [А..Я].
void TooltipManager::setThresholds(int& firstLetterThreshold, int& lastLetterThreshold)
{
    if(QLocale::system().country() == QLocale::RussianFederation)
    {
        firstLetterThreshold = FirstRussianLetter;
        lastLetterThreshold = LastRussianLetter;
    }
    else // QLocale::UnitedStates, I hope.
    {
        firstLetterThreshold = FirstEnglishLetter;
        lastLetterThreshold = LastEnglishLetter;
    }
}


// Refills widgetsQueue with the widgets
// we want to provide with tooltips.
void TooltipManager::refillQueue()
{
    widgetsQueue.clear();
    foreach(QWidget *child, currentVictim->findChildren<QWidget *>())
    {
        if( hasProperType(child)
            && !isExcluded(child)
            && child->isVisible()
            && !isObscured(child)
          )
            widgetsQueue.enqueue(child);
    }
}


// Returns true if the widget
// has a type that requires a tooltip.
bool TooltipManager::hasProperType(QWidget *widget)
{
    return (typeid(*widget) == typeid(QDial))
        || (typeid(*widget) == typeid(QLineEdit))
        || (typeid(*widget) == typeid(QListWidget))
        || (typeid(*widget) == typeid(QPlainTextEdit))
        || (typeid(*widget) == typeid(QTabBar))
        || (typeid(*widget) == typeid(QTableWidget))
        || (typeid(*widget) == typeid(QTextEdit))
        || (typeid(*widget) == typeid(QToolBox))
        || (typeid(*widget) == typeid(QTreeWidget))
        || dynamic_cast<QAbstractSpinBox*>(widget)
        || dynamic_cast<QComboBox*>(widget)
        || dynamic_cast<QAbstractButton*>(widget);
}


// Checks whether the widget is excluded from tooltip generating process.
bool TooltipManager::isExcluded(QWidget *widget)
{
    // If the widget's parent already has a tooltip
    // then, usually, the widget itself doesn't need one.
    return widget->parentWidget() &&
            (    (typeid(*widget->parentWidget()) == typeid(QDateEdit))
              || (typeid(*widget->parentWidget()) == typeid(QDateTimeEdit))
              || (typeid(*widget->parentWidget()) == typeid(QDoubleSpinBox))
              || (typeid(*widget->parentWidget()) == typeid(QFontComboBox))
              || (typeid(*widget->parentWidget()) == typeid(QSpinBox))
              || (typeid(*widget->parentWidget()) == typeid(QTabBar))
              || (typeid(*widget->parentWidget()) == typeid(QTimeEdit))
            );
}


// 9...key...firstLetterThreshold = false
bool TooltipManager::inBounds(int key, int firstLetterThreshold)
{
    return !((key > TooltipManager::LastNumber) & (key < firstLetterThreshold));
}


// Returns true if the widget is fully obscured by smth else.
bool TooltipManager::isObscured(QWidget *widget)
{
    return !widget->visibleRegion().intersects(widget->rect());
}


// Creates a tooltip (a QLabel with a letter or number in it) attached to some widget
// so that, when shown, it appears near this widget.
// The 'key' parameter is the int code of the letter the tooltip contains.
// It's also the key of the 'widgets' hashtable.
QLabel *TooltipManager::createTooltip(QChar key, QWidget *widget)
{
    QLabel *tooltip = new QLabel(key, currentVictim);
    tooltip->setStyleSheet(tooltipStyleSheet);
    tooltip->setAlignment(Qt::AlignCenter); // QLabel doesn't support stylesheet alignment
    tooltip->move(getTooltipPos(widget));
    tooltip->setHidden(true);
    return tooltip;
}


// Returns a position for a widget's tooltip
QPoint TooltipManager::getTooltipPos(QWidget *widget)
{
    return widget->mapTo(currentVictim, widget->rect().topLeft());
}


void TooltipManager::showTooltips()
{
    foreach(int key, widgets.keys())
        widgets[key].second->show();
}


void TooltipManager::hideTooltips()
{
    foreach(int key, widgets.keys())
        widgets[key].second->hide();
}


// Activates widget after the corresponding key was pressed.
// 'Activation' means executing some predefined method of a widget,
// e.g. click() for QPushButton and setFocus() for QLineEdit.
void TooltipManager::act(QWidget* widget)
{
    // unfortunately, we can't switch over type_info
    if (
            typeid(*widget) == typeid(QToolButton)
       )
    {
        QToolButton* toolButton = (QToolButton*)widget;
        QList<QAction*> actions = toolButton->actions();
        if(!actions.isEmpty() && actions.first()->menu())
            actions.first()->menu()->popup(toolButton->mapToGlobal(toolButton->rect().topRight()));
        else
            toolButton->click();
    }
    else
    if (
         dynamic_cast<QAbstractButton*>(widget)
       )
    {
        dynamic_cast<QAbstractButton*>(widget)->click();
    }
    else
    if (
         dynamic_cast<QComboBox*>(widget)
       )
    {
        (dynamic_cast<QComboBox*>(widget))->showPopup();
        currentVictim->setFocus();
    }
    else
    if (
        (typeid(*widget) == typeid(QToolBox))
       )
    {
        QToolBox *toolBox = (QToolBox *)widget;
        toolBox->setCurrentIndex((toolBox->currentIndex() + 1) % toolBox->count());
    }
    else
    {
        widget->setFocus();
    }
}


// Catches all events received by the parent
// and processes them (see TooltipManager::create function).
bool TooltipManager::eventFilter(QObject *obj, QEvent *event)
{
    switch(event->type())
    {
        case QEvent::KeyPress:
        {
             QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
             if (notify(keyEvent->key()))
                 return true;
             break;
        }
        case QEvent::Resize:
        {
            relocateTooltips();
            break;
        }
        default: {} // no warnings!
    }

    // standard event processing
    return QObject::eventFilter(obj, event);
}


// Relocates the tooltips after the victim's resizing.
void TooltipManager::relocateTooltips()
{
    if(manager->triggerKeyPressed)
        foreach(int key, widgets.keys())
        {
            widgets[key].second->move(getTooltipPos(widgets[key].first));
            widgets[key].second->setVisible(!isObscured(widgets[key].first));
        }
}


// >> SLOTS

// Manager grasps new active window
// finishing work done on the previous one
void TooltipManager::focusChanged()
{
    if(qApp->activeWindow() &&
       (!currentVictim ||
        currentVictim != qApp->activeWindow()))
    {
        if(currentVictim)
        {
            manager->hideTooltips();
            foreach(int key, manager->widgets.keys())
                delete widgets[key].second;
            manager->widgets.clear();
        }
        grasp(qApp->activeWindow());
    }
}

// Pointers to dead objects cause much trouble
void TooltipManager::victimDestroyed()
{
    manager->widgets.clear();
    currentVictim = 0;
}

// << SLOTS
