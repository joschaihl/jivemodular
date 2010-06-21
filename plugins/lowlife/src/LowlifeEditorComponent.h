/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#ifndef LOWLIFEPLUGINEDITOR_H
#define LOWLIFEPLUGINEDITOR_H

#include "DemoJuceFilter.h"

// a component for editing a single slot
class LowlifeSlotEditComponent : public Component, public SliderListener, public FilenameComponentListener, public ButtonListener
{
public:
   LowlifeSlotEditComponent(DemoJuceFilter* filter, int slot);
   ~LowlifeSlotEditComponent();
   
   void updateParametersFromFilter();
   void sliderValueChanged (Slider* sl);
   void filenameComponentChanged(FilenameComponent* filec);
   void buttonClicked(Button* button);
   void resized();

FilenameComponent* sampleFile;
ToggleButton* syncButton;
Slider* syncTicksSlider;
Slider* faderSlider;
Slider* keySlider;
Slider* tuneSlider;

DemoJuceFilter* myFilter;
int mySlot;
};

//==============================================================================
/**
    This is the Component that our filter will use as its UI.

    One or more of these is created by the DemoJuceFilter::createEditor() method,
    and they will be deleted at some later time by the wrapper code.

    To demonstrate the correct way of connecting a filter to its UI, this
    class is a ChangeListener, and our demo filter is a ChangeBroadcaster. The
    editor component registers with the filter when it's created and deregisters
    when it's destroyed. When the filter's parameters are changed, it broadcasts
    a message and this editor responds by updating its display.
*/
class LowlifeEditorComponent   : public AudioProcessorEditor,
                              public ChangeListener,
                              public SliderListener,
                              public ComboBoxListener
{
public:
    /** Constructor.

        When created, this will register itself with the filter for changes. It's
        safe to assume that the filter won't be deleted before this object is.
    */
    LowlifeEditorComponent(DemoJuceFilter* const ownerFilter);

    /** Destructor. */
    ~LowlifeEditorComponent();

    //==============================================================================
    /** Our demo filter is a ChangeBroadcaster, and will call us back when one of
        its parameters changes.
    */
    void changeListenerCallback (void* source);

   void comboBoxChanged(ComboBox* comboBoxThatHasChanged);
   void sliderValueChanged (Slider*);

    //==============================================================================
    /** Standard Juce paint callback. */
    void paint (Graphics& g);

    /** Standard Juce resize callback. */
    void resized();


private:
    //==============================================================================
    ResizableCornerComponent* resizer;
    ComponentBoundsConstrainer resizeLimits;
   Array<LowlifeSlotEditComponent*> slotEditors; // these are child compopnents, so we don't need to own them directly here

   Slider* numSlotsSlider;
   ComboBox* polyModeCombo;

    void updateParametersFromFilter();

    // handy wrapper method to avoid having to cast the filter to a DemoJuceFilter
    // every time we need it..
    DemoJuceFilter* getFilter() const throw()       { return (DemoJuceFilter*) getAudioProcessor(); }
};

#endif
