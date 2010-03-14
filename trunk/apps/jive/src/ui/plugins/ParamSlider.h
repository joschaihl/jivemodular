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

#ifndef JOST_PARAMSLIDER_H
#define JOST_PARAMSLIDER_H

#include "src/core/juce_StandardHeader.h"
#include "model/BasePlugin.h"


//==============================================================================
class ParamSlider  : public Slider, public AudioParameterListener
{
public:
   ParamSlider (AudioProcessor* const owner_, AudioParameter* managedParam_, const int index_)
      : Slider (String::empty),
        owner (owner_),
        managedParam(managedParam_),
        index (index_)
   {
      setRange (0.0, 1.0, 0.0);
      setSliderStyle (Slider::LinearBar);
      setTextBoxIsEditable (true);
      setScrollWheelEnabled (false);
      if (managedParam)
         managedParam->addListener(this);
   }

   ~ParamSlider()
   {
      if (managedParam)
         managedParam->removeListener(this);
   }

   void parameterChanged (AudioParameter* newParameter, const int index)
   {
      setValue (newParameter->getValueMapped (), false);
   }

   void attachedToParameter (AudioParameter* newParameter, const int index)
   {
   //        parameter = newParameter;
   }

   void detachedFromParameter (AudioParameter* newParameter, const int index)
   {
   //        if (newParameter == parameter)
   //            parameter = 0;
   }

   void valueChanged()
   {
      const float newVal = (float) getValue();

      if (owner->getParameter (index) != newVal)
          owner->setParameter (index, newVal);
   }

   const String getTextFromValue (double /*value*/)
   {
      return owner->getParameterText (index);
   }

   void mouseDown (const MouseEvent& e)
   {
      Slider::mouseDown(e);

      if (e.mods.isRightButtonDown())
         managedParam->handleMidiPopupMenu(e);
   }

//==============================================================================
juce_UseDebuggingNewOperator

private:
AudioProcessor* const owner;
AudioParameter* managedParam;

const int index;

ParamSlider (const ParamSlider&);
const ParamSlider& operator= (const ParamSlider&);
};


#endif   // JOST_PARAMSLIDER_H
