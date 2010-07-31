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
void MidiBinding::setNote(int m)
{
   if (mode == Controller)
      mode = NoteOn;
   triggerVal = m;
}

void MidiBinding::setCC(int m)
{
   mode = Controller;
   triggerVal = m;
}

void MidiBinding::setMode(int noteOnMode)
{
   if (noteOnMode < NoteOff || noteOnMode > Controller) 
      noteOnMode = NoteOn; 

   mode = static_cast<NoteBindingMode>(noteOnMode); 
}

void MidiBinding::setStepMode(BindingStepMode bidirectional_) 
{
   bidirectional = bidirectional_; 
   if (bidirectional == Increase && incrAmount < 0)
      incrAmount = -incrAmount;
   else if (bidirectional == Decrease && incrAmount > 0)
      incrAmount = -incrAmount;
};

float MidiBinding::applyCC(float val)
{
   return (maximum - minimum) * val + minimum;
}

float MidiBinding::applyNoteIncrement(float val)
{
   double range = (maximum-minimum);
   double upperLimit = (minimum + (incrMax * range));
   double lowerLimit = (minimum + (incrMin * range));
   double actualIncr = incrAmount * range;
   float returnVal = val;
   if (incrAmount > 0.9999) // toggle
      returnVal = val > 0.5 ? lowerLimit : upperLimit;
   else if (bidirectional == Increase || bidirectional == Decrease || bidirectional == Bidirectional)
   {
   // rejigged these to be relative to user-set max & mins 
      returnVal += actualIncr;
      if (returnVal > upperLimit + 0.00001)
      {
         if (bidirectional == Bidirectional)
         {
            incrAmount = -incrAmount;
            returnVal = upperLimit + actualIncr;
         }
         else
            returnVal = lowerLimit;
      }
      else if (returnVal < lowerLimit - 0.00001)
      {
         if (bidirectional == Bidirectional)
         {
            incrAmount = -incrAmount;
            returnVal = lowerLimit + actualIncr;
         }
         else
            returnVal = upperLimit;
      }
   }
   else if (bidirectional == SetToValue)
      returnVal = upperLimit;
   return returnVal;
}

String MidiBinding::getDescription()
{
   String description;
   if (getCC() != -1)
      description = String("Assigned to CC ") + String (getCC());
   else if (getNote() != -1)
      description = String("Assigned to Note ") + String (getNote());
   else
      description = String("Not assigned");
   return description;
}

//==============================================================================
MidiAutomatable::MidiAutomatable()
:
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

void MidiAutomatable::RegisterBinding(int bindingNum)
{
   if (midiAutomatorManager)
      midiAutomatorManager->registerMidiAutomatable (this, bindingNum);    
}

bool MidiAutomatable::isBindingNumValid(int bindingNum) const
{
   return (bindingNum >= 0 && bindingNum < bindings.size());
}

MidiBinding* MidiAutomatable::getBinding(int bindingNum) const 
{
   if (isBindingNumValid(bindingNum))
      return bindings[bindingNum];
   else 
      return NULL;
}

void MidiAutomatable::removeBinding(int bindingNum)
{
   if (isBindingNumValid(bindingNum))
      bindings.remove(bindingNum);
}

void MidiAutomatable::setControllerNumber (const int control, int bindingNum)
{
   MidiBinding* b = getBinding(bindingNum);
   if (b)
   {
      b->setCC(control);
      RegisterBinding(bindingNum);
   }
}

void MidiAutomatable::setNoteNumber (const int note, int bindingNum)
{
   MidiBinding* b = getBinding(bindingNum);
   if (b)
   {
      b->setNote(note);
      RegisterBinding(bindingNum);
   }
}

int MidiAutomatable::addControllerNumber (const int control)
{
   int newBinding = bindings.size();
   bindings.add(new MidiBinding);
   setControllerNumber(control, newBinding);
   RegisterBinding(newBinding);
   return newBinding;
}

int MidiAutomatable::addNoteNumber (const int note)
{
   int newBinding = bindings.size();
   bindings.add(new MidiBinding);
   setNoteNumber(note, newBinding);
   RegisterBinding(newBinding);
   return newBinding;
}

void MidiAutomatable::setNoteMode(int noteOnMode, int bindingNum) 
{ 
      MidiBinding* b = getBinding(bindingNum);
      if (b)
      {
         b->setMode(noteOnMode);
   RegisterBinding(bindingNum);
   }
};


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

// to do summarise multiple bindings
//   menu.addItem (-1, getBinding(0).getDescription(), false);
//   menu.addSeparator ();

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
        setControllerNumber (-1);
        setNoteNumber (-1);
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

