
#include "JiveParamSlider.h"

class MidiBindingEditorContent : public Component, public ComboBoxListener
{
public:
   MidiBindingEditorContent()
   {
      addAndMakeVisible(noteOnOrOff = new ComboBox("noteOnOrOff"));
      noteOnOrOff->addItem("Note on", 1);
      noteOnOrOff->addItem("Note off", 2);
      addAndMakeVisible(incrDirection = new ComboBox("incrDirection")); 
      incrDirection->addItem("Increase", 1);
      incrDirection->addItem("Decrease", -1);
      incrDirection->addItem("Bidi", 2);
      addAndMakeVisible(incrMax = new Slider("incrMax")); 
      incrMax->setSliderStyle(Slider::IncDecButtons);
      addAndMakeVisible(incrAmount = new ComboBox("incrAmount")); 
      incrAmount->addItem("1 (toggle)", 1);
      incrAmount->addItem("1/2", 2);
      incrAmount->addItem("1/3", 3);
      incrAmount->addItem("1/4", 4);
      incrAmount->addItem("1/5", 5);
      incrAmount->addItem("1/8", 8);
      incrAmount->addItem("1/16", 16);
      incrAmount->addItem("1/32", 32);
      incrAmount->addItem("1/127", 127);
      incrAmount->addListener(this);

      setSize(400, 200);
   }
   
   ~MidiBindingEditorContent()
   {
      deleteAllChildren();
   }
   
   virtual void comboBoxChanged (ComboBox* slider)
   {
      if (slider == incrAmount)
      {
         int realMax = incrAmount->getSelectedId();
         incrMax->setRange(1, realMax, 1);
         if (realMax < incrMax->getValue())
            incrMax->setValue(realMax);
      }
   }

   void resized() 
   {
      int h = getHeight() / 4;
      int w = getWidth();
      noteOnOrOff->setBounds(0, 0, w, h);
      incrDirection->setBounds(0, h, w, h);
      incrAmount->setBounds(0, 2*h, w, h);
      incrMax->setBounds(0, 3*h, w, h);
   }
   

   ComboBox* noteOnOrOff;
   ComboBox* incrDirection;
   ComboBox* incrAmount;
   Slider* incrMax;
};

void ParamSlider::mouseDown(const MouseEvent& e)
{
//      Slider::mouseDown(e);
//
//      if (e.mods.isRightButtonDown())
//         managedParam->handleMidiPopupMenu(e);
   if (managedParam && e.mods.isRightButtonDown ())
   {
      PopupMenu menu = managedParam->generateMidiPopupMenu();

      // TODO move this into Jive code
      menu.addItem (3, "Edit", (managedParam->getNoteNumber() != -1));

      int result = menu.showAt (e.getScreenX(), e.getScreenY());

      bool handlerd = managedParam->processMidiPopupMenu(result);
      
      if (!handlerd && result == 3)
      {
         if (managedParam->getNoteNumber() != -1)
         {
            MidiBindingEditorContent dialogStuff;
            double incrAmount = managedParam->getIncrAmount();
            int incr = round(1.0 / fabs(incrAmount));
            int imax = managedParam->getIncrMax() / fabs(incrAmount);//round(1.0 / fabs(incrMax));
            dialogStuff.incrAmount->setSelectedId(incr, false);
            dialogStuff.noteOnOrOff->setSelectedId(managedParam->isNoteOn() ? 1 : 2);
            int dir = 1;
            if (managedParam->isBidirectional()) dir = 2;
            else if (incrAmount < 0) dir = -1;
            dialogStuff.incrDirection->setSelectedId(dir);
            dialogStuff.incrMax->setValue(imax);
            
            DialogWindow::showModalDialog(String("Edit Note Binding"), &dialogStuff, 0, Colours::brown, true);
            
            incrAmount = 1.0 / dialogStuff.incrAmount->getSelectedId();
            managedParam->setIncrMax(incrAmount * dialogStuff.incrMax->getValue());
            managedParam->setNoteOn(1 == dialogStuff.noteOnOrOff->getSelectedId());
            managedParam->setBidirectional(2 == dialogStuff.incrDirection->getSelectedId() && incrAmount < 1.0);
            if (dialogStuff.incrDirection->getSelectedId() == -1)
               managedParam->setIncrAmount(-incrAmount);
//            
//            // ensure midi manag knows about note on / off
//            if (managedParam->midiAutomatorManager)
//               managedParam->midiAutomatorManager->registerMidiAutomatable(this);
         }      
      }
   }

   Slider::mouseDown (e);
}
