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
#include "JiveGateFilter.h"

float GateNotes[] = 
{
   0, // off
   1.0/8, // one every 8 bars
   1.0/6, // 
   1.0/4, // one every 4 bars
   1.0/3, // 
   1.0/2, // one every 2 bars
   2.0/3, // 
   1, // 1 per bar
   3.0/2, // 
   2, // 2 per bar
   3, // 3 per bar
   4, // 
   6, // 
   8,
   12,
   16,
   24,
   32,
   48,
   64
};

//==============================================================================
/**
    This function must be implemented to create a new instance of your
    plugin object.
*/
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JiveGateFilter();
}

void ppqToBarBeatTick(const double ppq, const int numerator, const int denominator, const double ticksPerBar, int& bar, int& beat, double& tick)
{
   bar = 0;
   beat = 0;
   tick = 0;

//    if (numerator == 0 || denominator == 0)
//        return T("1|1|0");

    const int ppqPerBar = (numerator * 4 / denominator); // aka beats per bar
//    const double beats  = (fmod (ppq, ppqPerBar) / ppqPerBar) * numerator;
    const double beats  = (ppq / ppqPerBar) * numerator; // total beats, and fraction

    bar       = ((int) ppq) / ppqPerBar + 1;
    beat      = ((int) beats) + 1;
//    tick     = ((int) (fmod (beats * ppqPerBar, 1.0) * 960.0));
    tick     = ((int) ((beats / ppqPerBar) * ticksPerBar));

//    String s;
//    s << bar << T('|') << beat << T('|') << ticks;
//    return s;
}

//==============================================================================
JiveGateFilter::JiveGateFilter()
:
   attack(0.01),
//   decay(0),
//   sustain(1),
   release(0.2),
   noise(0.3),
   sampleRate(44100),
   gateRate(1)
{
    zeromem(&adsrState, sizeof(adsrState));
}

JiveGateFilter::~JiveGateFilter()
{
}

//==============================================================================
const String JiveGateFilter::getName() const
{
    return JucePlugin_Name;
}

float JiveGateFilter::getParameter (int index)
{
   float paramValue = 0.0;
   
   if (index == Rate)
      paramValue = gateRate; // need inverse mapping!
   if (index == Attack)
      paramValue = attack;
//   if (index == Decay)
//      paramValue = decay;
//   if (index == Sustain)
//      paramValue = sustain;
   if (index == Release)
      paramValue = release; // for some reason need to fudge factor the release by half - something wrong (or I don't understand) in adsr
   if (index == Noise)
      paramValue = noise;
   
   return paramValue;
}

double JiveGateFilter::rateParamToTicksPerBar()
{
      int index = round(gateRate * sizeof(GateNotes)/sizeof(GateNotes[0])-1);
      return GateNotes[int(index)];
}

void JiveGateFilter::setParameter (int index, float newValue)
{   
   if (index == Rate)
   {
      gateRate = newValue;
      sendChangeMessage (this);
   }
   if (index == Attack)
   {
      attack = newValue;
      sendChangeMessage (this);
   }
   if (index == Release)
   {
      release = newValue;
      sendChangeMessage (this);
   }
   if (index == Noise)
   {
      noise = newValue;
      sendChangeMessage (this);
   }
}

double JiveGateFilter::logifyEnvelopeParameter(double newValue)
{
   float max = 10;
   float min = 1;
   int logstrength = 5;
   newValue = pow(log10(newValue * (max - min) + min), logstrength);
   return newValue;
}

const String JiveGateFilter::getParameterName (int index)
{
   String name(String("Placeholder") + String(index));
   
   if (index == Rate)
      name = String("Rate");
   if (index == Attack)
      name = String("Attack");
   if (index == Release)
      name = String("Release");
   if (index == Noise)
      name = String("Noise");

   return name;
}

const String JiveGateFilter::getParameterText (int index)
{
   String valueText = String(getParameter(index), 6);
   if (index == Rate)
      valueText = String(rateParamToTicksPerBar(), 6);
      
   return valueText;
}

const String JiveGateFilter::getInputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

const String JiveGateFilter::getOutputChannelName (const int channelIndex) const
{
    return String (channelIndex + 1);
}

bool JiveGateFilter::isInputChannelStereoPair (int index) const
{
    return false;
}

bool JiveGateFilter::isOutputChannelStereoPair (int index) const
{
    return false;
}

bool JiveGateFilter::acceptsMidi() const
{
    return true;
}

bool JiveGateFilter::producesMidi() const
{
    return true;
}

//==============================================================================
void JiveGateFilter::prepareToPlay (double sampleRate_, int samplesPerBlock)
{
   sampleRate = sampleRate_;
   bar = 0; beat = 0; 
   prevBeat = -1;
}

void JiveGateFilter::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void JiveGateFilter::processBlock (AudioSampleBuffer& buffer,
                                   MidiBuffer& midiMessages)
{
   AudioPlayHead::CurrentPositionInfo pos;

   bool rolling = false;
   double ppqPosition = 0, tick = 0;
   double beatsPerSample = 0;
   if (getPlayHead() != 0 && getPlayHead()->getCurrentPosition(pos))
   {
      rolling = pos.isPlaying;
      if (rolling)
      {
         ppqPosition = pos.ppqPosition;
         beatsPerSample = (1.0 / sampleRate) * (60.0 / pos.bpm);
      }
   }

   // for each sample
   for (int sample = 0; sample < buffer.getNumSamples(); sample++)
   { 
      bool trig = false;
      
      // perform any triggers
      if (rolling)
      {
         ppqToBarBeatTick(ppqPosition, pos.timeSigNumerator, pos.timeSigDenominator, rateParamToTicksPerBar(), bar, beat, tick);
         if (prevBeat != tick)
         {   
            trig = true;
            adsr_trigger(&adsrState, logifyEnvelopeParameter(attack) * sampleRate, fudgeReleaseTime() * sampleRate, 0, 0);
            //std::cerr << "trigger!" << std::endl;
         }
      }

      adsr_process(&adsrState);
      
      for (int channel = 0; channel < getNumInputChannels(); ++channel)
      {     
         float* sampleData = buffer.getSampleData(channel);
         sampleData[sample] = adsrState.env * ((0.5 - rand() / static_cast<float>(RAND_MAX)) * (noise) + sampleData[sample] * (1.0 - noise) / 2.0);
      }
      
      ppqPosition += beatsPerSample;
      prevBeat = rolling ? tick : -1; // ensure we get the first tick when we restart
   }
}

//==============================================================================
AudioProcessorEditor* JiveGateFilter::createEditor()
{
    return 0;
}

//==============================================================================
void JiveGateFilter::getStateInformation (MemoryBlock& destData)
{
    // create an outer XML element..
    XmlElement xmlState (T("JiveGateSettings"));

    // add some attributes to it..
    xmlState.setAttribute (T("pluginVersion"), 1);
    for (int i=0; i<getNumParameters(); i++)
        xmlState.setAttribute(getParameterName(i), getParameter(i));

    // then use this helper function to stuff it into the binary blob and return it..
    copyXmlToBinary (xmlState, destData);
}

void JiveGateFilter::setStateInformation (const void* data, int sizeInBytes)
{
    // use this helper function to get the XML from this binary blob..
    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);

    if (xmlState != 0)
    {
        // check that it's the right type of xml..
        if (xmlState->hasTagName (T("JiveGateSettings")))
        {
          for (int i=0; i<getNumParameters(); i++)
              setParameter(i, xmlState->getDoubleAttribute(getParameterName(i), getParameter(i)));

            sendChangeMessage (this);
        }

        delete xmlState;
    }
}
