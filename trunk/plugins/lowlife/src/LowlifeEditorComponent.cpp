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
   myFilter(filter),
   mySlot(slot)
{
   jassert(filter);
   addAndMakeVisible(sampleFile = new FilenameComponent("sample", filter->getZoneslotSample(slot), false, false, false, "*.wav", String::empty, "drop it here niggas"));
   sampleFile->addListener(this);

   addAndMakeVisible(faderSlider = new Slider(T("fader")));
   faderSlider->addListener (this);
   faderSlider->setRange (DemoJuceFilter::MinFader, DemoJuceFilter::MaxFader, 1.);
   faderSlider->setTooltip (T("attenuates this slot"));

   addAndMakeVisible(keyMinSlider = new Slider(T("keyMin")));
   keyMinSlider->addListener (this);
   keyMinSlider->setRange (DemoJuceFilter::MinKey, DemoJuceFilter::MaxKey, 1.);
   keyMinSlider->setTooltip (T("lowest possible key"));

   addAndMakeVisible(keyMaxSlider = new Slider(T("keyMax")));
   keyMaxSlider->addListener (this);
   keyMaxSlider->setRange (DemoJuceFilter::MinKey, DemoJuceFilter::MaxKey, 1.);
   keyMaxSlider->setTooltip (T("highest key"));

   addAndMakeVisible(tuneSlider = new Slider(T("tuning")));
   tuneSlider->addListener (this);
   tuneSlider->setRange (DemoJuceFilter::MinTune, DemoJuceFilter::MaxTune, 1.);
   tuneSlider->setTooltip (T("tuning for the hecker"));
}

LowlifeSlotEditComponent::~LowlifeSlotEditComponent()
{
    deleteAllChildren();
}

void LowlifeSlotEditComponent::resized()
{
   int h = getHeight();
   int w = getWidth();
   int walf = w/2;
   h = h / 4.0;
   int curh = 0;

   sampleFile->setBounds(0, curh, w, h);
   
   keyMinSlider->setBounds(0, curh+=h, walf, h);
   keyMaxSlider->setBounds(walf, curh, walf, h);

   tuneSlider->setBounds(0, curh+=h, w, h);
   faderSlider->setBounds(0, curh+=h, w, h);
}

void LowlifeSlotEditComponent::updateParametersFromFilter()
{
   if (myFilter)
   {
      sampleFile->setCurrentFile(myFilter->getZoneslotSample(mySlot), true, false);
      //polyModeCombo->setSelectedId(myFilter->getZoneslotPolyMode(mySlot) + 1, false);
      faderSlider->setValue(myFilter->getZoneslotFader(mySlot), false);
      tuneSlider->setValue(myFilter->getZoneslotTuneFactor(mySlot), false);
      keyMinSlider->setValue(myFilter->getZoneslotKeyMin(mySlot), false);
      keyMaxSlider->setValue(myFilter->getZoneslotKeyMax(mySlot), false);
   }
}

void LowlifeSlotEditComponent::sliderValueChanged(Slider* sl)
{
   if (myFilter)
   {
      if (sl == faderSlider) 
         myFilter->setZoneslotFader(mySlot, sl->getValue());
      if (sl == tuneSlider) 
         myFilter->setZoneslotTuneFactor(mySlot, sl->getValue());
      if (sl == keyMinSlider) 
         myFilter->setZoneslotKeyMin(mySlot, sl->getValue());
      if (sl == keyMaxSlider) 
         myFilter->setZoneslotKeyMax(mySlot, sl->getValue());
   }
}

void LowlifeSlotEditComponent::filenameComponentChanged(FilenameComponent* filec)
{
   if (myFilter)
   {
      if (filec == sampleFile) 
         myFilter->setZoneslotSample(mySlot, filec->getCurrentFile());
   }
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
   resizeLimits.setSizeLimits (320, 250, 1000, 800);

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

   setSize(200, 80);
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
//    DemoJuceFilter* const filter = getFilter();
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
