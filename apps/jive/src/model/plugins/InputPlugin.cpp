/*
 ==============================================================================

 This file is part of the JUCETICE project - Copyright 2007 by Lucio Asnaghi.

 JUCETICE is based around the JUCE library - "Jules' Utility Class Extensions"
 Copyright 2007 by Julian Storer.

 ------------------------------------------------------------------------------

 JUCE and JUCETICE can be redistributed and/or modified under the terms of
 the GNU Lesser General Public License, as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 JUCE and JUCETICE are distributed in the hope that they will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with JUCE and JUCETICE; if not, visit www.gnu.org/licenses or write to
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA

 ==============================================================================
*/

#include "InputPlugin.h"
#include "HostFilterBase.h"
#include "../../ui/plugins/PluginEditorComponent.h"


//==============================================================================
InputPlugin::InputPlugin (const int numChannels_)
  : numChannels (numChannels_)
{
    setValue (PROP_GRAPHXPOS, 300);
    setValue (PROP_GRAPHYPOS, 2);
}

InputPlugin::~InputPlugin ()
{
}

//==============================================================================
void InputPlugin::processBlock (AudioSampleBuffer& buffer,
                                MidiBuffer& midiMessages)
{
    const int blockSize = buffer.getNumSamples ();

#if JucePlugin_WantsMidiInput
    MidiBuffer* midiBuffer = midiBuffers.getUnchecked (0);

    if (midiMessages.getNumEvents() > 0)
        *midiBuffer = midiMessages;
#endif

    int numActiveInChans = 0;
    const int numInputsTotal = buffer.getNumChannels();
    const int numInputsWanted = outputBuffer->getNumChannels();

    for (int i = 0; i < jmin (numInputsWanted, numInputsTotal); ++i)
    {
        // copy inputs to our intenal buffer
        outputBuffer->copyFrom (numActiveInChans++,
                                0,
                                buffer.getSampleData (i),
                                blockSize);    
    }

    while (numActiveInChans < numInputsWanted)
    {
        outputBuffer->clear (numActiveInChans++, 0, outputBuffer->getNumSamples());
    }
}

//==============================================================================
AudioProcessorEditor* InputPlugin::createEditor ()
{
    return 0;
}

//==============================================================================
const int NUMPARAMS = 4; // bpm, bpm max, bpm min, play/stop
enum ParamIDs
{
   TempoCurrent = 0,
   TempoMin,
   TempoMax,
   PlayStop
};

TransportInputPlugin::TransportInputPlugin(const int numChannels)
:
   InputPlugin(numChannels),
   maxBPM(200),
   minBPM(50),
   lastRequestedBPM(-1),
   softTakeover(false),
   loadingParams(false)
{
   setNumParameters (NUMPARAMS);

   AudioParameter* parameter = new AudioParameter ();
   parameter->part (TempoCurrent);
   parameter->name ("Tempo BPM");
   parameter->get (MakeDelegate (this, &TransportInputPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &TransportInputPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &TransportInputPlugin::getParameterTextReal));
   registerParameter(TempoCurrent, parameter);

   parameter = new AudioParameter ();
   parameter->part (TempoMin);
   parameter->name ("Tempo Min");
   parameter->get (MakeDelegate (this, &TransportInputPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &TransportInputPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &TransportInputPlugin::getParameterTextReal));
   registerParameter(TempoMin, parameter);

   parameter = new AudioParameter ();
   parameter->part (TempoMax);
   parameter->name ("Tempo Max");
   parameter->get (MakeDelegate (this, &TransportInputPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &TransportInputPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &TransportInputPlugin::getParameterTextReal));
   registerParameter(TempoMax, parameter);

   parameter = new AudioParameter ();
   parameter->part (PlayStop);
   parameter->name ("Play/Stop");
   parameter->get (MakeDelegate (this, &TransportInputPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &TransportInputPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &TransportInputPlugin::getParameterTextReal));
   registerParameter(PlayStop, parameter);
}

