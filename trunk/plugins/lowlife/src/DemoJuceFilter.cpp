/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "includes.h"
#include "JucePluginCharacteristics.h"
#include "DemoJuceFilter.h"
#include "LowlifeEditorComponent.h"

//==============================================================================
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DemoJuceFilter();
}

//==============================================================================
DemoJuceFilter::DemoJuceFilter()
{
    lastUIWidth = 400;
    lastUIHeight = 300;
    
    getHProgramRef().ply_mode = 2;
}

DemoJuceFilter::~DemoJuceFilter()
{
}

//==============================================================================
const String DemoJuceFilter::getName() const
{
    return JucePlugin_Name;
}

const int DemoJuceFilter::MinKey = 0;
const int DemoJuceFilter::MaxKey = 127;
const int DemoJuceFilter::MinFader = -60;
const int DemoJuceFilter::MaxFader = 3;
const int DemoJuceFilter::MinTune = 0;
const int DemoJuceFilter::MaxTune = 100;
const int DemoJuceFilter::MinPolyMode = 0;
const int DemoJuceFilter::MaxPolyMode = 2;
const int DemoJuceFilter::MinSyncTicks = 0;
const int DemoJuceFilter::MaxSyncTicks = 999; // ticks are 16th notes

float DemoJuceFilter::getNormedParam(int slot, int paramId)
{
   float paramVal = 0;
   switch (paramId) 
   {
      case KeyLow:
         paramVal = ((getZoneslotKeyMin(slot) - MinKey) +1) / static_cast<float>(1 + MaxKey - MinKey);
         break;
      case KeyCentre:
         paramVal = ((getZoneslotKeyCentre(slot) - MinKey) +1) / static_cast<float>(1 + MaxKey - MinKey);
         break;
      case KeyHigh:
         paramVal = ((getZoneslotKeyMax(slot) - MinKey) + 1) / static_cast<float>(1 + MaxKey - MinKey);
         break;
      case Tune:
         paramVal = (getZoneslotTuneFactor(slot) - MinTune) / static_cast<float>(MaxTune - MinTune);
         break;
      case Fader:
         paramVal = (getZoneslotFader(slot) - MinFader) / static_cast<float>(MaxFader - MinFader);
         break;
      case BPMSync:
         paramVal = (getZoneslotBPMSync(slot) > 0.5);
         break;
      case SyncTicks:
         paramVal = (getZoneslotSyncTicks(slot) - MinSyncTicks) / static_cast<float>(MaxSyncTicks - MinSyncTicks);
         break;
   }
   return paramVal;
}

void DemoJuceFilter::setNormedParam(int slot, int paramId, float val)
{
   switch (paramId) 
   {
      case KeyLow:
         setZoneslotKeyMin(slot, (val * (1 + MaxKey - MinKey)) + MinKey);
         break;
      case KeyCentre:
         setZoneslotKeyCentre(slot, (val * (1 + MaxKey - MinKey)) + MinKey);
         break;
      case KeyHigh:
         setZoneslotKeyMax(slot, (val * (1 + MaxKey - MinKey)) + MinKey);
         break;
      case Tune:
         setZoneslotTuneFactor(slot, (val * (1 + MaxTune - MinTune)) + MinTune);
         break;
      case Fader:
         setZoneslotFader(slot, (val * (MaxFader - MinFader)) + MinFader);
         break;
      case BPMSync:
         setZoneslotBPMSync(slot, (val > 0.5));
         break;
      case SyncTicks:
         setZoneslotSyncTicks(slot, (val * (MaxSyncTicks - MinSyncTicks)) + MinSyncTicks);
         break;
      default:
         break;
   }

}

int DemoJuceFilter::getNumParameters()
{
   int numParams = 0;
   
   numParams = getMaxZoneslots() * paramsPerSlot;
   
   return numParams;
}

float DemoJuceFilter::getParameter (int index)
{
   int slotOfConcern = index / paramsPerSlot;
   int paramOfInterest = index % paramsPerSlot;

   return getNormedParam(slotOfConcern, paramOfInterest);
}

