
#include <dcopobject.h>

class MainDlgIface : virtual public DCOPObject
 {
     K_DCOP
     k_dcop:
 
        virtual void editColors() = 0;
        virtual void editAxes() = 0;
        virtual void editScaling() = 0;
        virtual void editFonts() = 0;
        virtual void editConstants() = 0;
        virtual void newFunction() = 0;
        virtual void newParametric() = 0;
        virtual void newPolar() = 0;
        virtual void toggleShowSlider0() = 0;
        virtual void toggleShowSlider1() = 0;
        virtual void toggleShowSlider2() = 0;
        virtual void toggleShowSlider3() = 0;
        virtual void slotOpen() = 0;
        virtual void slotOpenNew() = 0;
        virtual void slotSave() = 0;
        virtual void slotSaveas() = 0;
        virtual void slotEditPlots() = 0;
        virtual void slotPrint() = 0;
        virtual void slotExport() = 0;
        virtual void slotSettings() = 0;
        virtual void slotNames() = 0;
        virtual void slotCoord1() = 0;
        virtual void slotCoord2() = 0;
        virtual void slotCoord3() = 0;
        virtual void getYValue() = 0;
        virtual void findMinimumValue() = 0;
        virtual void findMaximumValue() = 0;
        virtual void graphArea() = 0;
};