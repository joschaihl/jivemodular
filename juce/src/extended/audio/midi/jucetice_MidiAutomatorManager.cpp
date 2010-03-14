/*
 ==============================================================================

 This file is part of the JUCETICE project - Copyright 2009 by Lucio Asnaghi.

 JUCETICE is based around the JUCE library - "Jules' Utility Class Extensions"
 Copyright 2007 by Julian Storer.

 ------------------------------------------------------------------------------

 JUCE and JUCETICE can be redistributed and/or modified under the terms of
 the GNU General Public License, as published by the Free Software Foundation;
 either version 2 of the License, or (at your option) any later version.

 JUCE and JUCETICE are distributed in the hope that they will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with JUCE and JUCETICE; if not, visit www.gnu.org/licenses or write to
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA

 ==============================================================================
*/

#include "../../../core/juce_StandardHeader.h"

BEGIN_JUCE_NAMESPACE

#include "jucetice_MidiAutomatorManager.h"

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

//==============================================================================
MidiAutomatable::MidiAutomatable()
: 
  controllerNumber (-1),
  noteNumber(-1),
  noteOn(true),
  incrAmount(1.0),
  incrMax(1.0),
  bidirectional(false),
#if 0
    transfer (MidiAutomatable::Linear),
#endif
    midiAutomatorManager (0)
{
#if 0
    function = MakeDelegate (this, &MidiAutomatable::transferFunctionLinear);
#endif
}

MidiAutomatable::~MidiAutomatable()
{
    if (midiAutomatorManager)
        midiAutomatorManager->removeMidiAutomatable (this); 
}

void MidiAutomatable::activateLearning ()
{
    jassert (midiAutomatorManager != 0) // if you fallback here then you don't have registered your
                                        // midi automatable object to the manager before actually starting
                                        // the midi learn feature

    if (midiAutomatorManager)
        midiAutomatorManager->setActiveLearner (this);
}

void MidiAutomatable::setControllerNumber (const int control)
{
    if (controllerNumber != control)
    {
        controllerNumber = control;
        if (control != -1)
         setNoteNumber(-1);

        if (midiAutomatorManager)
            midiAutomatorManager->registerMidiAutomatable (this);    
    }
}

void MidiAutomatable::setNoteNumber (const int note)
{
    if (noteNumber != note)
    {
        noteNumber = note;
        if (note != -1)
         setControllerNumber(-1);

        if (midiAutomatorManager)
            midiAutomatorManager->registerMidiAutomatable (this);    
    }
}

#if 0
void MidiAutomatable::setTransferFunction (MidiTransferFunction newFunction)
{
    switch (newFunction)
    {
    default:
    case MidiAutomatable::Linear:
        function = MakeDelegate (this, &MidiAutomatable::transferFunctionLinear);
        break;
    case MidiAutomatable::InvertedLinear:
        function = MakeDelegate (this, &MidiAutomatable::transferFunctionLinear);
        break;
    }

    transfer = newFunction;
}
#endif

void MidiAutomatable::setMidiAutomatorManager (MidiAutomatorManager* newManager)
{
    midiAutomatorManager = newManager;
}

#if 0
float MidiAutomatable::transferFunctionLinear (int ccValue)
{
    return ccValue * float_MidiScaler;
}

float MidiAutomatable::transferFunctionInvertedLinear (int ccValue)
{
    return 1.0f - ccValue * float_MidiScaler;
}
#endif

