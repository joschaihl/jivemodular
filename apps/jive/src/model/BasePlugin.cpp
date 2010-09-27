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

#include "BasePlugin.h"

//==============================================================================
#define MIDIBINDINGS_ELEMENT_NAME              T("midiBindings")
#define MIDIBINDING_ELEMENT_NAME              T("binding")
#define MIDIBINDING_PARAM_ATTRIB              T("parameterId")
#define MIDIBINDING_TRIGVAL_ATTRIB              T("triggerNoteOrCC")
#define MIDIBINDING_MODE_ATTRIB              T("bindingMode")
#define MIDIBINDING_RANGEMAX_ATTRIB              T("rangeMax")
#define MIDIBINDING_RANGEMIN_ATTRIB              T("rangeMin")
#define MIDIBINDING_INCRMAX_ATTRIB              T("incrementMax")
#define MIDIBINDING_INCRMIN_ATTRIB              T("incrementMin")
#define MIDIBINDING_INCREMENT_ATTRIB              T("incrValue")
#define MIDIBINDING_STEPMODE_ATTRIB              T("stepMode")
#define MIDIBINDING_VELOCITYSENSE_ATTRIB              T("velocitySensitivity")

//==============================================================================
int32 BasePlugin::globalUniqueCounter = 1;

//==============================================================================
BasePlugin::BasePlugin ()
    : PropertySet (false), // do not ignore case of key names
      uniqueHash (BasePlugin::globalUniqueCounter++),
      parentHost (0),
      mutedOutput (false),
      bypassOutput (false),
      outputGain (1.0f),
      currentOutputGain (1.0f),
      stemFileWriter(0),
      outputMidiChannel(-1),
      synthInputMidiChan(-1),
      outputMidiChanFilter(0),
      synthInputMidiChanFilter(0)
{
    keyboardState.reset();

    // default
    setValue (PROP_MIXERPEAK, 1);
    setValue (PROP_MIXERMETERON, 1);
    
    // must we set this here ?
    setParametersChangeChecksPerSecond (50);
}

BasePlugin::~BasePlugin ()
{
    closeStemFile();

    clearMidiOutputFilter();
}

String BasePlugin::getInstanceName()
{
   return getValue(PROP_GRAPHNAME, getName ());
}

void BasePlugin::setInstanceName(const String& instanceName)
{
   setValue(PROP_GRAPHNAME, instanceName);
}

