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

#include "includes.h"
#include "LowlifeEditorComponent.h"

LowlifeSlotEditComponent::LowlifeSlotEditComponent(DemoJuceFilter* filter, int slot)
:
   clipList(0),
   syncButton(0),
   syncTicksSlider(0),
   faderSlider(0),
   keySlider(0),
   tuneSlider(0),
   myFilter(filter),
   mySlot(slot)
{
   jassert(filter);

   addAndMakeVisible(clipList = new ClipListComponent("Sample clip list", "*.wav"));
   clipList->addListener(this);

   addAndMakeVisible(faderSlider = new Slider(T("fader")));
   faderSlider->addListener (this);
   faderSlider->setRange (DemoJuceFilter::MinFader, DemoJuceFilter::MaxFader, 1.);
   faderSlider->setTooltip (T("attenuates this slot"));

   addAndMakeVisible(syncButton = new ToggleButton(T("Tempo Sync")));
   syncButton->addButtonListener (this);

   addAndMakeVisible(syncTicksSlider = new Slider(T("Sync ticks")));
   syncTicksSlider->setSliderStyle(Slider::IncDecButtons);
   syncTicksSlider->addListener (this);
   syncTicksSlider->setRange (0.5, 1024, 0.5); // i.e. a max of 1024 beats in a stem/sample, and shortest sample 16th note (half a beat)
   syncTicksSlider->setTooltip (T("Number of beats in sample"));

   addAndMakeVisible(keySlider = new Slider(T("key")));
   keySlider->setSliderStyle(Slider::ThreeValueHorizontal);
   keySlider->setPopupDisplayEnabled(true, this);
   keySlider->addListener (this);
   keySlider->setRange (DemoJuceFilter::MinKey, DemoJuceFilter::MaxKey, 1.);
   keySlider->setTooltip (T("midi key min/centre/max"));

   addAndMakeVisible(tuneSlider = new Slider(T("tuning")));
   tuneSlider->addListener (this);
   tuneSlider->setRange (DemoJuceFilter::MinTune, DemoJuceFilter::MaxTune, 1.);
   tuneSlider->setTooltip (T("tuning for the hecker"));
}

LowlifeSlotEditComponent::~LowlifeSlotEditComponent()
{
   clipList->removeListener(this);
    deleteAllChildren();
}

void LowlifeSlotEditComponent::resized()
{
   float numRowsOfThings = 5.0;

   int h = getHeight();
   int w = getWidth();
   int walf = w/2;
   h = h / numRowsOfThings;
   int curh = 0;

   clipList->setBounds(0, curh, w, h);

   curh+=h;

   syncButton->setBounds(0, curh, walf, h);
   syncTicksSlider->setBounds(walf, curh, walf, h);
      
   keySlider->setBounds(0, curh+=h, w, h);

   tuneSlider->setBounds(0, curh+=h, w, h);
   faderSlider->setBounds(0, curh+=h, w, h);
}

void LowlifeSlotEditComponent::updateParametersFromFilter()
{
   if (myFilter)
   {
      if (clipList)
      {
         for (int i=0; i<myFilter->getZoneslotNumClips(mySlot); i++)
            clipList->setClipFile(i, myFilter->getZoneslotClipFile(mySlot, i));
         clipList->setCurrentClipIndex(myFilter->getZoneslotCurrentClip(mySlot), false); // don't notify
      }
      
      syncButton->setToggleState(myFilter->getRawParam(mySlot, DemoJuceFilter::BPMSync), false);
      syncTicksSlider->setValue(myFilter->getRawParam(mySlot, DemoJuceFilter::SyncTicks), false);
      keySlider->setMinValue(myFilter->getRawParam(mySlot, DemoJuceFilter::KeyLow), false);
      keySlider->setValue(myFilter->getRawParam(mySlot, DemoJuceFilter::KeyCentre), false);
      keySlider->setMaxValue(myFilter->getRawParam(mySlot, DemoJuceFilter::KeyHigh), false);
      tuneSlider->setValue(myFilter->getRawParam(mySlot, DemoJuceFilter::Tune), false);
      faderSlider->setValue(myFilter->getRawParam(mySlot, DemoJuceFilter::Fader), false);
   }
}

