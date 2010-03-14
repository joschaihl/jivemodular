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
#include "XSynthJuceFilter.h"
#include "XSynthEditorComponent.h"
#include <iostream>

//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DemoJuceFilter();
}

//==============================================================================
DemoJuceFilter::DemoJuceFilter()
:
    ptrPlug (0),
    ladspa (0),
    plugin (0),
    emptyBuffer (1,32),
    samplingRate (44100.0f),
	current_patch_num(0)
{
    gain = 1.0f;
    lastUIWidth = 400;
    lastUIHeight = 140;

    zeromem (&lastPosInfo, sizeof (lastPosInfo));
    lastPosInfo.timeSigNumerator = 4;
    lastPosInfo.timeSigDenominator = 4;
    lastPosInfo.bpm = 120;

	// init stuff based on plugin (as if it were generic)
	for (uint32 iPluginIndex = 0;; iPluginIndex++)
	{
		ptrPlug = dssi_descriptor (iPluginIndex);
		if (ptrPlug != NULL)
			break;
	}

	if (ptrPlug != 0)
	{
		ladspa = ptrPlug->LADSPA_Plugin;
		// version = ptrPlug->DSSI_API_Version;

		plugin = ladspa->instantiate (ladspa, (unsigned int) samplingRate);

		// count ports
		ins.clear ();
		outs.clear ();
		pars.clear ();

		for (uint i = 0; i < ladspa->PortCount; i++)
		{
			LADSPA_PortDescriptor pod = ladspa->PortDescriptors[i];

			if (pod & LADSPA_PORT_AUDIO)
			{
				if (pod & LADSPA_PORT_INPUT)         ins.add (i);
				else if (pod & LADSPA_PORT_OUTPUT)   outs.add (i);
			}
			else if (pod & LADSPA_PORT_CONTROL)
			{
				pars.add (i);
			}
		}
		
		// create ports
		numParams = pars.size ();
		params = new float [numParams];
		normalized = new float [numParams];
		memset (params, 0, numParams * sizeof (float));
		memset (normalized, 0, numParams * sizeof (float));

		// connect ports
		if (ladspa->connect_port)
		{
			for (int i = 0; i < pars.size (); i++)
            {
//               params[i] = rand() / static_cast<float>(RAND_MAX);
				ladspa->connect_port (plugin, pars [i], &normalized[i]);
            }
			for (int i = 0; i < ins.size (); i++)
				ladspa->connect_port (plugin, ins [i], emptyBuffer.getSampleData (0));
			for (int i = 0; i < outs.size (); i++)
				ladspa->connect_port (plugin, outs [i], emptyBuffer.getSampleData (0));
		}
		
	}
}

DemoJuceFilter::~DemoJuceFilter()
{
    if (ptrPlug && ladspa && ladspa->cleanup)
        ladspa->cleanup (plugin);
    ptrPlug = 0;

    if (params) delete[] params;
    if (normalized) delete[] normalized;
}

//==============================================================================
const String DemoJuceFilter::getName() const
{
    return "XSynth";
}

//==============================================================================
#include "gui_friendly_patches.c"

int DemoJuceFilter::getNumPrograms()                                        
{
	return friendly_patch_count;
}
int DemoJuceFilter::getCurrentProgram()                                     
{
	return current_patch_num; 
}
void DemoJuceFilter::setCurrentProgram (int index)
{

}

const String DemoJuceFilter::DemoJuceFilter::getProgramName (int index)                     
{
	return String(friendly_patches[current_patch_num].name); 
}

void DemoJuceFilter::changeProgramName (int index, const String& newName)   
{

}


//==============================================================================
int DemoJuceFilter::getNumParameters()
{
    return numParams;
}

float DemoJuceFilter::getParameter (int index)
{
//    return (index == 0) ? gain
//                        : 0.0f;
    return params [index];
}