//==============================================================================
void BasePlugin::savePropertiesToXml (XmlElement* xml)
{
    xml->setAttribute (PROP_GRAPHSELECTED,           getIntValue (PROP_GRAPHSELECTED, 0));
    xml->setAttribute (PROP_GRAPHLOCKED,             getIntValue (PROP_GRAPHLOCKED, 0));
    xml->setAttribute (PROP_GRAPHCOLOUR,             getValue (PROP_GRAPHCOLOUR, T("0xff808080")));
    xml->setAttribute (PROP_GRAPHNAME,               getInstanceName());
    xml->setAttribute (PROP_GRAPHXPOS,               getIntValue (PROP_GRAPHXPOS, -1));
    xml->setAttribute (PROP_GRAPHYPOS,               getIntValue (PROP_GRAPHYPOS, -1));
    xml->setAttribute (PROP_GRAPHWSIZE,              getIntValue (PROP_GRAPHWSIZE, -1));
    xml->setAttribute (PROP_GRAPHHSIZE,              getIntValue (PROP_GRAPHHSIZE, -1));
    xml->setAttribute (PROP_WINDOWXPOS,              getIntValue (PROP_WINDOWXPOS, -1));
    xml->setAttribute (PROP_WINDOWYPOS,              getIntValue (PROP_WINDOWYPOS, -1));
    xml->setAttribute (PROP_WINDOWWSIZE,             getIntValue (PROP_WINDOWWSIZE, -1));
    xml->setAttribute (PROP_WINDOWHSIZE,             getIntValue (PROP_WINDOWHSIZE, -1));
    xml->setAttribute (PROP_WINDOWPAGE,              getIntValue (PROP_WINDOWPAGE, 0));
    xml->setAttribute (PROP_WINDOWOPEN,              getIntValue (PROP_WINDOWOPEN, 0));
    xml->setAttribute (PROP_WINDOWVISIBLEMIDIKEY,    getIntValue (PROP_WINDOWVISIBLEMIDIKEY, 1));
    xml->setAttribute (PROP_PLUGPRESETDIR,           getValue (PROP_PLUGPRESETDIR, String::empty));
    xml->setAttribute (PROP_MIXERLABEL,              getValue (PROP_MIXERLABEL, String::empty));
    xml->setAttribute (PROP_MIXERINDEX,              getIntValue (PROP_MIXERINDEX, 0));
    xml->setAttribute (PROP_MIXERNARROW,             getIntValue (PROP_MIXERNARROW, 0));
    xml->setAttribute (PROP_MIXERPEAK,               getIntValue (PROP_MIXERPEAK, 1));
    xml->setAttribute (PROP_MIXERMETERON,            getIntValue (PROP_MIXERMETERON, 1));
    xml->setAttribute (PROP_WINDOWPREFERGENERIC,            getBoolValue (PROP_WINDOWPREFERGENERIC, false));
    xml->setAttribute (PROP_RENDERSTEM,            getBoolValue (PROP_RENDERSTEM, false));
    xml->setAttribute (PROP_SYNTHINPUTCHANNELFILTER, getSynthInputChannel());
    xml->setAttribute (PROP_OUTPUTCHANNELFILTER, getMidiOutputChannel());
   
   XmlElement* bindingsElement = new XmlElement(MIDIBINDINGS_ELEMENT_NAME);
   for (int i=0; i<getNumParameters(); i++)
   {
      AudioParameter* param = getParameterObject(i);
      if (param)
      {
         for (int j=0; j<param->getNumBindings(); j++)
         {
            MidiBinding* bp = param->getBinding(j);
            if (bp)
            {
               XmlElement* binding = new XmlElement(MIDIBINDING_ELEMENT_NAME);

               binding->setAttribute(MIDIBINDING_PARAM_ATTRIB, i);
               binding->setAttribute(MIDIBINDING_TRIGVAL_ATTRIB, bp->getTriggerValue());
               binding->setAttribute(MIDIBINDING_MODE_ATTRIB, bp->getMode());
               binding->setAttribute(MIDIBINDING_RANGEMAX_ATTRIB, bp->getMax());
               binding->setAttribute(MIDIBINDING_RANGEMIN_ATTRIB, bp->getMin());
               binding->setAttribute(MIDIBINDING_INCRMAX_ATTRIB, bp->getIncrMax());
               binding->setAttribute(MIDIBINDING_INCRMIN_ATTRIB, bp->getIncrMin());
               binding->setAttribute(MIDIBINDING_INCREMENT_ATTRIB, fabs(bp->getIncrAmount()));
               binding->setAttribute(MIDIBINDING_STEPMODE_ATTRIB, bp->getStepMode());
               binding->setAttribute(MIDIBINDING_VELOCITYSENSE_ATTRIB, bp->getVelocityScaling());

               bindingsElement->addChildElement(binding);
            }
         }
      }
   }
   xml->addChildElement(bindingsElement);
}

