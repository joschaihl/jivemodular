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

#include "MidiSequencePluginBase.h"
#include "HostFilterBase.h"
#include "SequenceComponent.h"

#define NOTE_VELOCITY      0.8f
#define NOTE_PREFRAMES     0.001
#define DEFAULT_MAXIMUM_CLIPS 16

double D_getNoteOnIndexed (const MidiMessageSequence* midiSequence, int chanNum = -1, const int index = 0)
{
   int note = 0; float beat = 0.0;
   float length = 0;
    int numNoteOn = 0;
    for (int i = 0; i < midiSequence->getNumEvents (); i++)
    {
        MidiMessageSequence::MidiEventHolder* eventOn = midiSequence->getEventPointer (i);
        MidiMessageSequence::MidiEventHolder* eventOff = midiSequence->getEventPointer (i)->noteOffObject;

        MidiMessage* msgOn = & eventOn->message;

         if (chanNum != -1 && (msgOn->getChannel() != chanNum))
            continue;

        if (eventOn->message.isNoteOn () && eventOff)
        {
            MidiMessage* msgOff = & eventOff->message;
            if (index == numNoteOn)
            {
                note = msgOn->getNoteNumber ();
                beat = msgOn->getTimeStamp ();
                length = ((msgOff->getTimeStamp () + NOTE_PREFRAMES) - msgOn->getTimeStamp ());
                break;
            }
            numNoteOn++;
        }
    }

   return length;
}

int D_getNumNoteOn (const MidiMessageSequence* midiSequence, int chanNum = -1)
{
    int numNoteOn = 0;

    for (int i = 0; i < midiSequence->getNumEvents (); i++)
        if (midiSequence->getEventPointer(i)->message.isNoteOn() && 
            (chanNum == -1 || midiSequence->getEventPointer(i)->message.getChannel() == chanNum)) 
         numNoteOn++;

    return numNoteOn;
}

//==============================================================================
MidiSequencePluginBase::MidiSequencePluginBase ()
  : transport (0),
    midiSequence (0),
    allClipsByChannelSequence(0),
    currentClip(0),
    doAllNotesOff (false),
    allNotesOff (MidiMessage::allNotesOff (1)),
    uptoBeatReloopHack(0.0),
    loopPhaseInBeats(0.0),
    playRate(1.0)
{
   midiSequence = new MidiMessageSequence();
   allClipsByChannelSequence = new MidiMessageSequence();
   
   // our string array needs to be 16 big
   for (int i=0; i<DEFAULT_MAXIMUM_CLIPS; i++)
      clipFiles.add(String());
   
   // note that "current midi clip" bindable parameter is registered in leaf class
}

MidiSequencePluginBase::~MidiSequencePluginBase ()
{
   deleteAndZero (midiSequence);
   deleteAndZero (allClipsByChannelSequence);
    
   removeAllParameters (true);
}

//==============================================================================
void MidiSequencePluginBase::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	noteOffs.clear();

    transport = getParentHost()->getTransport ();

    keyboardState.reset();
    
    startTimer (1000 / 25);	
}

void MidiSequencePluginBase::releaseResources()
{
    stopTimer ();
}

void MidiSequencePluginBase::processBlock (AudioSampleBuffer& buffer,
                                            MidiBuffer& midiMessages)
{
    const int blockSize = buffer.getNumSamples ();

    MidiBuffer* midiBuffer = midiBuffers.getUnchecked (0);

    keyboardState.processNextMidiBuffer (*midiBuffer,
                                         0, blockSize,
                                         true);

    if (transport->isPlaying ())
    {
        const int frameCounter = transport->getPositionInFrames ();
        const int framesPerBeat = transport->getFramesPerBeat ();
        const double framePerBeatDelta = 1.0f / (double) framesPerBeat;

        // prepare record incoming midi messages
        if (transport->isRecording ())
        {
            int samplePos = 0;
            MidiMessage msg (0xf4, 0.0);

            MidiBuffer::Iterator eventIterator (*midiBuffer);
            while (eventIterator.getNextEvent (msg, samplePos))
            {
                msg.setTimeStamp ((frameCounter + samplePos) * framePerBeatDelta);
                recordingSequence.addEvent (msg);
            }
        }

      const int nextBlockFrameNumber = frameCounter + blockSize;        
		const int seqIndex = getLoopRepeatIndex();
		double beatCount = getLoopBeatPosition();
		const double frameLenBeatCount = (nextBlockFrameNumber - frameCounter) / ((double)framesPerBeat);		
		double frameEndBeatCount = beatCount + frameLenBeatCount;
		if (frameEndBeatCount > getLengthInBeats())
			frameEndBeatCount -= getLengthInBeats();

      int midiChannel = getMidiChannel();
      
      ScopedReadLock seqlock(midiPlaybackSequenceLock);
      
      // normal case - the buffer occurs somewhere within the seq loop (i.e. start is before end!)
		if (frameEndBeatCount > beatCount)
		{
         if (uptoBeatReloopHack >0.0) // if this is > 0 it means we rendered a partial chunk of the start of the loop last time, so should not render those notes again, to avoid hanging double ups
            beatCount = uptoBeatReloopHack;
         newRenderEvents(*midiSequence, midiBuffer, beatCount, frameEndBeatCount, false, 0, framesPerBeat, nextBlockFrameNumber, seqIndex, blockSize, midiChannel);
         uptoBeatReloopHack = 0;
      }
      // special case - the buffer straddles the end/start seq loop
		else
		{
         double endChunkLenBeats = getLengthInBeats() - beatCount;
         int framesMoved = endChunkLenBeats * framesPerBeat;
         newRenderEvents(*midiSequence, midiBuffer, beatCount, getLengthInBeats(), true, 0, framesPerBeat, nextBlockFrameNumber-framesMoved, seqIndex, blockSize, midiChannel);
         newRenderEvents(*midiSequence, midiBuffer, 0.0, frameEndBeatCount, false, framesMoved, framesPerBeat, nextBlockFrameNumber, seqIndex+1, blockSize, midiChannel);
         uptoBeatReloopHack = frameEndBeatCount; // store where we relooped in case Transport asks us to do some of it again..
		}
    }

    if (doAllNotesOff || transport->willSendAllNotesOff ())
    {
      noteOffs.clear();
        midiBuffer->addEvent (allNotesOff, blockSize - 1);
        doAllNotesOff = false;
    }
}