void DemoJuceFilter::setParameter (int index, float value)
{
//    if (index == 0)
//    {
//        if (gain != value)
//        {
//            gain = value;
//
//            // if this is changing the gain, broadcast a change message which
//            // our editor will pick up.
//            sendChangeMessage (this);
//        }
//    }
    jassert (index >= 0 && index < pars.size ());

    const LADSPA_PortRangeHint* hint = & ladspa->PortRangeHints [pars [index]];

    float lower = hint->LowerBound *
                  (LADSPA_IS_HINT_SAMPLE_RATE (hint->HintDescriptor) ? samplingRate : 1.0f);
    float upper = hint->UpperBound *
                  (LADSPA_IS_HINT_SAMPLE_RATE (hint->HintDescriptor) ? samplingRate : 1.0f);

    // @TODO - Handle better lower/upper bound. this is ok for most cases
    //         but in some others it don't

    if (LADSPA_IS_HINT_TOGGLED (hint->HintDescriptor))
    {
        if (value < 0.5f)   normalized [index] = 0.0f;
        else                normalized [index] = 1.0f;
    }
    else if (LADSPA_IS_HINT_BOUNDED_BELOW (hint->HintDescriptor)
             && LADSPA_IS_HINT_BOUNDED_ABOVE (hint->HintDescriptor))
    {
        if (LADSPA_IS_HINT_LOGARITHMIC(hint->HintDescriptor) && (lower >= 1.0f && upper >= 1.0f))
            normalized [index] = expf(logf(lower) * value + logf(upper) * (1.0f - value));
        else
            normalized [index] = lower + (upper - lower) * value;
    }
    else if (LADSPA_IS_HINT_BOUNDED_BELOW (hint->HintDescriptor))
    {
        normalized [index] = value;
    }
    else if (LADSPA_IS_HINT_BOUNDED_ABOVE (hint->HintDescriptor))
    {
        normalized [index] = value * upper;
    }

    if (LADSPA_IS_HINT_INTEGER (hint->HintDescriptor))
        normalized [index] = (float) ((int) normalized [index]);

    params [index] = value;

}

const String DemoJuceFilter::getParameterName (int index)
{
//    if (index == 0)
//        return T("gain");
    if (ladspa && index < numParams)
    {
//        const LADSPA_PortRangeHint* hint = & ladspa->PortRangeHints [pars [index]];
//
//        if (LADSPA_IS_HINT_INTEGER (hint->HintDescriptor))
//            return String ((int) normalized [index]);
//        else
            return String (ladspa->PortNames[pars [index]]);
    }
    else
      return String::empty;
}

const String DemoJuceFilter::getParameterXMLName (int index)
{
   String name = getParameterName(index);
   name = name.replace(T(" "), T("_"));
   return name;
}

const String DemoJuceFilter::getParameterText (int index)
{
    if (ladspa)
    {
        const LADSPA_PortRangeHint* hint = & ladspa->PortRangeHints [pars [index]];

        if (LADSPA_IS_HINT_INTEGER (hint->HintDescriptor))
            return String ((int) normalized [index]);
        else
            return String (normalized [index], 4);
    }
    else
    {
        return String::empty;
    }
}

const String DemoJuceFilter::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String DemoJuceFilter::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool DemoJuceFilter::isInputChannelStereoPair (int index) const
{
    return false;
}

bool DemoJuceFilter::isOutputChannelStereoPair (int index) const
{
    return (index == 0);;
}

bool DemoJuceFilter::acceptsMidi() const
{
    return true;
}

bool DemoJuceFilter::producesMidi() const
{
    return false;
}

//==============================================================================
void DemoJuceFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    samplingRate = sampleRate;

    // do your pre-playback setup stuff here..
    keyboardState.reset();
    
    if (ladspa && ladspa->activate)
        ladspa->activate (plugin);    
}

void DemoJuceFilter::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    if (ladspa && ladspa->deactivate)
        ladspa->deactivate (plugin);
}

