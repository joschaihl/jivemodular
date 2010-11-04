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

#ifndef __JUCETICE_JOSTWRAPPEDJUCEPLUGIN_HEADER__
#define __JUCETICE_JOSTWRAPPEDJUCEPLUGIN_HEADER__

#include "../BasePlugin.h"

//==============================================================================
/**
    Juce-handled plugin wrapper class (Juce can load and run VST and AU nicely for us).
*/
class WrappedJucePlugin : public BasePlugin
{
public:

    //==============================================================================
    WrappedJucePlugin (PluginDescription* descToLoad, bool isInternal=false);
    ~WrappedJucePlugin ();

    //==============================================================================
    int getType () const                               { return JOST_PLUGINTYPE_WRAPPEDJUCEVST; }

    //==============================================================================
    bool loadPluginStuff ();
    bool loadPluginFromFile (const File& filePath) { return true; };
    File getFile () const                              { return File::nonexistent; };

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
    
    AudioPluginInstance* getAudioPluginInstance() { return instance; };
    const PluginDescription& getPluginDescription() { return pluginDescription; };

   bool isInternal() const { return isInternalPlugin; };

private:

   PluginDescription pluginDescription;
   AudioPluginInstance* instance;
   bool isInternalPlugin;
};

#endif // __JUCETICE_JOSTWRAPPEDJUCEPLUGIN_HEADER__