void DemoJuceFilter::setParameter (int index, float newValue)
{
   int slotOfConcern = index / paramsPerSlot;
   int paramOfInterest = index % paramsPerSlot;

   setNormedParam(slotOfConcern, paramOfInterest, newValue);

   sendChangeMessage(this);
}

const String DemoJuceFilter::getParameterName (int index)
{
   int slotOfConcern = index / paramsPerSlot;
   int paramOfInterest = index % paramsPerSlot;

   String slotName = String("Slot") + String(slotOfConcern+1) + (" ");

   switch (paramOfInterest)
   {
      case KeyLow:
         slotName += String("Low key");
      break;
      case KeyCentre:
         slotName += String("Base/centre key");
      break;
      case KeyHigh:
         slotName += String("High key");
      break;
      case Fader:
         slotName += String("Fader");
      break;
      case Tune:
         slotName += String("Tuniness");
      break;
      case BPMSync:
         slotName += String("BPM Sync");
      break;
      case SyncTicks:
         slotName += String("Sample length in 16ths (for BPM sync)");
      break;
   }

   return slotName;
}

const String DemoJuceFilter::getParameterText (int index)
{
   int slot = index / paramsPerSlot;
   int paramOfInterest = index % paramsPerSlot;
   String paramVal;
   switch (paramOfInterest) 
   {
      case KeyLow:
         paramVal = String(getZoneslotKeyMin(slot));
         break;
      case KeyCentre:
         paramVal = String(getZoneslotKeyCentre(slot));
         break;
      case KeyHigh:
         paramVal = String(getZoneslotKeyMax(slot));
         break;
      case Tune:
         paramVal = String(getZoneslotTuneFactor(slot));
         break;
      case Fader:
         paramVal = String(getZoneslotFader(slot));
         break;
      case BPMSync:
         paramVal = String(getZoneslotBPMSync(slot));
         break;
      case SyncTicks:
         paramVal = String(getZoneslotSyncTicks(slot));
         break;
   }

    return paramVal;
}

int DemoJuceFilter::getNumZoneslots()
{
   int zoneslots = 1;
   zoneslots = getHProgramRef().num_zones;
   return zoneslots;
}

void DemoJuceFilter::setNumZoneslots(int newslots)
{
   int oldslots = getNumZoneslots();
   if (newslots > oldslots)
   {
      // making new zoneslots
      for (int zoneslotCount=oldslots; zoneslotCount<newslots; zoneslotCount++)
      {
         HIGHLIFE_PROGRAM* pprg = getHProgram(zoneslotCount);
         if (pprg)
            highlifeInstance.tool_alloc_zone(pprg);
      }
      sendChangeMessage(this);
   }
   else if (newslots < oldslots)
   {
      // killing zoneslots
      for (int zoneslotCount=oldslots-1; zoneslotCount>=newslots; zoneslotCount--)
      {
         HIGHLIFE_PROGRAM* pprg = getHProgram(zoneslotCount);
         HIGHLIFE_ZONE* pz = getHZone(zoneslotCount);
         if (pprg && pz)
            highlifeInstance.tool_init_program(pprg); // zeroes out the prog so we know that it is not in use
      }
      sendChangeMessage(this);
   }
}

File DemoJuceFilter::getZoneslotSample(int slot)
{
   HIGHLIFE_ZONE* zo = getHZone(slot);
   File samfile;
   if (zo)
      samfile = File(zo->path);
   return samfile;
}

void DemoJuceFilter::setZoneslotSample(int slot, const File sampleFile)
{
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
   {
      highlifeInstance.tool_delete_wave (pz);
      highlifeInstance.tool_load_sample(pz, sampleFile);
   }
   sendChangeMessage (this);
}

int DemoJuceFilter::getPolyMode()
{
   int polymode = 0;
   HIGHLIFE_PROGRAM* pz = getHProgram();
   if (pz)
      polymode = pz->ply_mode;
   if (polymode < MinPolyMode || polymode > MaxPolyMode)
      polymode = MaxPolyMode;

   return polymode;
}