//==============================================================================
MidiAutomatorManager::MidiAutomatorManager ()
    : activeLearner (0)
{
    for (int i = 0; i < 128; i++)
        controllers.add (new TriggerValBindingMap);
//    for (int i = 0; i < 128; i++)
//        notes.add (new VoidArray);
    for (int i = 0; i < 128; i++)
        notes.add (new TriggerValBindingMap);
    for (int i = 0; i < 128; i++)
        noteOffs.add (new TriggerValBindingMap);
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
void MidiAutomatorManager::registerMidiAutomatable (MidiAutomatable* object, int bindingNum)
{
   object->setMidiAutomatorManager (this);

   removeMidiAutomatable(object, bindingNum);
   
   MidiBinding* bindingOptsP = object->getBinding(bindingNum);
   if (bindingOptsP)
   {   
      MidiBinding& bindingOpts = *bindingOptsP;
      if (bindingOpts.getCC () != -1)
      {
         TriggerValBindingMap* array = controllers.getUnchecked (bindingOpts.getCC ());
//         array->add (object);
         array->insert(TriggerValBinding(object, bindingNum));
      }
      else if (bindingOpts.getNote () != -1)
      {
         if (bindingOpts.getMode() == NoteHeld || bindingOpts.getMode() == NoteOn)
         {
   //         VoidArray* array = notes.getUnchecked (bindingOpts.getNoteNumber ());      
   //         array->add (object);
            TriggerValBindingMap* array = notes.getUnchecked (bindingOpts.getNote ());      
            array->insert(TriggerValBinding(object, bindingNum));
         }
         if (bindingOpts.getMode() == NoteHeld || bindingOpts.getMode() == NoteOff)
         {
            TriggerValBindingMap* array = noteOffs.getUnchecked (bindingOpts.getNote());      
//            array->add (object);
            array->insert(TriggerValBinding(object, bindingNum));
         }
      }
   }
}

//==============================================================================
void MidiAutomatorManager::removeMidiAutomatable (MidiAutomatable* object, int bindingNum)
{
   if (activeLearner == object)
      activeLearner = 0;

   for (int i = 0; i < 128; i++)
   {
      TriggerValBindingMap* array;
      array = controllers.getUnchecked (i);
      for (TriggerValBindingMap::iterator it=array->lower_bound(object); it != array->upper_bound(object); it++)
      {
         if (bindingNum == -1 || bindingNum == it->second)
            array->erase(it);
      }
      array = notes.getUnchecked (i);
      for (TriggerValBindingMap::iterator it=array->lower_bound(object); it != array->upper_bound(object); it++)
      {
         if (bindingNum == -1 || bindingNum == it->second)
         array->erase(it);
      }
      array = noteOffs.getUnchecked (i);
      for (TriggerValBindingMap::iterator it=array->lower_bound(object); it != array->upper_bound(object); it++)
      {
         if (bindingNum == -1 || bindingNum == it->second)
         array->erase(it);
      }
   }
}

//==============================================================================
void MidiAutomatorManager::clearMidiAutomatableFromCC (const int ccNumber)
{
    jassert (ccNumber >= 0 && ccNumber < 128)

//    VoidArray* array = controllers.getUnchecked (ccNumber);
   TriggerValBindingMap* array = controllers.getUnchecked (ccNumber);
    array->clear();
}
    
void MidiAutomatorManager::clearMidiAutomatableFromNote (const int note)
{
    jassert (note >= 0 && note < 128)

//    VoidArray* array = notes.getUnchecked (note);
//    array->clear();
   TriggerValBindingMap* array = notes.getUnchecked (note);
   array->clear();

}
void MidiAutomatorManager::clearMidiAutomatableFromNoteOff (const int note)
{
    jassert (note >= 0 && note < 128)

    TriggerValBindingMap* array = noteOffs.getUnchecked (note);
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
            activeLearner->addControllerNumber (message.getControllerNumber ());
            activeLearner = 0;
        }
        else
        {
            TriggerValBindingMap* mappy = controllers.getUnchecked(message.getNoteNumber ());
            for (TriggerValBindingMap::iterator it = mappy->begin(); it != mappy->end(); it++)
               messageWasHandled |= it->first->handleMidiMessage(message, it->second);
        }
    }
    else if (message.isNoteOnOrOff())
    {
        if (activeLearner != 0)
        {
            activeLearner->addNoteNumber (message.getNoteNumber());
            activeLearner = 0;
        }
        else if (message.isNoteOn())
        {
            TriggerValBindingMap* mappy = notes.getUnchecked(message.getNoteNumber ());
            for (TriggerValBindingMap::iterator it = mappy->begin(); it != mappy->end(); it++)
                messageWasHandled |= it->first->handleMidiMessage(message, it->second);
        }
        else 
        {
            TriggerValBindingMap* mappy = noteOffs.getUnchecked(message.getNoteNumber ());
            for (TriggerValBindingMap::iterator it = mappy->begin(); it != mappy->end(); it++)
               messageWasHandled |= it->first->handleMidiMessage(message, it->second);
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

