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

PopupMenu MidiAutomatable::generateMidiPopupMenu()
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

   return menu;
}

bool MidiAutomatable::processMidiPopupMenu(int result)
{
   bool weHandledIt = false;
    switch (result)
    {
    case 1:
        activateLearning ();
        weHandledIt = true;
        break;
    case 2:
        setControllerNumber (-1);
        setNoteNumber (-1);
        weHandledIt = true;
        break;

   default:
      if (result >= 1000 && result < 1128)
      {
         setControllerNumber (result - 1000);
         weHandledIt = true;
      }
      else if (result >= 2000 && result < 2128)
      {
         setNoteNumber (result - 2000);
         weHandledIt = true;
      }
   }

   return weHandledIt;
}

void MidiAutomatable::handleMidiPopupMenu(const MouseEvent& e)
{
    PopupMenu menu = generateMidiPopupMenu();
    
    int result = menu.showAt (e.getScreenX(), e.getScreenY());
    
    processMidiPopupMenu(result);
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
    jassert (note >= 0 && note < 128)

    VoidArray* array = notes.getUnchecked (note);
    
    array->clear();
}
void MidiAutomatorManager::clearMidiAutomatableFromNoteOff (const int note)
{
    jassert (note >= 0 && note < 128)

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

