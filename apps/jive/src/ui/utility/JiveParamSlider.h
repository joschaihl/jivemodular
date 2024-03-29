
#ifndef JOST_PARAMSLIDER_H
#define JOST_PARAMSLIDER_H

#include "Config.h"
#include "model/BasePlugin.h"


//==============================================================================
class ParamSlider  : public Slider, public AudioParameterListener
{
public:
   ParamSlider (AudioProcessor* const owner_, AudioParameter* managedParam_, const int index_)
      : Slider (String(index)),  
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
   
   int getParameterIndex() { return index; };

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
      {
          owner->setParameter (index, newVal);
         updateText();
      }
   }

   const String getTextFromValue (double /*value*/)
   {
      return owner->getParameterText (index);
   }

   void mouseDown (const MouseEvent& e);


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
