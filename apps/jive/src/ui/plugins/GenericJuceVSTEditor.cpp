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

#include "StandardHeader.h"
#include "GenericJuceVSTEditor.h"
#include "ParamSlider.h"


//==============================================================================
class ProcessorParameterPropertyComp   : public PropertyComponent,
                                         public AudioProcessorListener,
                                         public AsyncUpdater
{
public:
    ProcessorParameterPropertyComp (const String& name,
                                    AudioProcessor* const owner_, AudioParameter* managedParam_,
                                    const int index_)
        : PropertyComponent (name),
          owner (owner_),
          managedParam(managedParam_),
          index (index_)
    {
        addAndMakeVisible (slider = new ParamSlider (owner_, managedParam_, index_));
        owner_->addListener (this);
    }

    ~ProcessorParameterPropertyComp()
    {
        owner->removeListener (this);
        deleteAllChildren();
    }

    void refresh()
    {
        slider->setValue (owner->getParameter (index), false);
    }

    void audioProcessorChanged (AudioProcessor*)  {}

    void audioProcessorParameterChanged (AudioProcessor*, int parameterIndex, float)
    {
        if (parameterIndex == index)
            triggerAsyncUpdate();
    }

    void handleAsyncUpdate()
    {
        refresh();
    }

   AudioParameter* getManagedParameter() {return managedParam;};

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    AudioProcessor* const owner;
    AudioParameter* managedParam;
    const int index;
    Slider* slider;

    ProcessorParameterPropertyComp (const ProcessorParameterPropertyComp&);
    const ProcessorParameterPropertyComp& operator= (const ProcessorParameterPropertyComp&);
};

//==============================================================================
GenericJuceVSTEditor::GenericJuceVSTEditor (BasePlugin* plugin, AudioProcessor* const owner_)
    : AudioProcessorEditor (owner_)
{
    setOpaque (true);

    addAndMakeVisible (panel = new PropertyPanel());

    Array <PropertyComponent*> params;

    const int numParams = owner_->getNumParameters();
    int totalHeight = 0;

    for (int i = 0; i < numParams; ++i)
    {
        String name (owner_->getParameterName (i));
        if (name.trim().isEmpty())
            name = "Unnamed";
            
         AudioParameter* paramToMg = plugin->getParameterObject(i);
            
        ProcessorParameterPropertyComp* const pc = new ProcessorParameterPropertyComp (name, owner_, paramToMg, i);
        params.add (pc);
        totalHeight += pc->getPreferredHeight();
    }

    panel->addProperties (params);

    setSize (400, jlimit (25, 400, totalHeight));
}

AudioParameter* GenericJuceVSTEditor::getParamAt(int x, int y)
{
   AudioParameter* param = 0;
   Component* comp = getComponentAt(x, y);
   ProcessorParameterPropertyComp* propcomp = dynamic_cast<ProcessorParameterPropertyComp*>(comp);
   if (propcomp)
   {
      param = propcomp->getManagedParameter();
   }

   return param;
}

GenericJuceVSTEditor::~GenericJuceVSTEditor()
{
    deleteAllChildren();
}

void GenericJuceVSTEditor::paint (Graphics& g)
{
    g.fillAll (Colours::white);
}

void GenericJuceVSTEditor::resized()
{
    panel->setSize (getWidth(), getHeight());
}