void MidiAutomatable::handleMidiPopupMenu (const MouseEvent& e)
{
    PopupMenu menu, ccSubMenu, noteSubMenu;
    
    const int controllerNumber = getControllerNumber ();
    const int noteNumber = getNoteNumber ();

    for (int i = 0; i < 128; i++)
    {
         ccSubMenu.addItem (i + 1000,
                            "CC# " + String(i) + " " + MidiMessage::getControllerName (i),
                            true,
                            controllerNumber == i);
         noteSubMenu.addItem (i + 2000,
                            "Note# " + String(i) + " " + MidiMessage::getMidiNoteName (i, true, true, 4),
                            true,
                            noteNumber == i);
    }
    
    if (controllerNumber != -1)
        menu.addItem (-1, "Assigned to CC# " + String (controllerNumber), false);
    else if (noteNumber != -1)
        menu.addItem (-1, "Assigned to Note# " + String (noteNumber), false);
    else
        menu.addItem (-1, "Not assigned", false);
    menu.addSeparator ();

    menu.addItem (1, "Midi Learn");
    menu.addSubMenu ("Set CC", ccSubMenu);
    menu.addSubMenu ("Set Note", noteSubMenu);
    menu.addItem (2, "Reset", (controllerNumber != -1) || (noteNumber != -1 ));
    menu.addItem (3, "Edit", (noteNumber != -1));
    
    int result = menu.showAt (e.getScreenX(), e.getScreenY());
    switch (result)
    {
    case 1:
        activateLearning ();
        break;
    case 2:
        setControllerNumber (-1);
        setNoteNumber (-1);
        break;
   case 3:
   {
      if (getNoteNumber() != -1)
      {
         MidiBindingEditorContent dialogStuff;
         int incr = round(1.0 / fabs(incrAmount));
         int imax = incrMax / fabs(incrAmount);//round(1.0 / fabs(incrMax));
         dialogStuff.incrAmount->setSelectedId(incr, false);
         dialogStuff.noteOnOrOff->setSelectedId(noteOn ? 1 : 2);
         int dir = 1;
         if (bidirectional) dir = 2;
         else if (incrAmount < 0) dir = -1;
         dialogStuff.incrDirection->setSelectedId(dir);
         dialogStuff.incrMax->setValue(imax);
         
         DialogWindow::showModalDialog(String("Edit Note Binding"), &dialogStuff, 0, Colours::brown, true);
         
         incrAmount = 1.0 / dialogStuff.incrAmount->getSelectedId();
         incrMax = incrAmount * dialogStuff.incrMax->getValue();
         noteOn = (1 == dialogStuff.noteOnOrOff->getSelectedId());
         bidirectional = (2 == dialogStuff.incrDirection->getSelectedId() && incrAmount < 1.0);
         if (dialogStuff.incrDirection->getSelectedId() == -1)
            incrAmount = -incrAmount;
         
         // ensure midi manag knows about note on / off
         if (midiAutomatorManager)
            midiAutomatorManager->registerMidiAutomatable(this);
      }
   }
   break;
    default:
        if (result >= 1000 && result < 1128)
            setControllerNumber (result - 1000);
        else if (result >= 2000 && result < 2128)
            setNoteNumber (result - 2000);
    }
}

float MidiAutomatable::applyNoteIncrement(float val)
{
   float returnVal = val;
   if (incrAmount > 0.9999)
      returnVal = val > 0.5 ? 0.0 : 1.0;
   else {
      double upperLimit = (incrMax + 0.00001);
      returnVal += incrAmount;
      if (returnVal > upperLimit)
      {
         if (bidirectional)
         {
            incrAmount = -incrAmount;
            returnVal = incrMax + incrAmount;
         }
         else
            returnVal = 0.0;
      }
      else if (returnVal < -0.00001)
      {
         if (bidirectional)
         {
            incrAmount = -incrAmount;
            returnVal = 0.0 + incrAmount;
         }
         else
            returnVal = incrMax;
      }
   }
   return returnVal;
}

//==============================================================================
MidiAutomatorManager::MidiAutomatorManager ()
    : activeLearner (0)
{
    for (int i = 0; i < 128; i++)
        controllers.add (new VoidArray);
    for (int i = 0; i < 128; i++)
        notes.add (new VoidArray);
    for (int i = 0; i < 128; i++)
        noteOffs.add (new VoidArray);
}

MidiAutomatorManager::~MidiAutomatorManager ()
{
    for (int i = 0; i < 128; i++)
        delete controllers.getUnchecked (i);
    for (int i = 0; i < 128; i++)
        delete notes.getUnchecked (i);
    for (int i = 0; i < 128; i++)
        delete noteOffs.getUnchecked (i);
}

