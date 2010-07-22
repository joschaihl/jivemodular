
#ifndef PATTERNMATRIXPLUGIN_H
#define PATTERNMATRIXPLUGIN_H

#include "Config.h"

#include "../../BasePlugin.h"

//==============================================================================
class PatternMatrixPlugin : public BasePlugin
{
public:

    //==============================================================================
    PatternMatrixPlugin();
    ~PatternMatrixPlugin();

    //==============================================================================
    int getType () const                 { return JOST_PLUGINTYPE_MIDIPATTERNMATRIX; }

    //==============================================================================
    const String getName () const        { return T("PatternMatrix"); }
    int getNumInputs () const            { return 0; }
    int getNumOutputs () const           { return 0; }
    int getNumMidiInputs () const        { return 1; }
    int getNumMidiOutputs () const       { return 1; }
    bool acceptsMidi () const            { return true; }
    bool producesMidi () const           { return true; }
    bool isMidiOutput () const           { return false; }

    //==============================================================================
    bool hasEditor () const              { return true; }
    bool wantsEditor () const            { return true; }
    bool isEditorInternal () const       { return true; }
    AudioProcessorEditor* createEditor();

    //==============================================================================
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

   //==============================================================================
   void setPartCC(int part, int ccNum);
   int getPartCC(int part);
   int getPartPattern(int part);
   int getQuantize() { return quantizeLength;};
   void setQuantize(double q) {quantizeLength = q;};
   int getMidiChannel() { return midiChannel;};
   void setMidiChannel(int q) {if (midiChannel >=1 && midiChannel <=16) midiChannel = q;};

    //==============================================================================
    void setParameterReal (int paramNumber, float value);
    float getParameterReal (int paramNumber);
    const String getParameterTextReal (int partNumber, float value);

private:
   int currentPattern[MAXPARTS];
   int playingPattern[MAXPARTS];
   int partCC[MAXPARTS];
   double quantizeLength; // in bars
   int midiChannel;
   
   Transport* transport;
};


#endif // PATTERNMATRIXPLUGIN_H
