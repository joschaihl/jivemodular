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
#include "JucePluginCharacteristics.h"
#include "DistressorFilter.h"
//#include "DemoEditorComponent.h"

//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistressorFilter();
}

//==============================================================================
DistressorFilter::DistressorFilter()
{
	Pars = new OppressorParMan();
}

DistressorFilter::~DistressorFilter()
{
}

//==============================================================================
const String DistressorFilter::getName() const
{
    return JucePlugin_Name;
}

int DistressorFilter::getNumParameters()
{
	return Pars->getNumPars();
}

float DistressorFilter::getParameter (int index)
{
	return Pars->getParameter(index);
}

void DistressorFilter::setParameter (int index, float newValue)
{
	if (Pars->getParameter(index) != newValue) {Pars->setParameter(index,newValue);}

	//calcs here
    mode=0;
    thr = (float)pow(10.f, 2.f * Pars->getParameter(OppressorParMan::thresh) - 2.f);
    rat = 2.5f * Pars->getParameter(OppressorParMan::ratio) - 0.5f; 
    if(rat>1.0) { rat = 1.f + 16.f*(rat-1.f) * (rat - 1.f); mode = 1; }
    if(rat<0.0) { rat = 0.6f*rat; mode=1; }
    trim = (float)pow(10.f, 2.f * Pars->getParameter(OppressorParMan::level)); //was  - 1.f);
    att = (float)pow(10.f, -0.002f - 2.f * Pars->getParameter(OppressorParMan::attack));
    rel = (float)pow(10.f, -2.f - 3.f * Pars->getParameter(OppressorParMan::release));

    if(Pars->getParameter(OppressorParMan::limiter)>0.98)
    {
       lthr = 0.f; //limiter
    }
    else
    {
       lthr = 0.99f*(float)pow(10.0f,int(30.0*Pars->getParameter(OppressorParMan::limiter) - 20.0)/20.f); 
       mode = 1; 
    } 

    if(rat<0.0f && thr<0.1f) rat *= thr*15.f;
}

const String DistressorFilter::getParameterName (int index)
{
    switch (index){
     case 0: return String("thresh");  break;
     case 1: return String("ratio");  break;
	 case 2: return String("level");  break;
	 case 3: return String("attack");  break;
     case 4: return String("release");  break;
	 case 5: return String("limiter");  break;
	}

    return String::empty;
}

const String DistressorFilter::getParameterText (int index)
{
    
	return String(Pars->getParameter(index));
}
const String DistressorFilter::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String DistressorFilter::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool DistressorFilter::isInputChannelStereoPair (int index) const
{
    return false;
}

bool DistressorFilter::isOutputChannelStereoPair (int index) const
{
    return false;
}

bool DistressorFilter::acceptsMidi() const
{
    return true;
}

bool DistressorFilter::producesMidi() const
{
    return true;
}

//==============================================================================
void DistressorFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // do your pre-playback setup stuff here..
}

