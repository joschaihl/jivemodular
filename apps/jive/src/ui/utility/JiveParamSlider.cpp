
#include "JiveParamSlider.h"
#include "MidiBindingsEditor.h"

void ParamSlider::mouseDown(const MouseEvent& e)
{
   if (managedParam && e.mods.isRightButtonDown ())
   {
      PopupMenu menu = managedParam->generateMidiPopupMenu();

      menu.addItem (3, "Edit Bindings...", true);

      int result = menu.showAt (e.getScreenX(), e.getScreenY());

      bool handlerd = managedParam->processMidiPopupMenu(result);
      
      if (!handlerd && result == 3)
      {
         MidiBindingsEditor ed(managedParam);
         DialogWindow::showModalDialog(String("Edit Parameter Bindings"), &ed, 0, Colours::green, true);
      }
   }
   else
      Slider::mouseDown (e);
}