void MidiSequencePluginBase::newRenderEvents(
   const MidiMessageSequence& sourceMidiBuffer,
   MidiBuffer* outMidiBuffer,
   double beatCount, // beat pos at start of render chunk
   double frameEndBeatCount, // beat pos at end of render chunk
   bool isEndOfLoop, // are we the render that buts up to the end of the loop?
   const int frameCounter, // frame (sample) number at start of render chunk, relative to chunk (i.e. often 0)
   const int framesPerBeat, // number of frames per beat
   const int nextBlockFrameNumber, // next render chunk frame number (upper limit)
   const int seqIndex, // how many times we've already looped
   const int blockSize, // number of frames long this render chunk is
   const int midiChannel // the midi channel to use for all rendered events
   )
{
   bool weAreRenderingNoteOffs = (&sourceMidiBuffer == &noteOffs);

	for (int i = sourceMidiBuffer.getNextIndexAtTime(beatCount*playRate);
		i < sourceMidiBuffer.getNumEvents(); i++)
	{
      // get the event (its time is in beats)
		MidiMessage* midiMessage = &sourceMidiBuffer.getEventPointer (i)->message;
      if (!midiMessage || !midiMessage->isNoteOnOrOff()) // we only sequence note events - not a fully generic rec-play sequencer
         continue;
         
      // filter note-ons that are hidden from view
      int noteNumber = midiMessage->getNoteNumber();
      int minNote = getIntValue (PROP_SEQBOTTOMROW, 0);
      int maxNote = minNote + getIntValue (PROP_SEQNUMROWS, 127) - 1;
      if ((noteNumber < minNote || noteNumber > maxNote) && !weAreRenderingNoteOffs)
         continue;
         
      // determine whether the event is one that we need to play in this time chunk..
      double beatsTime = sourceMidiBuffer.getEventTime(i)/playRate;
      // we care about note offs after the end of the loop (in case the user has resized loop and cut off the end of a note)
      bool specialNoteOffPastReloop = !weAreRenderingNoteOffs && (isEndOfLoop && midiMessage->isNoteOff() && beatsTime >= frameEndBeatCount); 
      if (
         (beatsTime >= beatCount && beatsTime < frameEndBeatCount) //|| // event occurs within render chunk time frame OR
         //specialNoteOffPastReloop 
         )
      {
         // now we play the event ..
         if (
            (isEnabled() && midiMessage->isNoteOn()) || // if it's a note on & we're enabled, OR
            weAreRenderingNoteOffs // it's a queued note off
            )
         {
            // determine the frame time of the event relative to start of chunk
            int renderAtFrame = framesPerBeat * (beatsTime - beatCount) + frameCounter;
            
            // set the channel, and render it
            midiMessage->setChannel(midiChannel); // a bit dirty, touching the actual message, though no real need to take a copy
            outMidiBuffer->addEvent(*midiMessage, renderAtFrame);

            // store the note off so we are sure to play it 
            // we cache note offs for played notes so that if sequencer gets disabled, any dangling notes are turned note-offed 
            if (!weAreRenderingNoteOffs)
            {
               int noteOffIndex = sourceMidiBuffer.getIndexOfMatchingKeyUp(i);
               MidiMessage* noteOffMessage = &sourceMidiBuffer.getEventPointer (noteOffIndex)->message;
               if (noteOffMessage)
               {
                  MidiMessage blowOffSteam = MidiMessage::noteOff(midiMessage->getChannel(), midiMessage->getNoteNumber());
                  blowOffSteam.setTimeStamp(noteOffMessage->getTimeStamp());
                  noteOffs.addEvent(blowOffSteam);
               }
            }
         }
      }
      
      if (specialNoteOffPastReloop)
      {
         MidiMessage blowOffSteam = MidiMessage::noteOff(midiMessage->getChannel(), midiMessage->getNoteNumber());
         noteOffs.addEvent(blowOffSteam, beatsTime - getLengthInBeats());
      }
   }
   
   // and now we recurse for note offs!
   if (!weAreRenderingNoteOffs && noteOffs.getNumEvents() > 0)
   {
      newRenderEvents(
         noteOffs,
         outMidiBuffer,
         beatCount, // beat pos at start of render chunk
         frameEndBeatCount, // beat pos at end of render chunk
         isEndOfLoop, // are we the render that buts up to the end of the loop?
         frameCounter, // frame (sample) number at start of render chunk
         framesPerBeat, // number of frames per beat
         nextBlockFrameNumber, // next render chunk frame number (upper limit)
         seqIndex, // how many times we've already looped
         blockSize, // number of frames long this render chunk is
         midiChannel // the midi channel to use for all rendered events
      );
      cleanUpNoteOffs(beatCount, frameEndBeatCount);
   }

}

