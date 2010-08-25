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

#include "WrappedJucePlugin.h"
#include "HostFilterBase.h"

//==============================================================================
WrappedJucePlugin::WrappedJucePlugin (PluginDescription* desc)
:
   instance(0)
{
   if (desc)
   {
      pluginDescription = *desc;
      String errorMessage;
        
      instance = AudioPluginFormatManager::getInstance()->createPluginInstance (pluginDescription, errorMessage);

      loadPluginStuff();
   }
}

WrappedJucePlugin::~WrappedJucePlugin ()
{
   delete instance;
}

//==============================================================================
//bool WrappedJucePlugin::loadPluginFromFile (const File& filePath)
//{
//    ptrLibrary = PlatformUtilities::loadDynamicLibrary (filePath.getFullPathName());
//
//    if (ptrLibrary != 0)
//    {
//        DSSI_Descriptor_Function pfDescriptorFunction
//                = (DSSI_Descriptor_Function)
//                        PlatformUtilities::getProcedureEntryPoint (ptrLibrary, T("dssi_descriptor"));
//
//        if (pfDescriptorFunction != 0)
//        {
//            for (uint32 iPluginIndex = 0;; iPluginIndex++)
//            {
//                ptrPlug = pfDescriptorFunction (iPluginIndex);
//                if (ptrPlug != NULL)
//                    break;
//            }
//
//            if (ptrPlug == 0)
//            {
//                printf ("Cannot find any valid descriptor in shared library \n");
//                return false;
//            }
//        }
//        else
//        {
//            // plugin raised an exception
//            printf ("Cannot find any valid plugin in shared library \n");
//            return false;
//        }
//    }
//    else
//    {
//        printf ("You are trying to load a shared library ? \n");
//        return false;
//    }
//
//    jassert (ptrPlug);
//
//    pluginFile = filePath;
//    ladspa = ptrPlug->LADSPA_Plugin;
//    // version = ptrPlug->DSSI_API_Version;
//
//    plugin = ladspa->instantiate (ladspa, (unsigned int) samplingRate);
//
///*
//    if (ptrPlug->configure)
//        ptrPlug->configure (DSSI_PROJECT_DIRECTORY_KEY, (const char*) filePath.getParentDirectory()
//                                                                              .getFullPathName ());
//*/
//
//    // count ports
//    ins.clear ();
//    outs.clear ();
//    pars.clear ();
//    
//    for (uint i = 0; i < ladspa->PortCount; i++)
//    {
//        LADSPA_PortDescriptor pod = ladspa->PortDescriptors[i];
//    
//        if (pod & LADSPA_PORT_AUDIO)
//        {
//            if (pod & LADSPA_PORT_INPUT)         ins.add (i);
//            else if (pod & LADSPA_PORT_OUTPUT)   outs.add (i);
//        }
//        else if (pod & LADSPA_PORT_CONTROL)
//        {
//            pars.add (i);
//        }
//    }
//    
//    // create ports
//    int numParams = pars.size ();
//    params = new float [numParams];
//    normalized = new float [numParams];
//    memset (params, 0, numParams * sizeof (float));
//    memset (normalized, 0, numParams * sizeof (float));
//
//    // connect ports
//    if (ladspa->connect_port)
//    {
//        for (int i = 0; i < pars.size (); i++)
//            ladspa->connect_port (plugin, pars [i], &normalized[i]);
//        for (int i = 0; i < ins.size (); i++)
//            ladspa->connect_port (plugin, ins [i], emptyBuffer.getSampleData (0));
//        for (int i = 0; i < outs.size (); i++)
//            ladspa->connect_port (plugin, outs [i], emptyBuffer.getSampleData (0));
//    }
//
//    // count programs
//    if (ptrPlug->get_program)
//        for (numPrograms = 0; ptrPlug->get_program (plugin, numPrograms); ++numPrograms);
//
//    // set default to 0
////    setCurrentProgram (0);
//    setDefaultProgram ();
//
//    // start osc listener to itself
//    osc.setPort (18910);
//    osc.setRootAddress ("dssi");
//    osc.addListener (this);
//    osc.startListening ();
//
//    DBG ("WrappedJucePlugin::loadPluginFromFile");
//
//    // create params
//    setNumParameters (numParams);
//
//    for (int i = 0; i < numParams; i++)
//    {
//        AudioParameter* parameter = new AudioParameter ();
//        
//        parameter->part (i);
//        parameter->name (ladspa->PortNames [pars [i]]);
//        parameter->get (MakeDelegate (this, &WrappedJucePlugin::getParameterReal));
//        parameter->set (MakeDelegate (this, &WrappedJucePlugin::setParameterReal));
//        parameter->text (MakeDelegate (this, &WrappedJucePlugin::getParameterTextReal));
//        
//        registerParameter (i, parameter);
//    }
//
//    return true;
//}