void DemoJuceFilter::processBlock (AudioSampleBuffer& buffer,
                                   MidiBuffer& midiMessages)
{
/*
    // for each of our input channels, we'll attenuate its level by the
    // amount that our volume parameter is set to.
    for (int channel = 0; channel < getNumInputChannels(); ++channel)
    {
        buffer.applyGain (channel, 0, buffer.getNumSamples(), gain);
        
        // mix in opposite ratio of noise (i.e. generator)
        float* sampleData = buffer.getSampleData(channel);
         for (int sample = 0; sample < buffer.getNumSamples(); sample++)
            sampleData[sample] = (rand() / static_cast<float>(RAND_MAX)) * (1.0 - gain) + sampleData[sample];
    }

    // in case we have more outputs than inputs, we'll clear any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }
*/
    // if any midi messages come in, use them to update the keyboard state object. This
    // object sends notification to the UI component about key up/down changes
    keyboardState.processNextMidiBuffer (midiMessages,
                                         0, buffer.getNumSamples(),
                                         true);

    // have a go at getting the current time from the host, and if it's changed, tell
    // our UI to update itself.
    AudioPlayHead::CurrentPositionInfo pos;

    if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition (pos))
    {
        if (memcmp (&pos, &lastPosInfo, sizeof (pos)) != 0)
        {
            lastPosInfo = pos;
            sendChangeMessage (this);
        }
    }
    else
    {
        zeromem (&lastPosInfo, sizeof (lastPosInfo));
        lastPosInfo.timeSigNumerator = 4;
        lastPosInfo.timeSigDenominator = 4;
        lastPosInfo.bpm = 120;
    }
    
    if (ptrPlug && ladspa)
    {
      int blockSize = buffer.getNumSamples();
      
        // convert midi messages internally
        midiManager.convertMidiMessages (midiMessages, blockSize);

        // connect ports
//        for (int i = 0; i < ins.size (); i++)
//            ladspa->connect_port (plugin, ins [i], inputBuffer->getSampleData (i));
        for (int i = 0; i < outs.size (); i++)
		{
            ladspa->connect_port (plugin, outs [i], buffer.getSampleData (i));
//			std::cerr << " connecting output " << i << std::endl;
		}

        if (ptrPlug->run_synth)
        {
            ptrPlug->run_synth (plugin,
                                blockSize,
                                midiManager.getMidiEvents (),
                                midiManager.getMidiEventsCount ());

			// now paste the data into the right channel
			// not generic, this assumes we are a mono plugin and we've claimed 2 output channels
			buffer.copyFrom(1, 0, buffer, 0, 0, blockSize);			

			return;
        }
        else if (ptrPlug->run_synth_adding)
        {
            buffer.clear ();
            ptrPlug->run_synth_adding (plugin,
                                       blockSize,
                                       midiManager.getMidiEvents (),
                                       midiManager.getMidiEventsCount ());

			// now paste the data into the right channel
			// not generic, this assumes we are a mono plugin and we've claimed 2 output channels
			buffer.copyFrom(1, 0, buffer, 0, 0, blockSize);			

			return;
        }

        // run ladspa if present as 
        if (ladspa->run)
        {
            ladspa->run (plugin, blockSize);
        }
        else if (ladspa->run_adding)
        {
            buffer.clear ();
            ladspa->run_adding (plugin, blockSize);
        }
        else
        {
            buffer.clear ();
        }
		
    }
    
}

//==============================================================================
AudioProcessorEditor* DemoJuceFilter::createEditor()
{
    return new DemoEditorComponent (this);
}

//==============================================================================
void DemoJuceFilter::getStateInformation (MemoryBlock& destData)
{
    // you can store your parameters as binary data if you want to or if you've got
    // a load of binary to put in there, but if you're not doing anything too heavy,
    // XML is a much cleaner way of doing it - here's an example of how to store your
    // params as XML..

    // create an outer XML element..
    XmlElement xmlState (T("XSynthPatch"));

    // add some attributes to it..
    xmlState.setAttribute (T("pluginVersion"), 1);
    xmlState.setAttribute (T("gainLevel"), gain);
    xmlState.setAttribute (T("uiWidth"), lastUIWidth);
    xmlState.setAttribute (T("uiHeight"), lastUIHeight);

    // you could also add as many child elements as you need to here..
   for (int i=0; i<getNumParameters(); i++)
   {
      xmlState.setAttribute (getParameterXMLName(i), getParameter(i));
      DBG(String("Saved parameter ") + getParameterXMLName(i) + String(" as ") + String(getParameter(i)));
   }


    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
}

void DemoJuceFilter::setStateInformation (const void* data, int sizeInBytes)
{
    // use this helper function to get the XML from this binary blob..
    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState != 0)
    {
        // check that it's the right type of xml..
        if (xmlState->hasTagName (T("XSynthPatch")))
        {
            // ok, now pull out our parameters..
            gain = (float) xmlState->getDoubleAttribute (T("gainLevel"), gain);

            lastUIWidth = xmlState->getIntAttribute (T("uiWidth"), lastUIWidth);
            lastUIHeight = xmlState->getIntAttribute (T("uiHeight"), lastUIHeight);

         for (int i=0; i<getNumParameters(); i++)
         {
            DBG(String("Loading parameter ") + getParameterXMLName(i) + String(" to ") + String(xmlState->getDoubleAttribute(getParameterXMLName(i))));
               setParameter(i, xmlState->getDoubleAttribute(getParameterXMLName(i)));
         }


            sendChangeMessage (this);
        }

        delete xmlState;
    }
}