void MidiSequencePluginBase::cleanUpNoteOffs(double fromTime, double toTime)
{
	for (int i=noteOffs.getNumEvents()-1; i>=0; i--)
	{
      double beatsTime = noteOffs.getEventPointer(i)->message.getTimeStamp() / playRate;
		if (beatsTime >= fromTime && beatsTime < toTime)
			noteOffs.deleteEvent(i, false);
	}
}

void MidiSequencePluginBase::timerCallback ()
{
    if (transport->isRecording ()
        && transport->willStopRecord ())
    {
        DBG ("Finalizing recording !");

        MidiMessageSequence* oldSequence = midiSequence;
        MidiMessageSequence* sequence = new MidiMessageSequence ();

        sequence->addSequence (*oldSequence, 0.0f, 0.0f, 1000000000.0f);
        sequence->updateMatchedPairs ();
        sequence->addSequence (recordingSequence, 0.0f, 0.0f, 1000000000.0f);
        sequence->updateMatchedPairs ();

        {
        const ScopedLock sl (parentHost->getCallbackLock());
        midiSequence = sequence;
        }

        transport->resetRecording ();

        deleteAndZero (oldSequence);
        recordingSequence.clear ();

        sendChangeMessage (this);

        DBG ("Recording OK !");
    }
}


//==============================================================================
bool MidiSequencePluginBase::timeSignatureChanged (const int barsCount,
                                                    const int timeDenominator)
{
	// only change the transport if we have a bigger duration
	if (barsCount > transport->getNumBars())
	{
		transport->setTimeSignature (transport->getTempo (),
                                 barsCount,
                                 timeDenominator);
		transport->setRightLocator (barsCount * timeDenominator);
	}

    return true;
}

bool MidiSequencePluginBase::playingPositionChanged (const float absolutePosition)
{
    transport->setPositionAbsolute (absolutePosition);
    return true;
}

bool MidiSequencePluginBase::noteAdded (const int noteNumber,
                                    const float beatNumber,
                                    const float noteLength)
{
    MidiMessage msgOn = MidiMessage::noteOn (getMidiChannel(), noteNumber, NOTE_VELOCITY);
    msgOn.setTimeStamp (beatNumber);
    MidiMessage msgOff = MidiMessage::noteOff (getMidiChannel(), noteNumber);
    msgOff.setTimeStamp ((beatNumber + noteLength) - NOTE_PREFRAMES);

    DBG ("Adding:" + String (noteNumber) + " " + String (beatNumber));

    {
    const ScopedLock sl (parentHost->getCallbackLock ());

    midiSequence->addEvent (msgOn);
    midiSequence->addEvent (msgOff);
    midiSequence->updateMatchedPairs ();
    }

    DBG ("Added:" + String (noteNumber) + " " + String (beatNumber));

    return true;
}

bool MidiSequencePluginBase::noteRemoved (const int noteNumber,
                                      const float beatNumber,
                                      const float noteLength)
{
    DBG ("Removing:" + String (noteNumber) + " " + String (beatNumber));

    double noteOnBeats = beatNumber - NOTE_PREFRAMES;

    int eventIndex = midiSequence->getNextIndexAtTime (noteOnBeats);
    while (eventIndex < midiSequence->getNumEvents ())
    {
        MidiMessage* eventOn = &midiSequence->getEventPointer (eventIndex)->message;

        if (eventOn->isNoteOn () && eventOn->getNoteNumber () == noteNumber)
        {
            // TODO - check note off distance == noteLength
            {
            const ScopedLock sl (parentHost->getCallbackLock());

            midiSequence->deleteEvent (eventIndex, true);
            midiSequence->updateMatchedPairs ();
            }

            DBG ("Removed:" + String (eventIndex) + " > " + String (noteNumber) + " @ " + String (beatNumber));

//            if (transport->isPlaying ())
//                doAllNotesOff = true;

            return true;
        }

        eventIndex++;
    }

    DBG (">>> Remove failed:" + String (noteNumber) + " @ " + String (beatNumber));

    return false;
}

