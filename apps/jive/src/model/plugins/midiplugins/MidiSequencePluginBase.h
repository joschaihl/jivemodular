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

#ifndef __JUCETICE_JOSTMIDISEQUENCEPLUGINBASE_HEADER__
#define __JUCETICE_JOSTMIDISEQUENCEPLUGINBASE_HEADER__

#include "../../BasePlugin.h"


//==============================================================================
/**
    MidiSequencePlugin properties should be not-realtime options:
*/

#define PROP_SEQROWOFFSET                     T("sRoff")
#define PROP_SEQCOLSIZE                       T("sCsize")
#define PROP_SEQNOTESNAP                      T("sNsnap")
#define PROP_SEQNOTELENGTH                    T("sNlen")
#define PROP_SEQBAR                           T("sNbar")
#define PROP_SEQENABLED                       T("sEnabled")
#define PROP_SEQMIDICHANNEL                   T("sMidiChan")
#define PROP_SEQCURRENTCLIP                   T("sCurrentClip")
#define PROP_SEQCLIPFILES                   T("sClipFiles")
#define PROP_SEQCLIPITEM                   T("sClipItem")
#define PROP_SEQCLIPINDEX                   T("sClipIndex")
#define PROP_SEQCLIPFILE                   T("sClipFile")
#define PROP_SEQROWHEIGHT                   T("sRowHeight")
#define PROP_SEQBOTTOMROW                   T("sBottomRow")
#define PROP_SEQNUMROWS                   T("sNumRows")
#define PROP_SEQTRIGGERSYNCHEDTOGLOBAL                   T("sSyncToGlobal") // parameter controlling whether trigger "unmutes" or starts sequence from zero
#define PROP_SEQPLAYRATE                   T("sPlayRate") // 0 = 1 beat is 4 bars; >0.33 = 1 beat is 1 bar; >0.666 = normal

#define MIDISEQ_BASESEQUENCERPARAMCOUNT 4
#define MIDISEQ_PARAMID_CURRENTCLIP 0
#define MIDISEQ_PARAMID_NUMROWS 1
#define MIDISEQ_PARAMID_BOTTOMROWNOTE 2
#define MIDISEQ_PARAMID_ROWHEIGHT 3
#define MIDISEQ_PARAMID_TRIGGERSYNCHED 4
#define MIDISEQ_PARAMID_PLAYRATE 5
#define MIDISEQ_PARAMID_NEXTAVAILABLE 6