//==============================================================================
void MidiAutomatorManager::registerMidiAutomatable (MidiAutomatable* object)
{
    object->setMidiAutomatorManager (this);

   removeMidiAutomatable(object);
        
    if (object->getControllerNumber () != -1)
    {
        VoidArray* array = controllers.getUnchecked (object->getControllerNumber ());
        
        array->add (object);
    }
    else if (object->getNoteNumber () != -1)
    {
      if (object->isNoteOn())
      {
        VoidArray* array = notes.getUnchecked (object->getNoteNumber ());      
        array->add (object);
}
      else
      {
        VoidArray* array = noteOffs.getUnchecked (object->getNoteNumber ());      
        array->add (object);
      }
    }
}

//==============================================================================
void MidiAutomatorManager::removeMidiAutomatable (MidiAutomatable* object)
{
    if (activeLearner == object)
        activeLearner = 0;

    for (int i = 0; i < 128; i++)
    {
        VoidArray* array = controllers.getUnchecked (i);
        VoidArray* arrayNote = notes.getUnchecked (i);
        VoidArray* arrayNoteOff = noteOffs.getUnchecked (i);
        
        if (array->contains (object))
        {
            array->removeValue (object);
        }
        if (arrayNote->contains (object))
        {
            arrayNote->removeValue (object);
    }
        if (arrayNoteOff->contains (object))
        {
            arrayNoteOff->removeValue (object);
}
    }
}

//==============================================================================
void MidiAutomatorManager::clearMidiAutomatableFromCC (const int ccNumber)
{
    jassert (ccNumber >= 0 && ccNumber < 128)

    VoidArray* array = controllers.getUnchecked (ccNumber);
    
    array->clear();
}
    
void MidiAutomatorManager::clearMidiAutomatableFromNote (const int note)
{
    jassert (ccNumber >= 0 && note < 128)

    VoidArray* array = notes.getUnchecked (note);
    
    array->clear();
}
void MidiAutomatorManager::clearMidiAutomatableFromNoteOff (const int note)
{
    jassert (ccNumber >= 0 && note < 128)

    VoidArray* array = noteOffs.getUnchecked (note);
    
    array->clear();
}
    
//==============================================================================
void MidiAutomatorManager::setActiveLearner (MidiAutomatable* object)
{
    activeLearner = object;
}

//==============================================================================
bool MidiAutomatorManager::handleMidiMessage (const MidiMessage& message)
{
    bool messageWasHandled = false;

    if (message.isController ())
    {
        if (activeLearner != 0)
        {
            activeLearner->setControllerNumber (message.getControllerNumber ());
            activeLearner = 0;
        }
        else
        {
            VoidArray* array = controllers.getUnchecked (message.getControllerNumber ());
            
            for (int i = 0; i < array->size (); i++)
            {
                MidiAutomatable* learnObject = (MidiAutomatable*) array->getUnchecked (i);
                
                messageWasHandled |= learnObject->handleMidiMessage (message);
            }
        }
    }
    else if (message.isNoteOnOrOff())
    {
        if (activeLearner != 0)
        {
            activeLearner->setNoteNumber (message.getNoteNumber ());
            activeLearner = 0;
        }
        else if (message.isNoteOn())
        {
            VoidArray* array = notes.getUnchecked (message.getNoteNumber ());
    
            for (int i = 0; i < array->size (); i++)
            {
                MidiAutomatable* learnObject = (MidiAutomatable*) array->getUnchecked (i);
                
                messageWasHandled |= learnObject->handleMidiMessage (message);
            }
        }
        else 
        {
            VoidArray* array = noteOffs.getUnchecked (message.getNoteNumber ());
            
            for (int i = 0; i < array->size (); i++)
            {
                MidiAutomatable* learnObject = (MidiAutomatable*) array->getUnchecked (i);
                
                messageWasHandled |= learnObject->handleMidiMessage (message);
            }
        }
    
    }
    
    return messageWasHandled;
}

//==============================================================================
bool MidiAutomatorManager::handleMidiMessageBuffer (MidiBuffer& buffer)
{
    int samplePosition;
    MidiMessage message (0xf4);
    MidiBuffer::Iterator it (buffer);

    bool messageWasHandled = false;

    while (it.getNextEvent (message, samplePosition))
    {
        messageWasHandled |= handleMidiMessage (message);
    }
    
    return messageWasHandled;
}

END_JUCE_NAMESPACE