bool MidiSequencePluginBase::noteMoved (const int oldNote,
                                    const float oldBeat,
                                    const int noteNumber,
                                    const float beatNumber,
                                    const float noteLength)
{
    DBG ("Moving:" + String (oldNote) + " " + String (oldBeat));

    double noteOnBeats = oldBeat - NOTE_PREFRAMES;

    int eventIndex = midiSequence->getNextIndexAtTime (noteOnBeats);
    while (eventIndex < midiSequence->getNumEvents ())
    {
        MidiMessage* eventOn = &midiSequence->getEventPointer (eventIndex)->message;

        if (eventOn->isNoteOn () && eventOn->getNoteNumber () == oldNote)
        {
            // TODO - check old note off distance == oldNoteLength

            MidiMessage msgOn = MidiMessage::noteOn (getMidiChannel(), noteNumber, NOTE_VELOCITY);
            msgOn.setTimeStamp (beatNumber);
            MidiMessage msgOff = MidiMessage::noteOff (getMidiChannel(), noteNumber);
            msgOff.setTimeStamp ((beatNumber + noteLength) - NOTE_PREFRAMES);

            {
            const ScopedLock sl (parentHost->getCallbackLock());

            // remove old events
            midiSequence->deleteEvent (eventIndex, true);
            midiSequence->updateMatchedPairs ();
            // add new events
            midiSequence->addEvent (msgOn);
            midiSequence->addEvent (msgOff);
            midiSequence->updateMatchedPairs ();
            }

            DBG ("Moved:" + String (eventIndex) + " > "
                          + String (oldNote) + " @ " + String (oldBeat) + " to "
                          + String (noteNumber) + " @ " + String (beatNumber));

//            if (transport->isPlaying ())
//                doAllNotesOff = true;

            return true;
        }

        eventIndex++;
    }

    DBG (">>> Move failed:" + String (oldNote) + " @ " + String (oldBeat));

    return false;
}

bool MidiSequencePluginBase::noteResized (const int noteNumber,
                                      const float beatNumber,
                                      const float noteLength)
{
    DBG ("Resizing:" + String (noteNumber) + " " + String (beatNumber));

    double noteOnBeats = beatNumber - NOTE_PREFRAMES;

    int eventIndex = midiSequence->getNextIndexAtTime (noteOnBeats);
    while (eventIndex < midiSequence->getNumEvents ())
    {
        MidiMessage* eventOn = &midiSequence->getEventPointer (eventIndex)->message;

        if (eventOn->isNoteOn () && eventOn->getNoteNumber () == noteNumber)
        {
            // TODO - check old note off distance == oldNoteLength

            MidiMessage msgOn = MidiMessage::noteOn (getMidiChannel(), noteNumber, NOTE_VELOCITY);
            msgOn.setTimeStamp (beatNumber);
            MidiMessage msgOff = MidiMessage::noteOff (getMidiChannel(), noteNumber);
            msgOff.setTimeStamp ((beatNumber + noteLength) - NOTE_PREFRAMES);

            {
            const ScopedLock sl (parentHost->getCallbackLock());

            // delete old events
            midiSequence->deleteEvent (eventIndex, true);
            midiSequence->updateMatchedPairs ();
            // add new events
            midiSequence->addEvent (msgOn);
            midiSequence->addEvent (msgOff);
            midiSequence->updateMatchedPairs ();
            }

            DBG ("Resized:" + String (eventIndex) + " > "
                             + String (noteNumber) + " @ " + String (beatNumber) + " to " + String (noteLength));

//            if (transport->isPlaying ())
//                doAllNotesOff = true;

            return true;
        }

        eventIndex++;
    }

    DBG (">>> Resize failed:" + String (noteNumber) + " @ " + String (beatNumber));

    return false;
}

bool MidiSequencePluginBase::allNotesRemoved ()
{
    {
    const ScopedLock sl (parentHost->getCallbackLock());

    midiSequence->clear ();

    if (transport->isPlaying ())
        doAllNotesOff = true;

    }

    return true;
}

//==============================================================================
void MidiSequencePluginBase::getNoteOnIndexed (const int index, int& note, float& beat, float& length)
{
    int numNoteOn = 0;
    for (int i = 0; i < midiSequence->getNumEvents (); i++)
    {
        MidiMessageSequence::MidiEventHolder* eventOn = midiSequence->getEventPointer (i);
        MidiMessageSequence::MidiEventHolder* eventOff = midiSequence->getEventPointer (i)->noteOffObject;

        MidiMessage* msgOn = & eventOn->message;
        if (eventOn->message.isNoteOn () && eventOff)
        {
            MidiMessage* msgOff = & eventOff->message;
            if (index == numNoteOn)
            {
                note = msgOn->getNoteNumber ();
                beat = msgOn->getTimeStamp ();
                length = ((msgOff->getTimeStamp () + NOTE_PREFRAMES) - msgOn->getTimeStamp ());
                break;
            }
            numNoteOn++;
        }
    }
}

int MidiSequencePluginBase::getNumNoteOn () const
{
    int numNoteOn = 0;

    for (int i = 0; i < midiSequence->getNumEvents (); i++)
        if (midiSequence->getEventPointer (i)->message.isNoteOn ()) numNoteOn++;

    return numNoteOn;
}

//==============================================================================
static void dumpMidiMessageSequence (MidiMessageSequence* midiSequence)
{
#if JUCE_DEBUG && 0
    for (int i = 0; i < midiSequence->getNumEvents (); i++)
    {
        MidiMessage* midiMessage = &midiSequence->getEventPointer (i)->message;

        std::cerr << (String (midiMessage->getNoteNumber()) + " "
             + (midiMessage->isNoteOn() ? "ON " : "OFF ")
             + String (midiMessage->getTimeStamp())).toUTF8() << std::endl;

        // DBG ("Playing event @ " + String (frameCounter) + " : " + String (timeStamp));
    }
#endif
}

//==============================================================================
void MidiSequencePluginBase::getStateInformation (MemoryBlock &mb) 
{
	getChunk(mb); 
}

