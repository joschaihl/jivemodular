
#ifndef __JUCETICE_TRANSPORTPLUGIN_HEADER__
#define __JUCETICE_TRANSPORTPLUGIN_HEADER__

#include "../BasePlugin.h"

//==============================================================================
/**
    Transport control plugin - alowing midi bindings to transport tempo, start, 
    stop etc via normal plugin binding system.
*/
class TransportControlPlugin : public BasePlugin
{
public:

    //==============================================================================
    TransportControlPlugin();
    ~TransportControlPlugin();

    //==============================================================================
    int getType () const                               { return JOST_PLUGINTYPE_UTILITYTRANSPORTCONTROL; }

    //==============================================================================
    const String getName () const;
    int getID () const;
    int getNumInputs () const;
    int getNumOutputs () const;
    int getNumMidiInputs () const;
    int getNumMidiOutputs () const;
    bool acceptsMidi () const;
    void* getLowLevelHandle ();

    //==============================================================================
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    //==============================================================================
    int getNumParameters();
    void setParameterReal (int paramNumber, float value);
    float getParameterReal (int paramNumber);
    const String getParameterTextReal (int paramNumber, float value);

    //==============================================================================
    int getNumPrograms ();
    void setCurrentProgram (int programNumber);
    int getCurrentProgram ();
    const String getProgramName (int programNumber);
    const String getCurrentProgramName ();


void   loadPresetFromXml (XmlElement* xml);
    //==============================================================================
    bool hasEditor () const;
    bool wantsEditor () const;
    
private:
   double RoundBPM(double);


   double maxBPM;
   double minBPM;

   // soft takeover support .. later
   double lastRequestedBPM;
   bool softTakeover;
   
   bool loadingParams;
};

#endif // __JUCETICE_TRANSPORTPLUGIN_HEADER__
