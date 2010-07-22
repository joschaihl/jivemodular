
#include "Config.h"
#include "HostFilterBase.h"

#include "PatternMatrixPlugin.h"
#include "PatternMatrixEditor.h"

const int QuantiseParamId = MAXPARTS;

//==============================================================================
PatternMatrixPlugin::PatternMatrixPlugin ()
:
   transport(0),
   quantizeLength(1),
   midiChannel(1)
{
   // param for each part, and one for the quantize amount
   setNumParameters( + 1);
   
   AudioParameter* parameter = 0;

   for (int i = 0; i < MAXPARTS; i++)
   {
      parameter = new AudioParameter ();

      parameter->part (i);
      parameter->name (String("Part #") + String(i));
      parameter->get (MakeDelegate (this, &PatternMatrixPlugin::getParameterReal));
      parameter->set (MakeDelegate (this, &PatternMatrixPlugin::setParameterReal));
      parameter->text (MakeDelegate (this, &PatternMatrixPlugin::getParameterTextReal));

      registerParameter (i, parameter);
      
      currentPattern[i] = 0;
      playingPattern[i] = 0;
      partCC[i] = 127 - i;
   }
   
   parameter = new AudioParameter ();

   parameter->part (QuantiseParamId);
   parameter->name ("Quantize");
   parameter->get (MakeDelegate (this, &PatternMatrixPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &PatternMatrixPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &PatternMatrixPlugin::getParameterTextReal));

   registerParameter (QuantiseParamId, parameter);
}

PatternMatrixPlugin::~PatternMatrixPlugin ()
{
   removeAllParameters (true);
}

void PatternMatrixPlugin::setPartCC(int part, int ccNum)
{
   if (part < MAXPARTS)
      partCC[part] = ccNum;
   //sendChangeMessage(this);
}

int PatternMatrixPlugin::getPartCC(int part)
{
   int ccNum = 0;
   if (part < MAXPARTS)
      ccNum = partCC[part];
   return ccNum;
}

int PatternMatrixPlugin::getPartPattern(int part)
{
   int pattern = 0;
   if (part < MAXPARTS)
      pattern = currentPattern[part];
   return pattern;
}

//==============================================================================
void PatternMatrixPlugin::setParameterReal (int paramNumber, float value)
{
   if (paramNumber < MAXPARTS)
      currentPattern[paramNumber] = (value + 1.0 / MAXPATTERNSPERPART) * (MAXPATTERNSPERPART); // add 1 so the round rounds us where we want
   else if (paramNumber == QuantiseParamId)
   {
   
   }   
}

float PatternMatrixPlugin::getParameterReal (int paramNumber)
{
   float paramVal = 0;
   if (paramNumber < MAXPARTS)
      paramVal = currentPattern[paramNumber] / static_cast<float>(MAXPATTERNSPERPART);
   else if (paramNumber == QuantiseParamId)
   {
   
   }
   return paramVal;
}

const String PatternMatrixPlugin::getParameterTextReal (int paramNumber, float value)
{
   String paramVal;
   if (paramNumber < MAXPARTS)
   {
      paramVal = String(playingPattern[paramNumber]);
      if (currentPattern[paramNumber] != playingPattern[paramNumber])
         paramVal = paramVal + String(" (") + String(currentPattern[paramNumber]) + String(")");
   }
   else if (paramNumber == QuantiseParamId)
   {
   
   }  
   return paramVal;
}

//==============================================================================
void PatternMatrixPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    transport = getParentHost()->getTransport ();

}

void PatternMatrixPlugin::releaseResources()
{
}

void PatternMatrixPlugin::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
   MidiBuffer* midiBuffer = midiBuffers.getUnchecked (0);
   
   // process midi automation
   midiAutomatorManager.handleMidiMessageBuffer(*midiBuffer);

   int eventPos = 0;
   int predelay = 1;
   
   // quantise calculation
   int qths = getQuantize();
   if (qths > 0)
   {
      if (transport->isPlaying())
      {
         eventPos = -1; // default to nothing happening, only happens if on a quantise boundary
         const int frameCounter = transport->getPositionInFrames ();
         const int framesPerBeat = transport->getFramesPerBeat ();
         const int blockSize = buffer.getNumSamples ();
         const int quantiseFrameCount = framesPerBeat * (1.0 / qths) * transport->getTimeDenominator();
         int frameEnd = frameCounter + blockSize;
         int frameEndQuantised = frameEnd / quantiseFrameCount; // the number of quantises since zero, remainder rounded off
         int quantime = frameEndQuantised * quantiseFrameCount;
         if (quantime >= frameCounter && quantime <= frameEnd)
         {
            eventPos = (quantime - frameCounter);
            eventPos = predelay > eventPos ? eventPos - predelay : eventPos; 
         }   
         
      }
   }
   
   if (midiBuffer)
   {
      // eat all the midi input, we don't pass it thru
      midiBuffer->clear();
      
      // fill up output buffer with any CC changes
      for (int i = 0; (i < MAXPARTS) && (eventPos >= 0); i++)
      {
         if (currentPattern[i] != playingPattern[i])
         {
            // render a CC...
            float val = 1.0 / MAXPATTERNSPERPART + (currentPattern[i] / static_cast<float>(MAXPATTERNSPERPART)) * 127.0;
            midiBuffer->addEvent(MidiMessage::controllerEvent(midiChannel, partCC[i], val), eventPos);
            
            // effect the state change
            playingPattern[i] = currentPattern[i];
            sendChangeMessage(this);
         }
      }
   }
}

//==============================================================================
AudioProcessorEditor* PatternMatrixPlugin::createEditor ()
{
    return new PatternMatrixEditor (this);
}