void MidiSequencePluginBase::setStateInformation (const void* data, int sizeInBytes)
{
	MemoryBlock tempBlock (data, sizeInBytes);
	setChunk(tempBlock);
}

void MidiSequencePluginBase::getChunk (MemoryBlock& mb)
{
    MidiFile midiFile;

   pushCurrentEditedClipToAllClipsSequence(currentClip);

    // add real sequence scaled down !
    MidiMessageSequence scaledSequence;
    for (int i = 0; i < allClipsByChannelSequence->getNumEvents (); i++)
    {
        MidiMessage midiMessage = allClipsByChannelSequence->getEventPointer (i)->message;
        midiMessage.setTimeStamp (midiMessage.getTimeStamp() * 100000.0f);
        scaledSequence.addEvent (midiMessage);
    }
    midiFile.addTrack (scaledSequence);

    MemoryOutputStream os (4096, 2048, &mb);
    midiFile.writeTo (os);

    dumpMidiMessageSequence (midiSequence);
}

void MidiSequencePluginBase::setChunk (const MemoryBlock& mb)
{
   MemoryInputStream is (mb.getData(), mb.getSize(), false);

   MidiFile midiFile;
   midiFile.readFrom (is);

   allClipsByChannelSequence->clear ();

   const MidiMessageSequence* notesSequence = midiFile.getTrack (0);
   if (notesSequence)
   {
      // add real sequence scaled down !
      for (int i = 0; i < notesSequence->getNumEvents (); i++)
      {
         MidiMessage midiMessage = notesSequence->getEventPointer (i)->message;
         if (midiMessage.isNoteOnOrOff() || midiMessage.isController())
         {
            midiMessage.setTimeStamp (midiMessage.getTimeStamp() / 100000.0f);
            allClipsByChannelSequence->addEvent (midiMessage);
         }
      }
      if (transport->isPlaying ())
         doAllNotesOff = true;
   }
   
   allClipsByChannelSequence->updateMatchedPairs ();
   getCurrentEditClipFromAllClipsSequence(currentClip);

   dumpMidiMessageSequence (midiSequence);
}

//==============================================================================
void MidiSequencePluginBase::savePropertiesToXml (XmlElement* xml)
{
    BasePlugin::savePropertiesToXml (xml);

    xml->setAttribute (PROP_SEQROWOFFSET,            getIntValue (PROP_SEQROWOFFSET, 0));
    xml->setAttribute (PROP_SEQCOLSIZE,              getIntValue (PROP_SEQCOLSIZE, 80));
    xml->setAttribute (PROP_SEQNOTELENGTH,           getIntValue (PROP_SEQNOTELENGTH, 4));
    xml->setAttribute (PROP_SEQNOTESNAP,             getIntValue (PROP_SEQNOTESNAP, 4));
    xml->setAttribute (PROP_SEQBAR,                  getIntValue (PROP_SEQBAR, 4));
    xml->setAttribute (PROP_SEQENABLED,              getBoolValue (PROP_SEQENABLED, true));
    xml->setAttribute (PROP_SEQMIDICHANNEL,          getIntValue (PROP_SEQMIDICHANNEL, 1));
    xml->setAttribute (PROP_SEQCURRENTCLIP,          getCurrentClipIndex());
    xml->setAttribute (PROP_SEQROWHEIGHT,          getIntValue (PROP_SEQROWHEIGHT, 10));
    xml->setAttribute (PROP_SEQBOTTOMROW,          getIntValue (PROP_SEQBOTTOMROW, 0));
    xml->setAttribute (PROP_SEQNUMROWS,          getIntValue (PROP_SEQNUMROWS, 127));
    xml->setAttribute (PROP_SEQTRIGGERSYNCHEDTOGLOBAL,          getDoubleValue (PROP_SEQTRIGGERSYNCHEDTOGLOBAL, 1));
    
   XmlElement* clipsElement = new XmlElement(PROP_SEQCLIPFILES);
   for (int i=0; i<jmin(getMaxUsedClipIndex()+1, DEFAULT_MAXIMUM_CLIPS); i++)
   {
      XmlElement* clipEl = new XmlElement(PROP_SEQCLIPITEM);
      clipEl->setAttribute(PROP_SEQCLIPINDEX, i);
      clipEl->setAttribute(PROP_SEQCLIPFILE, clipFiles[i]);
      clipsElement->addChildElement(clipEl);
   }
   xml->addChildElement(clipsElement);
}