void BasePlugin::loadPropertiesFromXml (XmlElement* xml)
{
    setValue (PROP_GRAPHSELECTED,                    xml->getIntAttribute (PROP_GRAPHSELECTED, 0));
    setValue (PROP_GRAPHLOCKED,                      xml->getIntAttribute (PROP_GRAPHLOCKED, 0));
    setValue (PROP_GRAPHCOLOUR,                      xml->getStringAttribute (PROP_GRAPHCOLOUR, T("0xff808080")));
    setInstanceName (xml->getStringAttribute (PROP_GRAPHNAME, getInstanceName()));
    setValue (PROP_GRAPHXPOS,                        xml->getIntAttribute (PROP_GRAPHXPOS, -1));
    setValue (PROP_GRAPHYPOS,                        xml->getIntAttribute (PROP_GRAPHYPOS, -1));
    setValue (PROP_GRAPHWSIZE,                       xml->getIntAttribute (PROP_GRAPHWSIZE, 50));
    setValue (PROP_GRAPHHSIZE,                       xml->getIntAttribute (PROP_GRAPHHSIZE, 50));
    setValue (PROP_WINDOWXPOS,                       xml->getIntAttribute (PROP_WINDOWXPOS, -1));
    setValue (PROP_WINDOWYPOS,                       xml->getIntAttribute (PROP_WINDOWYPOS, -1));
    setValue (PROP_WINDOWWSIZE,                      xml->getIntAttribute (PROP_WINDOWWSIZE, -1));
    setValue (PROP_WINDOWHSIZE,                      xml->getIntAttribute (PROP_WINDOWHSIZE, -1));
    setValue (PROP_WINDOWPAGE,                       xml->getIntAttribute (PROP_WINDOWPAGE, 0));
    setValue (PROP_WINDOWOPEN,                       xml->getIntAttribute (PROP_WINDOWOPEN, 0));
    setValue (PROP_WINDOWVISIBLEMIDIKEY,             xml->getIntAttribute (PROP_WINDOWVISIBLEMIDIKEY, 1));
    setValue (PROP_PLUGPRESETDIR,                    xml->getStringAttribute (PROP_PLUGPRESETDIR, String::empty));
    setValue (PROP_MIXERLABEL,                       xml->getStringAttribute (PROP_MIXERLABEL, String::empty));
    setValue (PROP_MIXERINDEX,                       xml->getIntAttribute (PROP_MIXERINDEX, 0));
    setValue (PROP_MIXERNARROW,                      xml->getIntAttribute (PROP_MIXERNARROW, 0));
    setValue (PROP_MIXERPEAK,                        xml->getIntAttribute (PROP_MIXERPEAK, 1));
    setValue (PROP_MIXERMETERON,                     xml->getIntAttribute (PROP_MIXERMETERON, 1));
    setValue (PROP_WINDOWPREFERGENERIC,                     xml->getBoolAttribute (PROP_WINDOWPREFERGENERIC, false));
    setValue (PROP_RENDERSTEM,                     xml->getBoolAttribute (PROP_RENDERSTEM, false));

    setSynthInputChannelFilter(xml->getIntAttribute(PROP_SYNTHINPUTCHANNELFILTER, -1));
    setMidiOutputChannelFilter(xml->getIntAttribute(PROP_OUTPUTCHANNELFILTER, -1));
    
   XmlElement* bindingsElement = xml->getChildByName(MIDIBINDINGS_ELEMENT_NAME);

   if (bindingsElement)
   {
      forEachXmlChildElementWithTagName(*bindingsElement, bindingElement, MIDIBINDING_ELEMENT_NAME)
      {
         int paramid = bindingElement->getIntAttribute(MIDIBINDING_PARAM_ATTRIB, -1);
         int trigval = bindingElement->getIntAttribute(MIDIBINDING_TRIGVAL_ATTRIB, -1);
         int mode = bindingElement->getIntAttribute(MIDIBINDING_MODE_ATTRIB, -1);
         AudioParameter* param = getParameterObject(paramid);
         if (param && trigval >= 0 && trigval <= 127 && mode >= NoteOff && mode <= Controller && paramid >= 0 && paramid <= getNumParameters())
         {
            int dex = -1;
            if (mode == Controller)
               dex = param->addControllerNumber(trigval);
            else
               dex = param->addNoteNumber(trigval);
            MidiBinding* bp = param->getBinding(dex);
            if (bp)
            {
               bp->setMode(NoteBindingMode(mode));
               bp->setMax(bindingElement->getDoubleAttribute(MIDIBINDING_RANGEMAX_ATTRIB, 1.0));
               bp->setMin(bindingElement->getDoubleAttribute(MIDIBINDING_RANGEMIN_ATTRIB, 0.0));
               bp->setStepMax(bindingElement->getDoubleAttribute(MIDIBINDING_INCRMAX_ATTRIB, 1.0));
               bp->setStepMin(bindingElement->getDoubleAttribute(MIDIBINDING_INCRMIN_ATTRIB, 0.0));
               bp->setIncrAmount(bindingElement->getDoubleAttribute(MIDIBINDING_INCREMENT_ATTRIB, 1.0));
               bp->setStepMode(BindingStepMode(bindingElement->getIntAttribute(MIDIBINDING_STEPMODE_ATTRIB, 0)));
               bp->setVelocityScaling(bindingElement->getDoubleAttribute(MIDIBINDING_VELOCITYSENSE_ATTRIB, 0.0));
               
               param->RegisterBinding(dex);
            }
         }
         
      }
   }
}