TransportInputPlugin::~TransportInputPlugin()
{
}

//==============================================================================
int TransportInputPlugin::getNumParameters()
{
   return NUMPARAMS;
}

double TransportInputPlugin::RoundBPM(double bpm)
{
   double dp2 = 100;
   int bpmtenk = ((bpm) * dp2 + 0.5);
   return bpmtenk / dp2;
}

void TransportInputPlugin::setParameterReal (int index, float value)
{
   Transport* t = getParentHost()->getTransport();
   switch (index)
   {
   case TempoCurrent:
   {
      double oldBpm = t->getTempo();
      double newBpm = RoundBPM(minBPM + (value * (maxBPM - minBPM)));
      double smallbpm = jmin(newBpm, lastRequestedBPM);
      double bigbpm = jmax(newBpm, lastRequestedBPM);
      if (lastRequestedBPM != -1 && oldBpm >= smallbpm && oldBpm <= bigbpm)
         t->setTempo(newBpm); // warning! could cause horrible jumps if tempo set from toolbar, or max/min have recently changed!
      // todo - soften changes, or soft takeover
      lastRequestedBPM = newBpm;
   }
   break;
   case TempoMin:
      minBPM = int(value * 250); // 0-256 BPM, 0<min<250
      if (maxBPM <= minBPM)
      {
         maxBPM = minBPM + 6;
      }
      lastRequestedBPM = -1; // trigger soft-takeover for next tempo change
   break;
   case TempoMax:
      maxBPM = int(6 + (value * 250)); // 0-256 BPM, 6<max<256
      if (minBPM >= maxBPM)
      {
         minBPM = maxBPM - 6;
      }
      lastRequestedBPM = -1; // trigger soft-takeover for next tempo change
   break;
   case PlayStop:
      if (!loadingParams)
         t->togglePlay();
   break;
   }
}

float TransportInputPlugin::getParameterReal (int index)
{
   Transport* t = getParentHost()->getTransport();
   float val = 0.0;
   switch (index)
   {
   case TempoCurrent:
   {
//      double oldBpm = t->getTempo(); 
      double oldBpm = lastRequestedBPM; // show the requested value, works better during soft-takeover
      if (oldBpm == -1)
         oldBpm = t->getTempo(); // ..eexcept when last value is -1 (i.e. init, first pass after soft-takeover trigger)
      val = (oldBpm - minBPM) / (maxBPM - minBPM);
   }
   break;
   case TempoMin:
      val = minBPM / 250;
   break;
   case TempoMax:
      val = (maxBPM - 6) / 250;
   break;
   case PlayStop:
      if (t->isPlaying())
         val = 1.0;
      else
         val = 0.0;
   break;
   }
   return val;
}

const String TransportInputPlugin::getParameterTextReal (int index, float value)
{
   String valText;
   switch (index)
   {
   case TempoCurrent:
      valText = String(minBPM + (value * (maxBPM - minBPM)));
   break;
   case TempoMin:
      valText = String(minBPM);
   break;
   case TempoMax:
      valText = String(maxBPM);
   break;
   case PlayStop:
//      if (value >= 0.5) 
//         valText = String("Playing");
//      else
//         valText = String("Stopped");
   break;
   }
   return valText;
}

void TransportInputPlugin::loadPresetFromXml(XmlElement* xml)
{
   loadingParams = true;
   BasePlugin::loadPresetFromXml(xml);
   loadingParams = false;
}

//==============================================================================
void TransportInputPlugin::processBlock (AudioSampleBuffer& buffer,
                               MidiBuffer& midiMessages)
{
   InputPlugin::processBlock(buffer, midiMessages);
   
   if (midiBuffers.size() > 0)
   {
      MidiBuffer* midiBuffer = midiBuffers.getUnchecked (0);

      // process midi automation
      midiAutomatorManager.handleMidiMessageBuffer(*midiBuffer);
   }
}