void MidiSequencePluginBase::loadPropertiesFromXml (XmlElement* xml)
{
    BasePlugin::loadPropertiesFromXml (xml);

    setValue (PROP_SEQROWOFFSET,                     xml->getIntAttribute (PROP_SEQROWOFFSET, 0));
    setValue (PROP_SEQCOLSIZE,                       xml->getIntAttribute (PROP_SEQCOLSIZE, 80));
    setValue (PROP_SEQNOTELENGTH,                    xml->getIntAttribute (PROP_SEQNOTELENGTH, 4));
    setValue (PROP_SEQNOTESNAP,                      xml->getIntAttribute (PROP_SEQNOTESNAP, 4));
    setValue (PROP_SEQBAR,                           xml->getIntAttribute (PROP_SEQBAR, 4));
    setValue (PROP_SEQENABLED,                       xml->getBoolAttribute (PROP_SEQENABLED, true));
    setValue (PROP_SEQMIDICHANNEL,                   xml->getIntAttribute (PROP_SEQMIDICHANNEL, 1));
    currentClip = xml->getIntAttribute (PROP_SEQCURRENTCLIP, 0);
    setValue (PROP_SEQROWHEIGHT,                   xml->getIntAttribute (PROP_SEQROWHEIGHT, 10));
    setValue (PROP_SEQBOTTOMROW,                   xml->getIntAttribute (PROP_SEQBOTTOMROW, 0));
    setValue (PROP_SEQNUMROWS,                   xml->getIntAttribute (PROP_SEQNUMROWS, 127));
    setValue (PROP_SEQTRIGGERSYNCHEDTOGLOBAL,                   xml->getDoubleAttribute (PROP_SEQTRIGGERSYNCHEDTOGLOBAL, 1));
    setValue (PROP_SEQPLAYRATE,                   xml->getDoubleAttribute (PROP_SEQPLAYRATE, 1));

   XmlElement* clipsElement = xml->getChildByName(PROP_SEQCLIPFILES);
   if (clipsElement)
   {
      forEachXmlChildElementWithTagName (*clipsElement, e, PROP_SEQCLIPITEM)
      {
         int clipindex = e->getIntAttribute (PROP_SEQCLIPINDEX);
         String clipfile = e->getStringAttribute (PROP_SEQCLIPFILE);
         clipFiles.set(clipindex, clipfile);
      }
   }
}

//==============================================================================
AudioProcessorEditor* MidiSequencePluginBase::createEditor ()
{
    return new SequenceComponent (this);
}

void MidiSequencePluginBase::setSequence(const MidiMessageSequence* mseq, int clipNumber)
{
   if (clipNumber == -1)
      clipNumber = getCurrentClipIndex();
      
	if (mseq && clipNumber >= 0 && clipNumber < 16)
	{
      allClipsByChannelSequence->deleteMidiChannelMessages(clipNumber + 1);
      allClipsByChannelSequence->addSequence(*mseq, 0, 0, DBL_MAX);
      allClipsByChannelSequence->updateMatchedPairs();
	}
}

int MidiSequencePluginBase::getLoopRepeatIndex() 
{ 
	return static_cast<int>(floor(transport->getPositionInBeats())) / getLengthInBeats(); 
}

double MidiSequencePluginBase::getLoopBeatPosition() 
{
// parameter-controlled phase (experiment!)
//   double phaseInBeats = getDoubleValue(PROP_SEQLOOPPHASE, 00) * getLengthInBeats();
   double phaseInBeats = loopPhaseInBeats;
   double loopLenBeats = getLengthInBeats();
	double beat = ((transport->getPositionInBeats() - phaseInBeats)) - (getLoopRepeatIndex() * loopLenBeats); 
    if (beat > loopLenBeats)
      beat -= loopLenBeats;
   else if (beat < 0)
      beat += loopLenBeats;
   return beat;
}

double MidiSequencePluginBase::getPlayRate()
{
   return playRate;
}

//==============================================================================

void MidiSequencePluginBase::setMidiChannel(int chan)
{
   setValue(PROP_SEQMIDICHANNEL, chan);
   
   // automatically ensure any output midi filter is switched to the new channel
   if (getMidiOutputChannel() != -1 && getMidiOutputChannel() != getMidiChannel())
      setMidiOutputChannelFilter(getMidiChannel());
}

int MidiSequencePluginBase::getMaxUsedClipIndex()
{
   int maxClip = 0;
   for (int i=16; i>0 && maxClip == 0; i--)
   {
      // if the clip's midi channel has some events
      int clipChannel = i+1;
      MidiMessageSequence tmp;
      allClipsByChannelSequence->extractMidiChannelMessages(clipChannel, tmp, false);
      if (tmp.getNumEvents() > 0)
         maxClip = i; // found the last one, will cause loop break

      // also if there is a clip file in a slot then it is a clip (even if it has no notes)
      if (clipFiles[i].isNotEmpty())
         maxClip = i;
   }      
   return maxClip;
}

String MidiSequencePluginBase::getClipFile(int clipIndex) const
{
   String currentFile = "<unnamed>";
   
   if (clipIndex >= 0 && clipIndex < clipFiles.size() && clipFiles[clipIndex].containsNonWhitespaceChars())
      currentFile = clipFiles[clipIndex];
      
   return currentFile;
}

int MidiSequencePluginBase::getCurrentClipIndex()
{
   return currentClip;
}

void MidiSequencePluginBase::setCurrentClipIndex(int index, bool forceImportEvenIfSameAsCurrent)
{
   int oldClipIndex = currentClip;
   if (index >= 0 && index < 16) // has to be a valid midi channel
      currentClip = index;
   int newClipIndex = currentClip;

   if (allClipsByChannelSequence && (forceImportEvenIfSameAsCurrent || (newClipIndex != oldClipIndex)))
   {
      // copy any edits to current into the clip sequence
      pushCurrentEditedClipToAllClipsSequence(oldClipIndex);
      
      // copy requested clip to play/edit/view sequence
      getCurrentEditClipFromAllClipsSequence(newClipIndex);
   }

   sendChangeMessage (this);
}

