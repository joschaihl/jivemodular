

#include "TransportControl.h"
#include "HostFilterBase.h"

const int NUMPARAMS = 4; // bpm, bpm max, bpm min, play/stop
enum ParamIDs
{
   TempoCurrent = 0,
   TempoMin,
   TempoMax,
   PlayStop
};


//==============================================================================
TransportControlPlugin::TransportControlPlugin()
:
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
   parameter->get (MakeDelegate (this, &TransportControlPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &TransportControlPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &TransportControlPlugin::getParameterTextReal));
   registerParameter(TempoCurrent, parameter);

   parameter = new AudioParameter ();
   parameter->part (TempoMin);
   parameter->name ("Tempo Min");
   parameter->get (MakeDelegate (this, &TransportControlPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &TransportControlPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &TransportControlPlugin::getParameterTextReal));
   registerParameter(TempoMin, parameter);

   parameter = new AudioParameter ();
   parameter->part (TempoMax);
   parameter->name ("Tempo Max");
   parameter->get (MakeDelegate (this, &TransportControlPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &TransportControlPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &TransportControlPlugin::getParameterTextReal));
   registerParameter(TempoMax, parameter);

   parameter = new AudioParameter ();
   parameter->part (PlayStop);
   parameter->name ("Play/Stop");
   parameter->get (MakeDelegate (this, &TransportControlPlugin::getParameterReal));
   parameter->set (MakeDelegate (this, &TransportControlPlugin::setParameterReal));
   parameter->text (MakeDelegate (this, &TransportControlPlugin::getParameterTextReal));
   registerParameter(PlayStop, parameter);
}

TransportControlPlugin::~TransportControlPlugin()
{
}

//==============================================================================
const String TransportControlPlugin::getName () const
{
   return "Transport";
}

int TransportControlPlugin::getID () const
{
    return JOST_PLUGINTYPE_UTILITYFADER;
}

int TransportControlPlugin::getNumInputs () const
{
    return 0; // no audio
}

int TransportControlPlugin::getNumOutputs () const
{
    return 0; // no audio
}

int TransportControlPlugin::getNumMidiInputs () const
{
   return 1; // midi control of fader
}

int TransportControlPlugin::getNumMidiOutputs () const
{
    return 0; // no midi thru
}

void* TransportControlPlugin::getLowLevelHandle ()
{
    return 0;
}

bool TransportControlPlugin::acceptsMidi () const
{
    return true;
}

//==============================================================================
void TransportControlPlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
}

void TransportControlPlugin::releaseResources()
{
}

//==============================================================================
void TransportControlPlugin::processBlock (AudioSampleBuffer& buffer,
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
}

//==============================================================================
int TransportControlPlugin::getNumParameters()
{
   return NUMPARAMS;
}

double TransportControlPlugin::RoundBPM(double bpm)
{
   double dp2 = 100;
   int bpmtenk = ((bpm) * dp2 + 0.5);
   return bpmtenk / dp2;
}

void TransportControlPlugin::setParameterReal (int index, float value)
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

float TransportControlPlugin::getParameterReal (int index)
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

const String TransportControlPlugin::getParameterTextReal (int index, float value)
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

void TransportControlPlugin::loadPresetFromXml(XmlElement* xml)
{
   loadingParams = true;
   BasePlugin::loadPresetFromXml(xml);
   loadingParams = false;
}

//==============================================================================
int TransportControlPlugin::getNumPrograms ()
{
    return 0;
}

void TransportControlPlugin::setCurrentProgram (int programNumber)
{

}

int TransportControlPlugin::getCurrentProgram ()
{
    return 0;
}

const String TransportControlPlugin::getProgramName (const int programNumber)
{
    return T("Default preset");
}

const String TransportControlPlugin::getCurrentProgramName ()
{
    return getProgramName (0);
}

//==============================================================================
 // default editor will be awesome .. I think this is the right combo of flags?
 bool TransportControlPlugin::hasEditor () const
{
    return false;
}
bool TransportControlPlugin::wantsEditor () const
{
    return true;
}

