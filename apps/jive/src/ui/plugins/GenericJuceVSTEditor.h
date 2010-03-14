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

#ifndef JOST_GENERICJUCEVSTEDITOR_H
#define JOST_GENERICJUCEVSTEDITOR_H

#include "src/core/juce_StandardHeader.h"
#include "src/audio/processors/juce_AudioProcessorEditor.h"
#include "BasePlugin.h"

//==============================================================================
/**
    A type of UI component that displays the parameters of an AudioProcessor as
    a simple list of sliders.

    This can be used for showing an editor for a processor that doesn't supply
    its own custom editor.

    @see AudioProcessor
*/
class JUCE_API  GenericJuceVSTEditor      : public AudioProcessorEditor
{
public:
    //==============================================================================
    GenericJuceVSTEditor (BasePlugin* plugin, AudioProcessor* const owner);
    ~GenericJuceVSTEditor();

    //==============================================================================
    void paint (Graphics& g);
    void resized();
   
   AudioParameter* getParamAt(int x, int y);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
    PropertyPanel* panel;

    GenericJuceVSTEditor (const GenericJuceVSTEditor&);
    const GenericJuceVSTEditor& operator= (const GenericJuceVSTEditor&);
};


#endif   // JOST_GENERICJUCEVSTEDITOR_H