void MidiSequencePluginBase::pushCurrentEditedClipToAllClipsSequence(int oldClipIndex)
{
   MidiMessageSequence currentPlayingClip;
   {
      ScopedReadLock seqlock(midiPlaybackSequenceLock);
      currentPlayingClip = MidiMessageSequence(*midiSequence);
   }
   setClipMidiSequence(currentPlayingClip, oldClipIndex);
}

void MidiSequencePluginBase::getCurrentEditClipFromAllClipsSequence(int newClipIndex)
{
   MidiMessageSequence newSeq;
   allClipsByChannelSequence->extractMidiChannelMessages(newClipIndex + 1, newSeq, false);
   for (int i=0; i<newSeq.getNumEvents(); i++)
      newSeq.getEventPointer(i)->message.setChannel(getMidiChannel());
   newSeq.updateMatchedPairs();

   if (transport->isPlaying ())
       doAllNotesOff = true;

   {
      ScopedWriteLock seqlock(midiPlaybackSequenceLock);
      
      deleteAndZero(midiSequence);
      midiSequence = new MidiMessageSequence(newSeq);
      midiSequence->updateMatchedPairs();
   }
}

void MidiSequencePluginBase::handleEditedClipList(StringArray newClipList, std::vector<int> changeInfo)
{
   // ensure any edits to current clip get saved
   pushCurrentEditedClipToAllClipsSequence(currentClip);

   MidiMessageSequence* newAllSequence = new MidiMessageSequence();

   int newCurrentClip = 0;
   for (int i=0; i<changeInfo.size(); i++) // for each position in the new order
   {
      MidiMessageSequence tmp;
      // get oldChan sequence
      allClipsByChannelSequence->extractMidiChannelMessages(changeInfo[i]+1, tmp, false); // get events from old channel
      // change to use new midi channel
      for (int j=0; j<tmp.getNumEvents(); j++)
         tmp.getEventPointer(j)->message.setChannel(i+1); 
      // copy to everything sequence
      newAllSequence->addSequence(tmp, 0, 0, DBL_MAX);
      
      if (changeInfo[i] == currentClip)
         newCurrentClip = changeInfo[i];
   }
   
   newAllSequence->updateMatchedPairs();
   MidiMessageSequence* oldall = allClipsByChannelSequence;
   allClipsByChannelSequence = newAllSequence;
   delete oldall;
   clipFiles = newClipList;

   currentClip = newCurrentClip;
   getCurrentEditClipFromAllClipsSequence(newCurrentClip);
   sendChangeMessage(this);
}

void MidiSequencePluginBase::moveClipFromIndexToIndex(int cur, int newIndex)
{
   // copy it to new location (if in range; otherwise this method implements remove!)
   MidiMessageSequence tmp;
   if (newIndex >= 0 && newIndex < 16)
   {
      int newChan = newIndex + 1;
      // copy clip to move
      allClipsByChannelSequence->extractMidiChannelMessages(cur+1, tmp, false);
      // clear out destination clip slot
      allClipsByChannelSequence->deleteMidiChannelMessages(newIndex+1);
      // fix channel
      for (int i=0; i<tmp.getNumEvents(); i++)
         tmp.getEventPointer(i)->message.setChannel(newChan);
      // copy it back in
      allClipsByChannelSequence->addSequence(tmp, 0, 0, DBL_MAX);
   }

   // remove it from old location
   if (cur >= 0 && cur < 16)
      allClipsByChannelSequence->deleteMidiChannelMessages(cur+1);

   allClipsByChannelSequence->updateMatchedPairs();
}

void MidiSequencePluginBase::setClipMidiSequence(const MidiMessageSequence& seqClip, int clipIndex)
{
   int clipChan = clipIndex + 1;
   for (int i=0; i<seqClip.getNumEvents(); i++)
   {
      MidiMessageSequence::MidiEventHolder* event = seqClip.getEventPointer(i);
      event->message.setChannel(clipChan);
   }
   allClipsByChannelSequence->deleteMidiChannelMessages(clipChan);
   allClipsByChannelSequence->addSequence(seqClip, 0, 0, DBL_MAX);
}

void MidiSequencePluginBase::importMidiFileToClip(const File& file, int clipIndex)
{
   FileInputStream midiFileIn(file);
   MidiFile droppedMidiFile;
   droppedMidiFile.readFrom(midiFileIn);
   droppedMidiFile.convertTimestampTicksToSeconds();
   if (droppedMidiFile.getNumTracks() > 0)
   {
      MidiMessageSequence imported(*droppedMidiFile.getTrack(0));
      imported.updateMatchedPairs(); // "readFrom"d clips do not get updated with cached noteoff stuff, this line is critical!
      
      for (int i=0; i<imported.getNumEvents(); i++)
      {
         MidiMessageSequence::MidiEventHolder* event = imported.getEventPointer(i);
         event->message.setTimeStamp(event->message.getTimeStamp() * 2.0); // traditional workaround in seq gui import! (this 
      }
      setClipMidiSequence(imported, clipIndex);
   }
}

