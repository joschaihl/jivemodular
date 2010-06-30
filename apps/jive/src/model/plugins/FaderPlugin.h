
#ifndef __JUCETICE_FADERPLUGIN_HEADER__
#define __JUCETICE_FADERPLUGIN_HEADER__

#include "../BasePlugin.h"

//==============================================================================
/**
    Very simple stereo fader plugin for building aux sends, channels, crossfaders etc.
    Fades between 2 inputs for crossfade function; leave one input pair disconnected for
    standard fader.
    
    Linear at present, could in future use dB scale, or turn more into a channel strip with EQ, compressor etc.
*/
class FaderPlugin : public BasePlugin
{
public:

    //==============================================================================
    FaderPlugin();
    ~FaderPlugin();

    //==============================================================================
    int getType () const                               { return JOST_PLUGINTYPE_UTILITYFADER; }

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

    //==============================================================================
    // save/load preset/synth/effect parameters
    virtual void savePresetToXml (XmlElement* element);
    virtual void loadPresetFromXml (XmlElement* element);

    //==============================================================================
    bool hasEditor () const;
    bool wantsEditor () const;
    
private:
   float faderPosition;
   float prevFaderPosition;

};

#endif // __JUCETICE_FADERPLUGIN_HEADER__
