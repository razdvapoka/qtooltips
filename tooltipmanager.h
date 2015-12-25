#ifndef TOOLTIPMANAGER_H
#define TOOLTIPMANAGER_H

#include <QObject>
#include <QQueue>
#include <QHash>
#include <QLabel>

class MapWidget;

class TooltipManager : public QObject
{
    Q_OBJECT

public:

    static void setActive(bool active);
    static void grasp(QWidget *victim);
    static void setTriggerKey(Qt::Key key);

private:

    enum KeyThresholds
    {
        FirstNumber = Qt::Key_0,
        LastNumber = Qt::Key_9,
        FirstEnglishLetter = Qt::Key_A,
        LastEnglishLetter = Qt::Key_Z,
        FirstRussianLetter = 0x410,
        LastRussianLetter = 0x42f
    };

    TooltipManager();
    static TooltipManager *manager;
    static QWidget *currentVictim;
    static QString tooltipStyleSheet;
    static Qt::Key triggerKey;

    bool triggerKeyPressed;

    QHash<int, QPair<QWidget*, QLabel*> > widgets;
    QQueue<QWidget *> widgetsQueue;

    void setThresholds(int& first, int& last);
    void createTooltips(bool refill);
    void refillQueue();
    QPoint getTooltipPos(QWidget *widget);
    QLabel *createTooltip(QChar key, QWidget *widget);
    void showTooltips();
    void hideTooltips();
    bool notify(int key);
    void act(QWidget *widget);
    bool hasProperType(QWidget *widget);
    bool isExcluded(QWidget *widget);
    bool inBounds(int key, int first);
    bool isObscured(QWidget *widget);
    static int getTooltipHeight();
    static int getTooltipWidth();
    bool eventFilter(QObject *obj, QEvent *event);

private slots:

    void focusChanged();
    void victimDestroyed();
    void relocateTooltips();
};

#endif // TOOLTIPMANAGER_H