void DistressorFilter::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void DistressorFilter::oppressor(AudioSampleBuffer* buffer, AudioSampleBuffer* out, int sampleFrames)
{
	float *in1 = buffer->getSampleData (0);
	float *in2 = buffer->getSampleData (1);
	float *out1 = out->getSampleData (0);
	float *out2 = out->getSampleData (1);

	float a, b, i, j, g, e=env, e2=env2, ra=rat, re=(1.f-rel), at=att;
	float tr=trim, th=thr, lth=lthr; // , xth=xthr;  
	
	--in1;	
	--in2;	
	--out1;
	--out2;

	if(mode) //comp/gate/lim
	{
		if(lth==0.f) lth=1000.f;
		while(--sampleFrames >= 0)
		{
			a = *++in1;
			b = *++in2;

			i = (a<0.f)? -a : a;
			j = (b<0.f)? -b : b;
			i = (j>i)? j : i;

			e = (i>e)? e + at * (i - e) : e * re;
			e2 = (i>e)? i : e2 * re; //ir;

			g = (e>th)? tr / (1.f + ra * ((e/th) - 1.f)) : tr;

			if(g<0.f) g=0.f; 
			if(g*e2>lth) g = lth/e2; //limit 

			//  ge = (e>xth)? ge + ga - ga * ge : ge * xrat; //gate

			*++out1 = a * (g);	
			*++out2 = b * (g);	
		}
	}
	else //compressor only
	{
		while(--sampleFrames >= 0)
		{
		a = *++in1;
		b = *++in2;

		i = (a<0.f)? -a : a;
		j = (b<0.f)? -b : b;
		i = (j>i)? j : i; //get peak level

		e = (i>e)? e + at * (i - e) : e * re; //envelope
		g = (e>th)? tr / (1.f + ra * ((e/th) - 1.f)) : tr; //gain

		*++out1 = a * (g); //vca
		*++out2 = b * (g);	
		}
	}

	if(e <1.0e-10) env =0.f; else env =e;
	if(e2<1.0e-10) env2=0.f; else env2=e2;
	// if(ge<1.0e-10) genv=0.f; else genv=ge;
}

void DistressorFilter::processBlock (AudioSampleBuffer& buffer,
                                   MidiBuffer& midiMessages)
{
   AudioSampleBuffer inputBuffer(buffer);
   oppressor(&inputBuffer, &buffer, buffer.getNumSamples());
   

    // for each of our input channels, we'll attenuate its level by the
    // amount that our volume parameter is set to.
//    for (int channel = 0; channel < getNumInputChannels(); ++channel)
//    {
//        buffer.applyGain (channel, 0, buffer.getNumSamples(), gain);
//        
//        // mix in opposite ratio of noise (i.e. generator)
//        float* sampleData = buffer.getSampleData(channel);
//         for (int sample = 0; sample < buffer.getNumSamples(); sample++)
//            sampleData[sample] = (rand() / static_cast<float>(RAND_MAX)) * (1.0 - gain) + sampleData[sample];
//    }
//
//    // in case we have more outputs than inputs, we'll clear any output
//    // channels that didn't contain input data, (because these aren't
//    // guaranteed to be empty - they may contain garbage).
//    for (int i = getNumInputChannels(); i < getNumOutputChannels(); ++i)
//    {
//        buffer.clear (i, 0, buffer.getNumSamples());
//    }

}

//==============================================================================
AudioProcessorEditor* DistressorFilter::createEditor()
{
//    return new DemoEditorComponent (this);
return 0;
}

//==============================================================================
void DistressorFilter::getStateInformation (MemoryBlock& destData)
{
    // you can store your parameters as binary data if you want to or if you've got
    // a load of binary to put in there, but if you're not doing anything too heavy,
    // XML is a much cleaner way of doing it - here's an example of how to store your
    // params as XML..

    // create an outer XML element..
    XmlElement xmlState (T("MYPLUGINSETTINGS"));

    // add some attributes to it..
    xmlState.setAttribute (T("pluginVersion"), 1);
//    xmlState.setAttribute (T("gainLevel"), gain);
//    xmlState.setAttribute (T("uiWidth"), lastUIWidth);
//    xmlState.setAttribute (T("uiHeight"), lastUIHeight);

    // you could also add as many child elements as you need to here..


    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
}

void DistressorFilter::setStateInformation (const void* data, int sizeInBytes)
{
    // use this helper function to get the XML from this binary blob..
    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState != 0)
    {
        // check that it's the right type of xml..
        if (xmlState->hasTagName (T("MYPLUGINSETTINGS")))
        {
            // ok, now pull out our parameters..
//            gain = (float) xmlState->getDoubleAttribute (T("gainLevel"), gain);
//
//            lastUIWidth = xmlState->getIntAttribute (T("uiWidth"), lastUIWidth);
//            lastUIHeight = xmlState->getIntAttribute (T("uiHeight"), lastUIHeight);

            sendChangeMessage (this);
        }

        delete xmlState;
    }
}
