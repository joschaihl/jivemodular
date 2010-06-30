

#include "FaderPlugin.h"
#include "HostFilterBase.h"

//==============================================================================
FaderPlugin::FaderPlugin()
:
   faderPosition(0.5),
   prevFaderPosition(0.5)
{
   setNumParameters (1);

   AudioParameter* parameter = new AudioParameter ();
   parameter->part (0);
   parameter->name ("Fader Position");
   parameter->get (MakeDelegate (this, &FaderPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &FaderPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &FaderPlugin::getParameterTextReal));
   registerParameter(0, parameter);
}

FaderPlugin::~FaderPlugin()
{
}

//==============================================================================
const String FaderPlugin::getName () const
{
   return "Utility Fader";
}

int FaderPlugin::getID () const
{
    return JOST_PLUGINTYPE_UTILITYFADER;
}

int FaderPlugin::getNumInputs () const
{
    return 4; // 2 stereo inputs
}

int FaderPlugin::getNumOutputs () const
{
    return 2; // single stereo output
}

int FaderPlugin::getNumMidiInputs () const
{
   return 1; // midi control of fader + midi thru
}

int FaderPlugin::getNumMidiOutputs () const
{
    return 1; // midi thru
}

void* FaderPlugin::getLowLevelHandle ()
{
    return 0;
}

bool FaderPlugin::acceptsMidi () const
{
    return true;
}

//==============================================================================
void FaderPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void FaderPlugin::releaseResources()
{
}

//==============================================================================
void FaderPlugin::processBlock (AudioSampleBuffer& buffer,
                               MidiBuffer& midiMessages)
{
    const int blockSize = buffer.getNumSamples ();    

   // Similar for midi input
   MidiBuffer dud;
   MidiBuffer* midiBuffer = &dud;
   if (midiBuffers.size() > 0)
   {
      midiBuffer = midiBuffers.getUnchecked (0);

      // add events from keyboards
      keyboardState.processNextMidiBuffer(*midiBuffer, 0, blockSize, true);

      // process midi automation
      midiAutomatorManager.handleMidiMessageBuffer(*midiBuffer);
   }
   
   if (inputBuffer && outputBuffer && blockSize)
   {
      // do the plugin work:
      // apply 1.0-faderPosition gain to input A l+r (chs 1 & 2 in buffer)
      float inverseStart = 1.0-prevFaderPosition;
      float inverseEnd = 1.0-faderPosition;
      for (int i = 0; i < 2; i++)
         inputBuffer->applyGainRamp(i, 0, blockSize, inverseStart, inverseEnd);
      // apply faderPosition gain to input B l+r  (chs 3 & 4 in buffer)
      for (int i = 2; i < 4; i++)
         inputBuffer->applyGainRamp(i, 0, blockSize, prevFaderPosition, faderPosition);
         
      // sum into output l+r (ch 1 & 2 of buffer)
      for (int i=0; i<2; i++)
      {
         outputBuffer->copyFrom(i, 0, *inputBuffer, i, 0, blockSize);
         outputBuffer->addFrom(i, 0, *inputBuffer, i+2, 0, blockSize);
      }
   }
   
   prevFaderPosition = faderPosition;
}

//==============================================================================
int FaderPlugin::getNumParameters()
{
   return 1;
}

void FaderPlugin::setParameterReal (int index, float value)
{
   if (index == 0)
   {
      // we will always ramp changes over one buffer length so it's smooth
      // note potential for long latency if buffer size is big
      prevFaderPosition = faderPosition;
      faderPosition = value;
   }
}

float FaderPlugin::getParameterReal (int index)
{
   float val = 0.0;
   if (index == 0)
      val = faderPosition;
    return val;
}

const String FaderPlugin::getParameterTextReal (int index, float value)
{
   String valText;
   if (index == 0)
      valText = String(faderPosition);
    return valText;
}

//==============================================================================
int FaderPlugin::getNumPrograms ()
{
    return 0;
}

void FaderPlugin::setCurrentProgram (int programNumber)
{

}

int FaderPlugin::getCurrentProgram ()
{
    return 0;
}

const String FaderPlugin::getProgramName (const int programNumber)
{
    return T("Default preset");
}

const String FaderPlugin::getCurrentProgramName ()
{
    return getProgramName (0);
}

//==============================================================================
 // default editor will be awesome .. I think this is the right combo of flags?
 bool FaderPlugin::hasEditor () const
{
    return false;
}
bool FaderPlugin::wantsEditor () const
{
    return true;
}