bool WrappedJucePlugin::loadPluginStuff ()
{
   if (instance)
   {
      int numParams = instance->getNumParameters();
    setNumParameters (numParams);
    for (int i=0; i<numParams; i++)
    {
      AudioParameter* parameter = new AudioParameter ();
        parameter->part (i);
        parameter->name (instance->getParameterName(i));
        parameter->get (MakeDelegate (this, &WrappedJucePlugin::getParameterReal));
        parameter->set (MakeDelegate (this, &WrappedJucePlugin::setParameterReal));
        parameter->text (MakeDelegate (this, &WrappedJucePlugin::getParameterTextReal));
      registerParameter(i, parameter);
    }

   }
   
   return true;
}

//==============================================================================
const String WrappedJucePlugin::getName () const
{
   return pluginDescription.name;
}

int WrappedJucePlugin::getID () const
{
    return pluginDescription.uid;
}


int WrappedJucePlugin::getNumInputs () const
{
    return pluginDescription.numInputChannels;
}

int WrappedJucePlugin::getNumOutputs () const
{
    return pluginDescription.numOutputChannels;
}

int WrappedJucePlugin::getNumMidiInputs () const
{
//    return acceptsMidi() ? 1 : 0;
   return 1; // we always accept midi, so plugin parameters can be automated even if plugin personally does not care for midi
}

int WrappedJucePlugin::getNumMidiOutputs () const
{
    return instance && instance->producesMidi() ? 1 : 0;
}

void* WrappedJucePlugin::getLowLevelHandle ()
{
    return 0;
}

bool WrappedJucePlugin::acceptsMidi () const
{
    return instance && instance->acceptsMidi();
}

//==============================================================================
void WrappedJucePlugin::prepareToPlay (double sampleRate, int samplesPerBlock)
{
   DBG ("WrappedJucePlugin::prepareToPlay");
   if (instance)
   {
      instance->prepareToPlay(sampleRate, samplesPerBlock);
      instance->setPlayHead(getParentHost()->getTransport());
   }
}

void WrappedJucePlugin::releaseResources()
{
    DBG ("WrappedJucePlugin::releaseResources");
    if (instance)
      instance->releaseResources();
}

//==============================================================================
void WrappedJucePlugin::processBlock (AudioSampleBuffer& buffer,
                               MidiBuffer& midiMessages)
{
    const int blockSize = buffer.getNumSamples ();    

    if (instance)
    {
      // Juce plugins put their input into (passed in) buffer, so we need to copy this out from the Jost inputBuffer
      for (int i = 0; i < getNumInputs(); i++)
            buffer.copyFrom(i, 0, *inputBuffer, i, 0, buffer.getNumSamples());

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
      
      // apply a midi filter on the input to the synth if one is set
      MidiFilter* synthInputFilter = getSynthInputChannelFilter();
      if (synthInputFilter)
      {
         MidiManipulator manip;
         manip.setMidiFilter(synthInputFilter);
         manip.processEvents(*midiBuffer, blockSize);
      }
      
      // Call through to Juce plugin instance to get the VST to actually do its thing!
      instance->processBlock(buffer, *midiBuffer);

      // haven't worked out what jiggerying needs to be done to the midi output yet

      // Juce plugins put their output into (passed in) buffer, so we need to copy this out into the Jost outputBuffer
      for (int i = 0; i < getNumOutputs(); i++)
            outputBuffer->copyFrom(i, 0, buffer, i, 0, buffer.getNumSamples());
   }

}

//==============================================================================
int WrappedJucePlugin::getNumParameters()
{
   return instance->getNumParameters();
}

void WrappedJucePlugin::setParameterReal (int index, float value)
{
   if (instance)
      instance->setParameter(index, value);
}

float WrappedJucePlugin::getParameterReal (int index)
{
   float val = 0.0;
   if (instance)
      val = instance->getParameter(index);
    return val;
}

const String WrappedJucePlugin::getParameterTextReal (int index, float value)
{
   String valText;
   if (instance)
      valText = instance->getParameterText(index);
    return valText;
}

//==============================================================================
int WrappedJucePlugin::getNumPrograms ()
{
    return 0;
}

void WrappedJucePlugin::setCurrentProgram (int programNumber)
{

}

int WrappedJucePlugin::getCurrentProgram ()
{
    return 0;
}

const String WrappedJucePlugin::getProgramName (const int programNumber)
{
    return T("Default preset");
}

const String WrappedJucePlugin::getCurrentProgramName ()
{
    return getProgramName (0);
}

//==============================================================================
bool WrappedJucePlugin::hasEditor () const
{
    return true; // tricky answer to a tricky question!
}

bool WrappedJucePlugin::wantsEditor () const
{
    return false;
}


//==============================================================================
void WrappedJucePlugin::savePresetToXml(XmlElement* element)
{
   if (instance)
   {
      MemoryBlock pluginState;
      instance->getStateInformation(pluginState);

      XmlElement* chunk = new XmlElement (T("juceVSTPluginData"));
      chunk->addTextElement (pluginState.toBase64Encoding ());
      element->addChildElement (chunk);
   }
}

void WrappedJucePlugin::loadPresetFromXml(XmlElement* element)
{
   if (instance)
   {
      XmlElement* chunk = element->getChildByName (T("juceVSTPluginData"));
      if (chunk)
      {
         MemoryBlock mb;
         mb.fromBase64Encoding (chunk->getAllSubText ());
         instance->setStateInformation (mb.getData(), mb.getSize ());
      }
   }
}

