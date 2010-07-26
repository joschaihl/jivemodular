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

#ifndef __JUCETICE_MIDI_LEARN_HEADER__
#define __JUCETICE_MIDI_LEARN_HEADER__

#include "../../../containers/juce_VoidArray.h"
#include "../../../audio/midi/juce_MidiMessage.h"
#include "../../../audio/midi/juce_MidiBuffer.h"

#include "../../../gui/components/mouse/juce_MouseEvent.h"
#include "../../../gui/components/menus/juce_PopupMenu.h"


class MidiAutomatorManager;

enum NoteBindingMode
{
   NoteOff = 0, // bound to note-off only
   NoteOn = 1, // bound to note-on only
   NoteHeld = 2, // parameter is max when note is down, otherwise min
   Controller = 3 // the classic CC binding!
};

enum BindingStepMode
{
   Decrease = 1,
   Increase = 2,
   Bidirectional = 3,
   SetToValue = 4
};

// A lightweight ish class encapsulating a single binding.
// Split out from MidiAutomatable so can support multiple bindings to single parameter.
// Enhanced to support more interesting binding modes/options.
class MidiBinding 
{
public:
   MidiBinding()
   :
      mode(NoteOn),
      triggerVal(-1),
      maximum(1.0),
      minimum(0.0),
      invert(false),
      incrAmount(1.0),
      bidirectional(Increase),
      incrMax(1.0),
      incrMin(0.0)
   {
   };
   
   void setNote(int m);
   void setCC(int m);
   void setMode(int m);

   bool isNoteBinding() const { return mode != Controller; };

   int getNote() const { return isNoteBinding() ? triggerVal : -1; };
   int getCC() const { return !isNoteBinding() ? triggerVal : -1; };  
   int getTriggerValue() const { return triggerVal; };

   NoteBindingMode getMode() const { return mode; }; // note-specific for now
   float getIncrAmount() const { return incrAmount; };
   BindingStepMode getStepMode() const { return bidirectional; };

   float getMax() const { return maximum; };
   float getMin() const { return minimum; };

   float getIncrMax() const { return incrMax; };
   float getIncrMin() const { return incrMin; };
   
   void setTriggerValue(int tv) { triggerVal = tv; };

   void setIncrAmount(float incrAmount_) { incrAmount = incrAmount_; };
   void setStepMode(BindingStepMode bidirectional_);

   void setMax(float incrMax_) { maximum = incrMax_; };
   void setMin(float incrMax_) { minimum = incrMax_; };

   void setStepMax(float incrMax_) { incrMax = incrMax_; };
   void setStepMin(float incrMax_) { incrMin = incrMax_; };
   
   float applyNoteIncrement(float val);
   float applyCC(float val);

   String getDescription();

protected:
//   bool isNoteBinding; // true if note, false if CC
   NoteBindingMode mode;   
   int triggerVal; // note or CC depending on mode
      
   float maximum;
   float minimum;
   bool invert;

   // note-specific options (hmm.. at present)
   // will either generalise these options, use a union, or have subclass or 2?
   float incrAmount;
   BindingStepMode bidirectional;
   float incrMax;
   float incrMin;
};

//==============================================================================
/**
     A midi learning object
*/
class MidiAutomatable
{
public:

#if 0
    typedef FastDelegate1<int, float> MidiTransferDelegate;

    //==============================================================================
    /** TODO */
    enum MidiTransferFunction
    {
        Linear = 0,
        InvertedLinear
    };
#endif

    //==============================================================================
    /** Destructor */
    virtual ~MidiAutomatable();

    //==============================================================================
    /** It actually start midi learning */
    void activateLearning ();

   //==============================================================================
   MidiBinding& getBinding(int bindingNum=0) { return currentBinding; } // param because we will have lots..

   // these accessors are temporary - make no sense if this thing supports multiple bindings
   // will be removed when MidiBinding is factored out and tested ok etc..

   int getControllerNumber () const                  { return currentBinding.getCC() ; };
   int getNoteNumber () const                  { return currentBinding.getNote(); };

   void setControllerNumber (const int control);
   void setNoteNumber (const int note);
   void setNoteMode(int noteOnMode);
   
#if 0
    //==============================================================================
    /** Returns the actual controller number */
    MidiTransferFunction getTransferFunction () const { return transfer; }

    /** Set a new controller number */
    void setTransferFunction (MidiTransferFunction control);
#endif

    //==============================================================================
    /** This is used inside a mouseDown of a corresponding Component */
    void handleMidiPopupMenu(const MouseEvent& e);

    // Client code can use generateMidiPopupMenu and processMidiPopupMenu to add custom items to the menu.
    // Added items must have ids between 3-1000.
    PopupMenu generateMidiPopupMenu();
    bool processMidiPopupMenu(int menuSelection);

    //==============================================================================
    /** Handle a midi message coming in */
    virtual bool handleMidiMessage (const MidiMessage& message) = 0;

protected:

   friend class MidiAutomatorManager;
   
   MidiBinding currentBinding;

#if 0
    MidiTransferFunction transfer;
    MidiTransferDelegate function;
#endif

    MidiAutomatorManager* midiAutomatorManager;

    //==============================================================================
    /** Attach a new manager */
    void setMidiAutomatorManager (MidiAutomatorManager* newManager);
    
    //==============================================================================
    /** Constructor */
    MidiAutomatable ();
};


//==============================================================================
/**
     A midi learning object
*/
class MidiAutomatorManager
{
public:

    //==============================================================================
    /** Constructor */
    MidiAutomatorManager ();

    /** Destructor */
    ~MidiAutomatorManager();

    //==============================================================================
    /** Register a new object

        The function is important because it actually tells to our objects which
        is the actual midi learn manager where we listen to.
        
        It is the first thing to call in order to use the actual learning caps.
    */ 
    void registerMidiAutomatable (MidiAutomatable* object);

    /** Deregister an existing object

        The function is important because it actually tells to our objects which
        is the actual midi learn manager where we listen to.
    */ 
    void removeMidiAutomatable (MidiAutomatable* object);

    //==============================================================================
    /** Deregister all objects listening to a CC number */ 
    void clearMidiAutomatableFromCC (const int ccNumber);
    void clearMidiAutomatableFromNote (const int note);
    void clearMidiAutomatableFromNoteOff (const int note);

    //==============================================================================
    /** Handle a single midi message */
    bool handleMidiMessage (const MidiMessage& message);

    /** Convenience function that handle multiple midi messages */
    bool handleMidiMessageBuffer (MidiBuffer& buffer);

protected:

    friend class MidiAutomatable;

    //==============================================================================
    /** Tells which is the learning object */
    void setActiveLearner (MidiAutomatable* object);

    Array<VoidArray*> controllers;
    Array<VoidArray*> notes;
    Array<VoidArray*> noteOffs;
    MidiAutomatable* activeLearner;
};


#endif // __WIRECORE_MIDILEARN_HEADER__