//==============================================================================
void BasePlugin::openStemFile(String uniquePrefix, int sampleRate)
{
   String stemFname(uniquePrefix);
   stemFname += String("__");
   stemFname += getInstanceName();
   stemFname += String(".wav");
   
   FileOutputStream* outputStream = new FileOutputStream(
      File(Config::getInstance ()->lastStemsDirectory).getChildFile(stemFname));
         
   StringPairArray metadata;
   if (getNumOutputs())
      stemFileWriter = WavAudioFormat().createWriterFor (outputStream,
                                        sampleRate,
                                        getNumOutputs(),
                                        32, // aka floating point, good paranoid format for potentially clippy plugins!
                                        metadata,
                                        0);   
}

void BasePlugin::closeStemFile()
{
   delete stemFileWriter;
   stemFileWriter = 0;
}

void BasePlugin::renderBlock(AudioSampleBuffer& buffer)
{
   if (stemFileWriter)
      buffer.writeToAudioWriter(stemFileWriter, 0, buffer.getNumSamples());
}

//==============================================================================
void BasePlugin::setMidiOutputChannelFilter(int midiChannel)
{
   clearMidiOutputFilter();
   
   if (midiChannel >= 1 && midiChannel <= 16)
   {
      outputMidiChanFilter = new MidiFilter();
      outputMidiChanFilter->setUseChannelFilter(true);
      outputMidiChanFilter->clearAllChannels();
      outputMidiChanFilter->setChannel(midiChannel);
      outputMidiChannel = midiChannel;
   }
}

MidiFilter* BasePlugin::getMidiOutputChannelFilter()
{
   return outputMidiChanFilter;
}

void BasePlugin::clearMidiOutputFilter()
{
   if (outputMidiChanFilter)
      delete outputMidiChanFilter;
   outputMidiChannel = -1;
}

void BasePlugin::setSynthInputChannelFilter(int midiChannel)
{
   clearSynthInputFilter();
   
   if (midiChannel >= 1 && midiChannel <= 16)
   {
      synthInputMidiChanFilter = new MidiFilter();
      synthInputMidiChanFilter->setUseChannelFilter(true);
      synthInputMidiChanFilter->clearAllChannels();
      synthInputMidiChanFilter->setChannel(midiChannel);
      synthInputMidiChan = midiChannel;
   }
}

MidiFilter* BasePlugin::getSynthInputChannelFilter()
{
   return synthInputMidiChanFilter;
}

void BasePlugin::clearSynthInputFilter()
{
   if (synthInputMidiChanFilter)
      delete synthInputMidiChanFilter;
   synthInputMidiChan = -1;
   synthInputMidiChanFilter = 0;
}

//==============================================================================
void BasePlugin::savePresetToXml (XmlElement* xml)
{
    xml->setAttribute (T("gain"), outputGain);
    xml->setAttribute (T("mute"), mutedOutput);
    xml->setAttribute (T("bypass"), bypassOutput);
    xml->setAttribute (T("outMidiChan"), outputMidiChannel);

    MemoryBlock mb;
    getStateInformation (mb);

    XmlElement* chunk = new XmlElement (T("data"));
    chunk->addTextElement (mb.toBase64Encoding ());
    xml->addChildElement (chunk);

    XmlElement* params = new XmlElement (T("parameters"));
    writeParametersToXmlElement (params);
    xml->addChildElement (params);
}    

void BasePlugin::loadPresetFromXml (XmlElement* xml)
{
    // default vst values
    outputGain =  xml->getDoubleAttribute (T("gain"), 1.0);
    mutedOutput = xml->getBoolAttribute (T("mute"), 0);
    bypassOutput = xml->getBoolAttribute (T("bypass"), 0);
    outputMidiChannel = xml->getIntAttribute (T("outMidiChan"), -1);

    // current preset
    XmlElement* chunk = xml->getChildByName (T("data"));
    if (chunk)
    {
        MemoryBlock mb;
        mb.fromBase64Encoding (chunk->getAllSubText ());
        setStateInformation (mb.getData(), mb.getSize ());
    }

    XmlElement* params = xml->getChildByName (T("parameters"));
    if (params)
    {
        readParametersFromXmlElement (params, false);
    }
}

