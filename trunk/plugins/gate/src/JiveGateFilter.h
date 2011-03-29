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

#ifndef DEMOJUCEPLUGINFILTER_H
#define DEMOJUCEPLUGINFILTER_H

#include "adsr.h"

//==============================================================================
/**
   A rhythmic gater.
   
   Parameters:
   rate - 
   attack (seconds - 0-1)
   release (seconds - 0-1)
   noise - add some noise (0-1)
   
   To Do:
   Manual Trigger. Above 0.5 instantaneously triggers.
   Phase. So can have off-beat hi-hat style pattern with some groove. 
   Tricky balancing this with wanting different phases at different rates; but could just 
   implement as % latency, so works transparently for all rates. Or have a beat phase and 
   a percent phase in combination.
   Output note. A param for the note number, and the plugin outputs a note for each trigger.
   Duration. Could specify duration as a %, and this would affect trig off (currently is 
   instantaneous - sustain would mean something, could be brought back), and would also control 
   the output midi note's duration.

   Coming soon parameters:
   trigger - manually trigger at will (nice when rate is off!)
   length - 0: actual seconds; 1: scale whole adsr to 32 beats. (log scale?)
   phase ? beatoffset - 0.5: exactly on beat; 0: half beat early; 1: half beat late
   shape - coming soon, specify the shape of the curves in the adsr, if this is even necessary with adsr 
   
   Tricks:
   
   Map different held notes to different rates, so banging a key gives you a specific rhythm.
   
   Set attack to long and release to short for a reverse woosh build thing.

   Gone params:
      decay (seconds - 0-1)
   sustain (level)

*/
class JiveGateFilter  : public AudioProcessor,
                        public ChangeBroadcaster
{
public:
    //==============================================================================
    JiveGateFilter();
    ~JiveGateFilter();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    void processBlock (AudioSampleBuffer& buffer,
                       MidiBuffer& midiMessages);

    //==============================================================================
    AudioProcessorEditor* createEditor();

    //==============================================================================
    const String getName() const;

   enum 
   {
      Rate,
      Offset,
      Attack,
//      Decay,
//      Sustain,
      Release,
      Length,
      Trigger,
      Noise,
      NumParams
   };

    int getNumParameters() {return NumParams;};

    float getParameter (int index);
    void setParameter (int index, float newValue);

    const String getParameterName (int index);
    const String getParameterText (int index);

    const String getInputChannelName (const int channelIndex) const;
    const String getOutputChannelName (const int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    bool acceptsMidi() const;
    bool producesMidi() const;

    //==============================================================================
    int getNumPrograms()                                        { return 0; }
    int getCurrentProgram()                                     { return 0; }
    void setCurrentProgram (int index)                          { }
    const String getProgramName (int index)                     { return String::empty; }
    void changeProgramName (int index, const String& newName)   { }

    //==============================================================================
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
   double rateParamToTicksPerBar();
   double fudgeReleaseTime() {return release * 0.5;};
   bool isGateOpen() {return adsrState.state != ADSR_IDLE;};

   adsr_t adsrState; 
   double attack;
//   double decay;
//   double sustain;
   double release;
   double noise;
   int sampleRate;
   double gateRate;

   int bar, beat;
   double prevBeat;
};

#endif
