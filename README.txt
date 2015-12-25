TooltipManager (eventFilter version)

I. Functionality

Manual mode:

0) The user presses the trigger key (set with setTriggerKey method) on the keyboard and little labels 
with letters or numbers in them appear near some UI elements of the manager's victim.
1) If the user pushes a key with a letter or number that one of the labels 
contains then the UI element under this label is activated. 
'Activation' means executing some predefined method of the element that is activated. 
The method depends on the type of the element, 
e.g. 'click()' for buttons, 'showPopup()' for comboboxes. 
The default activation is just setting focus on the element 
(see act(QWidget *widget) method).
2) If the user pushes a key which none of the labels contains then the labels disappear.

Active mode:

The same as in the manual mode only the manager automatically switches between windows 
(grasps the currently activated window) of the application.


At once TooltipManager can show as many tooltips 
as there are number of the letters in the keyboard's current layout + 10 digits;
So, for English it is 36, for Russian - 42.
If the number of widgets on a form is greater than the aforementioned maximum 
then you can press the trigger key several times to see the next portion of tooltips.


II. Usage


In order to use TooltipManager do the following:
0) Add tooltipmanager.h to the project's headers
   or INCLUDEPATH += /path/to/folder/with/this/header to the project's .pro file.
1) Add "LIBS     += -L/path/to/lib -lTooltipManager" to the project's .pro file.

--------------------------------

Manual mode:

2) Add #include "tooltipmanager.h" to the header of any class 
   from where you can reference the widget you want to add tooltips to.

3) Call TooltipManager::grasp(&w) where "w" is the widget you want to add tooltips to.

    //e.g.
    MyWidget w;
    TooltipManager::grasp(&w);

---------------OR----------------

Active mode:

2) Add #include "tooltipmanager.h" to the source file or the header of the class
   where you'd like to activate the manager.

3) Call TooltipManager::setActive(true). To disable it call TooltipManager::setActive(false).

    // e.g
    ...
    #include "tooltipmanager.h"
    ...

    int main(int argc, char *argv[])
    {
        QApplication a(argc, argv);
        TooltipManager::setActive(true);
        MainWindow w;
        w.show();
        return a.exec();
    }

--------------------------------

4) That's it!

5) Also you can customize the appearence of the tooltips, 
   their shifts relatively to the widgets they are attached to,
   and the key that shows the tooltips.

    QString tooltipStyleSheet = "color: #FFF68E;" // Text color
                                "background-color: #8B864E;"
                                "min-height: 20;" // Set min-* and max-* equal to make * fixed.
                                "max-height: 20;"
                                "min-width: 20;"
                                "max-width: 20;";

    TooltipManager::setTooltipStyleSheet(tooltipStyleSheet);
    // the topleft corner of a tooltip moves left and up 
    // by 20px from the topleft corner of its base-widget.
    TooltipManager::setShifts(-20,-20);
    TooltipManager::setTriggerKey(Qt::Key_Control);


III. Extension


TooltipManager class can be extended so that additional UI elements get tooltips.
To extend TooltipManager do the following:
0) Add another || condition to the TooltipManager::hasProperType(QWidget *widget) method:

|| (typeid(*widget) == typeid(YourType))

1) That's all if you want to just focus on the widget after its tooltip key was pressed.
2) If you want to define some special behaviour then add one more 'if'-branch
   to the TooltipManager::act(QWidget *widget) method:

    if (typeid(*widget) == typeid(YourType))    
        ((YourType*)widget)->doSomething();

IV. Notes


The alphabet depends on the keyboard layout that is active when you press the trigger key.
TooltipManager knows about the Russian and English alphabets.
If you call TooltipManager while any other keyboard layout is active 
then it will still use the english alphabet.