//==============================================================================
/**
    A single track midi sequencer plugin with record functionality

    It is an internal plugin, and it interact with gui directly, so it needs to
    have a CriticalSection to regulate note read/write.

    Also, it is a PianoGridListener, as it will be registered with a PianoGrid
    that will notify this plugin about note add, remove, move and resize.

    @see SequenceComponent, PianoGrid
*/
class MidiSequencePluginBase : public BasePlugin,
						       public MidiGridListener,
                               public PianoGridListener,
                               public Timer
{
public:
    //==============================================================================
    /** Construct a midi sequence plugin */
    MidiSequencePluginBase ();

    /** Destructor */
    ~MidiSequencePluginBase ();

    //==============================================================================
    /** Get the type of the plugin

        This is an internal plugin, but is treated as any other plugin technology
    */
    virtual int getType () const         { return JOST_PLUGINTYPE_MIDISEQ; }

    //==============================================================================
    const String getName () const         { return T("Sequencer"); }
    int getVersion () const               { return 1; }
    int getNumInputs () const             { return 0; }
    int getNumOutputs () const            { return 0; }
    int getNumMidiInputs () const         { return 1; }
    int getNumMidiOutputs () const        { return 1; }
    bool acceptsMidi () const             { return true; }
    bool producesMidi () const            { return true; }
    bool isMidiInput () const             { return true; }

    //==============================================================================
    bool hasEditor () const               { return true; }
    bool wantsEditor () const             { return true; }
    bool isEditorInternal () const        { return true; }
    AudioProcessorEditor* createEditor();

    //==============================================================================
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();

    //==============================================================================
    void timerCallback ();

    //==============================================================================
    bool timeSignatureChanged (const int barsCount,
                               const int timeDenominator);

    bool playingPositionChanged (const float absolutePosition);

    //==============================================================================
    virtual bool noteAdded (const int noteNumber,
                             const float beatNumber,
                             const float noteLength);

    virtual bool noteRemoved (const int noteNumber,
                               const float beatNumber,
                               const float noteLength);

    virtual bool noteMoved (const int oldNote,
                             const float oldBeat,
                             const int noteNumber,
                             const float beatNumber,
                             const float noteLength);

    virtual bool noteResized (const int noteNumber,
                               const float beatNumber,
                               const float noteLength);

    virtual bool allNotesRemoved ();

    //==============================================================================
    /** Return the specified note on

        This is typically used in GUI, when components need to rebuild internal
        note structure (typically a piano grid)
    */
    void getNoteOnIndexed (const int index, int& note, float& beat, float& length);

    /** Counts the available note on */
    int getNumNoteOn () const;

    //==============================================================================
    /* Set the midi sequence being played by the sequencer */
	void setSequence(const MidiMessageSequence* mseq, int clipNumber = -1);

    /* Get the midi sequence being played by the sequencer */
	MidiMessageSequence* getSequence() { return midiSequence; };

    //==============================================================================
    void setParameterReal (int paramNumber, float value);
    float getParameterReal (int paramNumber);
    const String getParameterTextReal (int paramNumber, float value);

   //==============================================================================

   int getMaxUsedClipIndex();

   String getClipFile(int clipIndex) const;

   int getCurrentClipIndex();
   void setCurrentClipIndex(int index, bool forceImportEvenIfSameAsCurrent=false);

   void importClipFiles(const StringArray& files);
   void handleEditedClipList(StringArray newClipList, std::vector<int> changeInfo);
   void setClipMidiSequence(const MidiMessageSequence& seqClip, int clipIndex);
   
    //==============================================================================
	/* Get the number of loops that have played since transport time zero (used for looping) */
	int getLoopRepeatIndex();
	
	/* Get the current playback position (accounting for loops) for the sequence, relative to the current transport position (used for looping) */
	double getLoopBeatPosition();

	/* Get the length of the sequence expressed as a number of beats */
	int getLengthInBeats() { return getIntValue(PROP_SEQBAR, 4) * getBeatsPerBar() / getPlayRate(); };
	
	/* Get the number of beats per bar (currently hard-coded to four) */
	double getBeatsPerBar() { return 4; };

	/* Returns true if sequencer playback is enabled */
   /* er.. virtual because the CC-bound enabledness is implemented in the subclass */
    virtual bool isEnabled() {return getBoolValue(PROP_SEQENABLED, true); };

	/* Get/set the MIDI channel used for all events (notes & CCs) output from the sequencer */
    int getMidiChannel() {return getIntValue(PROP_SEQMIDICHANNEL, 1); };
    void setMidiChannel(int chan);

   double getPlayRate();

    //==============================================================================
    /** Serialize internal properties to an Xml element */
    void savePropertiesToXml (XmlElement* element);

    /** Deserialize internal properties from an Xml element */
    void loadPropertiesFromXml (XmlElement* element);

    //==============================================================================
    /** Save midi sequence state to memory block */
	void getStateInformation (MemoryBlock &mb);

    /** Load midi sequence state from raw bytes */
    void setStateInformation (const void* data, int sizeInBytes);
	
    /** Serialize midi file to a memory block */
    void getChunk (MemoryBlock& mb);

    /** Deserialize midi file from a memory block */
    void setChunk (const MemoryBlock& mb);

protected:
   
   // at the moment the only way to set up clipFiles and midi correctly is via importClipFiles
   // so.. this routine is not for public use!
   void importMidiFileToClip(const File& file, int clipIndex);

	// internal, used when rendering midi events to ensure loops render appropriately and note offs don't get lost

	void newRenderEvents(const MidiMessageSequence& sourceMidiBuffer,
	                          MidiBuffer* outMidiBuffer,
	                          double beatCount,
	                          double frameEndBeatCount,
                             bool isEndOfLoop,
	                          const int frameCounter,
	                          const int framesPerBeat,
	                          const int nextBlockFrameNumber,
	                          const int seqIndex,
	                          const int blockSize,
                             const int midiChan);

	void cleanUpNoteOffs(double fromTime, double toTime);
    
    void pushCurrentEditedClipToAllClipsSequence(int oldClipIndex);
    void getCurrentEditClipFromAllClipsSequence(int newClipIndex);
   void moveClipFromIndexToIndex(int cur, int newIndex);

	MidiMessageSequence noteOffs;

    Transport* transport;

   ReadWriteLock midiPlaybackSequenceLock;
    MidiMessageSequence* midiSequence;
    MidiMessageSequence recordingSequence;
    MidiMessageSequence* allClipsByChannelSequence;
    
    StringArray clipFiles;
    int currentClip;

    bool doAllNotesOff;
    MidiMessage allNotesOff;

   double uptoBeatReloopHack;
   double loopPhaseInBeats;
   double playRate;
};


#endif