void LowlifeSlotEditComponent::clipListChanged(ClipListComponent* ctrlThatHasChanged, StringArray newClipList, std::vector<int> changeInfo)
{
   myFilter->clearZoneslotClips(mySlot);
   for (int i=0; i<newClipList.size(); i++)
   {
      String clipFile;
      clipFile = newClipList[i];
      myFilter->setZoneslotClipFile(mySlot, i, clipFile);
   }
   updateParametersFromFilter(); // get the new clips in the combo
   myFilter->setZoneslotCurrentClip(mySlot, ctrlThatHasChanged->getCurrentClipIndex()); 
}

void LowlifeSlotEditComponent::currentClipChanged(ClipListComponent* ctrlThatHasChanged)
{
   myFilter->setZoneslotCurrentClip(mySlot, ctrlThatHasChanged->getCurrentClipIndex()); 
}

void LowlifeSlotEditComponent::clipFilesDropped(ClipListComponent* ctrlThatHasChanged, const StringArray& files)
{
   for (int i=0; i<files.size(); i++)
   {
      if (files[i].isNotEmpty())
         myFilter->setZoneslotClipFile(mySlot, i, files[i]);
   }
   updateParametersFromFilter(); // get the new clips in the combo
   myFilter->setZoneslotCurrentClip(mySlot, ctrlThatHasChanged->getCurrentClipIndex()); 
}

void LowlifeSlotEditComponent::sliderValueChanged(Slider* sl)
{
   if (myFilter)
   {
      if (sl == syncTicksSlider)
         myFilter->setRawParam(mySlot, DemoJuceFilter::SyncTicks, sl->getValue());
      
      else if (sl == keySlider) 
      {
         myFilter->setRawParam(mySlot, DemoJuceFilter::KeyLow, sl->getMinValue());
         myFilter->setRawParam(mySlot, DemoJuceFilter::KeyCentre, sl->getValue());
         myFilter->setRawParam(mySlot, DemoJuceFilter::KeyHigh, sl->getMaxValue());
      }

      else if (sl == tuneSlider) 
         myFilter->setRawParam(mySlot, DemoJuceFilter::Tune, sl->getValue());

      else if (sl == faderSlider) 
         myFilter->setRawParam(mySlot, DemoJuceFilter::Fader, sl->getValue());
   }
}

void LowlifeSlotEditComponent::buttonClicked(Button* button)
{
   if (button == syncButton)
      myFilter->setRawParam(mySlot, DemoJuceFilter::BPMSync, syncButton->getToggleState());
}

//==============================================================================
LowlifeEditorComponent::LowlifeEditorComponent (DemoJuceFilter* const ownerFilter)
: 
   AudioProcessorEditor (ownerFilter),
   resizer(0),
   numSlotsSlider(0),
   polyModeCombo(0)
{
   updateParametersFromFilter();

    // add the triangular resizer component for the bottom-right of the UI
    addAndMakeVisible (resizer = new ResizableCornerComponent (this, &resizeLimits));
   resizeLimits.setSizeLimits (440, 250, 1600, 1200);

   addAndMakeVisible(numSlotsSlider = new Slider("numSlots"));
   if (numSlotsSlider)
   {
      numSlotsSlider->setSliderStyle(Slider::IncDecButtons);
      if (ownerFilter)
         numSlotsSlider->setRange(0, ownerFilter->getMaxZoneslots(), 1);
      numSlotsSlider->addListener(this);
   }
   
   addAndMakeVisible(polyModeCombo = new ComboBox(T("polymode")));
   if (polyModeCombo)
   {
      polyModeCombo->addListener (this);
      polyModeCombo->addItem("Monophonic", DemoJuceFilter::MinPolyMode + 1);
      polyModeCombo->addItem("Legato", DemoJuceFilter::MinPolyMode + 2);
      polyModeCombo->addItem("Polyphonic", DemoJuceFilter::MaxPolyMode + 1);
   }

    // register ourselves with the filter - it will use its ChangeBroadcaster base
    // class to tell us when something has changed, and this will call our changeListenerCallback()
    // method.
    ownerFilter->addChangeListener (this);

   setSize(resizeLimits.getMinimumWidth(), resizeLimits.getMinimumHeight());
}