void MidiSequencePluginBase::importClipFiles(const StringArray& files)
{
   int numNewClips = files.size();
   
   // ensure any edits to current clip get saved
   pushCurrentEditedClipToAllClipsSequence(currentClip);
    
   // push existing clips down the list to make room for the new clips
   // ensure we push the files AND the midi clips in allClipsByChannelSequence
   for (int i=getMaxUsedClipIndex(); i>=0; i--)
      moveClipFromIndexToIndex(i, i+numNewClips);
   
   // load the new files into the empty slots at the top of the list
   for (int i=0; i<numNewClips; i++)
   {
      importMidiFileToClip(File(files[i]), numNewClips-i-1);
      clipFiles.insert(0, files[i]);
   }

   getCurrentEditClipFromAllClipsSequence(0);
   setCurrentClipIndex(0);

   // ensure we don't have more than 16 files in the list
   for (int i=clipFiles.size(); i>=DEFAULT_MAXIMUM_CLIPS; i--)
      clipFiles.remove(i);
}


//==============================================================================
const int maxPx = 108; 
const int minPx = 8;

void MidiSequencePluginBase::setParameterReal (int paramNumber, float value)
{
   if (paramNumber == MIDISEQ_PARAMID_CURRENTCLIP)
   {
      int newClipIndex = value * DEFAULT_MAXIMUM_CLIPS;
      if (newClipIndex > getMaxUsedClipIndex())
         newClipIndex = 0;
      setCurrentClipIndex(newClipIndex);
   }
   
   if (paramNumber == MIDISEQ_PARAMID_NUMROWS)
      setValue (PROP_SEQNUMROWS, 1 + value * 126.0);
   if (paramNumber == MIDISEQ_PARAMID_BOTTOMROWNOTE)
      setValue (PROP_SEQBOTTOMROW, value * 127.0);
   if (paramNumber == MIDISEQ_PARAMID_ROWHEIGHT)
      setValue (PROP_SEQROWHEIGHT, minPx + value * (maxPx-minPx));
   if (paramNumber == MIDISEQ_PARAMID_TRIGGERSYNCHED)
      setValue (PROP_SEQTRIGGERSYNCHEDTOGLOBAL, value);
   if (paramNumber == MIDISEQ_PARAMID_PLAYRATE)
   {
      setValue (PROP_SEQPLAYRATE, value);
      double oldPlayRate = playRate;
      double div = 1 / 3.0;
      if (value < div * 1)
         playRate = 1/16.0;
      else if (value < div * 2)
         playRate = 1/4.0;
      else
         playRate = 1.0;
      
      if ((playRate != oldPlayRate) && transport->isPlaying())
         doAllNotesOff = true;
   }
  sendChangeMessage (this);
}

float MidiSequencePluginBase::getParameterReal (int paramNumber)
{
   float value = 0.0f;
   if (paramNumber == MIDISEQ_PARAMID_CURRENTCLIP)
      value = static_cast<double>(currentClip) / DEFAULT_MAXIMUM_CLIPS;

   else if (paramNumber == MIDISEQ_PARAMID_NUMROWS)
      value = (getIntValue(PROP_SEQNUMROWS, 127) - 1) / 126.0;
   else if (paramNumber == MIDISEQ_PARAMID_BOTTOMROWNOTE)
      value = getIntValue(PROP_SEQBOTTOMROW, 0) / 127.0;
   else if (paramNumber == MIDISEQ_PARAMID_ROWHEIGHT)
      value = (getIntValue(PROP_SEQROWHEIGHT, 10) - minPx) / static_cast<float>(maxPx-minPx);
   else if (paramNumber == MIDISEQ_PARAMID_TRIGGERSYNCHED)
      value = getDoubleValue(PROP_SEQTRIGGERSYNCHEDTOGLOBAL, 1) > 0.5;
   else if (paramNumber == MIDISEQ_PARAMID_PLAYRATE)
      value = getDoubleValue(PROP_SEQPLAYRATE, 1);
   
   return value;
}

const String MidiSequencePluginBase::getParameterTextReal (int paramNumber, float value)
{
   String paramTxt;//(String(getParameterReal(paramNumber, value));
   if (paramNumber == MIDISEQ_PARAMID_CURRENTCLIP)
   {
      int newClipIndex = round(value * DEFAULT_MAXIMUM_CLIPS);
      paramTxt = String(String(newClipIndex) + String(" ") + File(getClipFile(newClipIndex)).getFileNameWithoutExtension());
   }
   if (paramNumber == MIDISEQ_PARAMID_NUMROWS)
      paramTxt = String(getIntValue(PROP_SEQNUMROWS, 127));
   else if (paramNumber == MIDISEQ_PARAMID_BOTTOMROWNOTE)
      paramTxt = String(getIntValue(PROP_SEQBOTTOMROW, 0));
   else if (paramNumber == MIDISEQ_PARAMID_ROWHEIGHT)
      paramTxt = String(getIntValue(PROP_SEQROWHEIGHT, 10));
   else if (paramNumber == MIDISEQ_PARAMID_PLAYRATE)
   {
      double div = 1 / 3.0;
      if (value < div * 1)
         paramTxt = String("1 beat => 4 bars");
      else if (value < div * 2)
         paramTxt = String("1 beat => 1 bar (quarter speed)");
      else
         paramTxt = String("normal");
   }
   else if (paramNumber == MIDISEQ_PARAMID_TRIGGERSYNCHED)
   {
      if (value > 0.5)
         paramTxt = String("Phase locked to global");
      else
         paramTxt = String("Phase triggered on sequence trigger");
   }

   return paramTxt;
}