void DemoJuceFilter::setPolyMode(int poly)
{
   if (poly < MinPolyMode || poly > MaxPolyMode)
      poly = MaxPolyMode;

   HIGHLIFE_PROGRAM* pz = getHProgram();
   if (pz)
      pz->ply_mode = poly;
   sendChangeMessage (this);
}

int DemoJuceFilter::getZoneslotFader(int slot)
{
   int key = 0;
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      key = pz->mp_gain;
   return key;
}

void DemoJuceFilter::setZoneslotFader(int slot, int gain)
{
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      pz->mp_gain = gain;
   sendChangeMessage (this);
}

int DemoJuceFilter::getZoneslotKeyMin(int slot)
{
   int key = 0;
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      key = pz->lo_input_range.midi_key;
   return key;

}

void DemoJuceFilter::setZoneslotKeyMin(int slot, int keymin)
{
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      pz->lo_input_range.midi_key = keymin;
   sendChangeMessage (this);
}

int DemoJuceFilter::getZoneslotKeyCentre(int slot)
{
   int key = 0;
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      key = pz->midi_root_key;
   return key;
}

void DemoJuceFilter::setZoneslotKeyCentre(int slot, int keymin)
{
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      pz->midi_root_key = keymin;
   sendChangeMessage (this);
}

int DemoJuceFilter::getZoneslotKeyMax(int slot)
{
   int key = 0;
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      key = pz->hi_input_range.midi_key;
   return key;
}

void DemoJuceFilter::setZoneslotKeyMax(int slot, int keym)
{
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      pz->hi_input_range.midi_key = keym;
   sendChangeMessage (this);
}

int DemoJuceFilter::getZoneslotTuneFactor(int slot)
{
   int key = 0;
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      key = pz->midi_keycents;
   return key;
}

void DemoJuceFilter::setZoneslotTuneFactor(int slot, int fac)
{
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      pz->midi_keycents = fac;
   sendChangeMessage (this);
}

bool DemoJuceFilter::getZoneslotBPMSync(int slot)
{
   bool syn = 0;
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      syn = pz->mp_synchro;
   return syn;
}

void DemoJuceFilter::setZoneslotBPMSync(int slot, bool fac)
{
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      pz->mp_synchro = fac;
   sendChangeMessage (this);
}

int DemoJuceFilter::getZoneslotSyncTicks(int slot)
{
   int tic = 0;
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      tic = pz->mp_num_ticks;
   return tic;
}

void DemoJuceFilter::setZoneslotSyncTicks(int slot, int tic)
{
   HIGHLIFE_ZONE* pz = getHZone(slot);
   if (pz)
      pz->mp_num_ticks = tic;
   sendChangeMessage (this);
}

const String DemoJuceFilter::getInputChannelName (const int channelIndex) const
{
    return String(channelIndex + 1);
}

const String DemoJuceFilter::getOutputChannelName (const int channelIndex) const
{
    return String(channelIndex + 1);
}

bool DemoJuceFilter::isInputChannelStereoPair (int index) const
{
    return false;
}

bool DemoJuceFilter::isOutputChannelStereoPair (int index) const
{
    return false;
}

bool DemoJuceFilter::acceptsMidi() const
{
    return true;
}

bool DemoJuceFilter::producesMidi() const
{
    return true;
}

//==============================================================================
void DemoJuceFilter::prepareToPlay (double sampleRate, int samplesPerBlock)
{
       highlifeInstance.prepareToPlay(sampleRate, samplesPerBlock);
       highlifeInstance.setPlayConfigDetails(0, 1, sampleRate, samplesPerBlock);
}