LowlifeEditorComponent::~LowlifeEditorComponent()
{
    getFilter()->removeChangeListener (this);

    deleteAllChildren();
}

//==============================================================================
void LowlifeEditorComponent::paint (Graphics& g)
{
    // just clear the window
    g.fillAll (Colour::greyLevel (0.9f));
}

void LowlifeEditorComponent::resized()
{
   LowlifeSlotEditComponent* tmp;
   if (slotEditors.size() > 0)
   {
      int hdv = (getHeight() - 16.0) / slotEditors.size();
      for (int i=0; i<slotEditors.size(); i++)
      {
         tmp = slotEditors.getUnchecked(i);
         if (tmp)
            tmp->setBounds(0, hdv * i, getWidth(), hdv);
      }
   }
   if (resizer)
      resizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);
   if (numSlotsSlider)
      numSlotsSlider->setBounds (0, getHeight() - 16, 80, 16);
   if (polyModeCombo)
      polyModeCombo->setBounds(80, getHeight() - 16, getWidth() - 80 - 16, 16);
}

//==============================================================================
void LowlifeEditorComponent::changeListenerCallback (void* source)
{
    // this is the filter telling us that it's changed, so we'll update our
    // display of the time, midi message, etc.
    updateParametersFromFilter();
}

void LowlifeEditorComponent::sliderValueChanged (Slider* slider)
{
DemoJuceFilter* const filter = getFilter();
if (filter && slider == numSlotsSlider)
    filter->setNumZoneslots(slider->getValue());
}

void LowlifeEditorComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
DemoJuceFilter* const myFilter = getFilter();
   if (myFilter)
   {
      if (comboBoxThatHasChanged == polyModeCombo) 
         myFilter->setPolyMode(comboBoxThatHasChanged->getSelectedId() - 1);
   }
}


//==============================================================================
void LowlifeEditorComponent::updateParametersFromFilter()
{
    DemoJuceFilter* const filter = getFilter();

    // we use this lock to make sure the processBlock() method isn't writing to the
    // lastMidiMessage variable while we're trying to read it, but be extra-careful to
    // only hold the lock for a minimum amount of time..
    filter->getCallbackLock().enter();

   // we don't need anything that's in processData..

    // ..release the lock ASAP
    filter->getCallbackLock().exit();

   int slots = filter->getNumZoneslots();
   LowlifeSlotEditComponent* tmp;
   if (slots != slotEditors.size())
   {
      for (int i=0; i<slotEditors.size(); i++)
      {
         tmp = slotEditors.getUnchecked(i);
         removeChildComponent(tmp);
         delete tmp;
      }
      slotEditors.clear();
      for (int i=0; i<slots; i++)
      {
         addAndMakeVisible(tmp = new LowlifeSlotEditComponent(filter, i));
         
         slotEditors.add(tmp);
      }
      
      resized();
   }

      for (int i=0; i<slots; i++)
      {
         tmp = slotEditors.getUnchecked(i);
         if (tmp)
            tmp->updateParametersFromFilter();
      }
      
      
   if (numSlotsSlider)
      numSlotsSlider->setValue(slots, false);
   if (polyModeCombo)
      polyModeCombo->setSelectedId(filter->getPolyMode() + 1, false);
}