void DemoJuceFilter::releaseResources()
{
    // when playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

void DemoJuceFilter::processBlock (AudioSampleBuffer& buffer,
                                   MidiBuffer& midiMessages)
{
   highlifeInstance.setPlayHead(getPlayHead());
   highlifeInstance.processBlock(buffer, midiMessages);
}

//==============================================================================
AudioProcessorEditor* DemoJuceFilter::createEditor()
{
    return new LowlifeEditorComponent (this);
}

//==============================================================================

/*// magic number to identify memory blocks that we've stored as XML
const uint32 magicXmlNumber = 0x21324356;

int ruaGetXmlToBinaryLen (const XmlElement& xml)
{
    const String xmlString (xml.createDocument (String::empty, true, false));
    const int stringLength = xmlString.length();
   return stringLength + 8;
}

void ruaCopyXmlToBinary (const XmlElement& xml,
                                      MemoryBlock& destData)
{
    const String xmlString (xml.createDocument (String::empty, true, false));
    const int stringLength = xmlString.length();

    destData.setSize (stringLength + 10);

    char* const d = (char*) destData.getData();
    *(uint32*) d = swapIfBigEndian ((const uint32) magicXmlNumber);
    *(uint32*) (d + 4) = swapIfBigEndian ((const uint32) stringLength);

    xmlString.copyToBuffer (d + 8, stringLength);
}

XmlElement* ruaGetXmlFromBinary (const void* data,
                                              const int sizeInBytes)
{
    if (sizeInBytes > 8
         && littleEndianInt ((const char*) data) == magicXmlNumber)
    {
        const uint32 stringLength = littleEndianInt (((const char*) data) + 4);

        if (stringLength > 0)
        {
            XmlDocument doc (String (((const char*) data) + 8,
                                     jmin ((sizeInBytes - 8), stringLength)));

            return doc.getDocumentElement();
        }
    }

    return 0;
}
*/
void DemoJuceFilter::getStateInformation (MemoryBlock& highLifeState)
{
    // you can store your parameters as binary data if you want to or if you've got
    // a load of binary to put in there, but if you're not doing anything too heavy,
    // XML is a much cleaner way of doing it - here's an example of how to store your
    // params as XML..

//    // create an outer XML element..
//    XmlElement xmlState (T("MYPLUGINSETTINGS"));
//
//    // add some attributes to it..
//    xmlState.setAttribute (T("pluginVersion"), 1);
//    xmlState.setAttribute (T("gainLevel"), gain);
//    xmlState.setAttribute (T("uiWidth"), lastUIWidth);
//    xmlState.setAttribute (T("uiHeight"), lastUIHeight);
//    
//
   
      // for now, we are not saving our real params, but we are saving the highlife state _instead_
      // this is because I am too lazy _right_now_ to deal with xml + binary blob issues!
      // really what is important now is tweaking the save/load of the highlife state so it doesn't save sample data 
      // (just saves file path)
      // when this is crankin, we can sort out saving our own params (if we have any)..
      // ..or only save the parameters from highlife that we want to save, via xml
      // MemoryBlock::toBase64Encoding would do it ;)
      
    // you could also add as many child elements as you need to here..
//    MemoryBlock highLifeState;
   //for (int i=0;i<getMaxZoneslots(); i++)
    highlifeInstance.getStateInformation(highLifeState);

    // then use this helper function to stuff it into the binary blob and return it..
//    copyXmlToBinary (xmlState, highLifeState);
}

void DemoJuceFilter::setStateInformation (const void* data, int sizeInBytes)
{
   setNumZoneslots(0);
   //for (int i=0;i<getMaxZoneslots(); i++)
    highlifeInstance.setStateInformation(data, sizeInBytes);

 //   // use this helper function to get the XML from this binary blob..
//    XmlElement* const xmlState = getXmlFromBinary (data, sizeInBytes);
//
//    if (xmlState != 0)
//    {
//        // check that it's the right type of xml..
//        if (xmlState->hasTagName (T("MYPLUGINSETTINGS")))
//        {
//            // ok, now pull out our parameters..
//            gain = (float) xmlState->getDoubleAttribute (T("gainLevel"), gain);
//
//            lastUIWidth = xmlState->getIntAttribute (T("uiWidth"), lastUIWidth);
//            lastUIHeight = xmlState->getIntAttribute (T("uiHeight"), lastUIHeight);
//
//            sendChangeMessage (this);
//        }
//
//        delete xmlState;
//    }
   
   // count the zoneslots in the loaded file
//   jassert(zoneslotCount == 0);
//   for (zoneslotCount=0; zoneslotCount<NUM_PROGRAMS; zoneslotCount++)
//      if (getHProgramRef(zoneslotCount).num_zones <= 0)
//         break;

   sendChangeMessage(this);
}
