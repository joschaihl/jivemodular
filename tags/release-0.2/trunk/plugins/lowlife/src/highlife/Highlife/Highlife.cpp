/*-
 * Copyright (c) discoDSP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of copyright holders nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *        This product includes software developed by discoDSP
 *        http://www.discodsp.com/ and contributors.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL COPYRIGHT HOLDERS OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// HighLife Implentation File                                                                                                          //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"


//==============================================================================
CHighLife::CHighLife ()
{
	// call all sounds off
	plug_all_sounds_off();

	// reset voice-poly sample pos trackers
	gui_recent_update=true;

	for(int vp=0;vp<MAX_POLYPHONY;vp++)
		vt_opos[vp]=0;

#ifndef DISABLE_VST_HOST
	// host init
	host_plug=NULL;
	host_plug_can_process=false;
	host_plug_queue_size=0;
#endif

    // suspended state
    suspended_state=0;

	// demo limit
	noise_count=0;

	// reset user gui vars
	user_gui_page=0;
	user_last_key=0x3c;
	user_program=0;
	user_pressed=0;
	user_ox=0;
	user_oy=0;
	user_delta_y_accum=0;
	user_master_volume=1.0f;

	// reset sample editor
	user_sed_manual=0;
	user_sed_zone=0;
	user_sed_cue=0;
	user_editor_mode=1;
	user_sed_offset=0;
	user_sed_spp=32;
	user_sed_adapt=1;
	user_sed_sel_sta=0;
	user_sed_sel_len=0;
	
	// user freezing settings reset
	user_route_midi=0;
	user_prg_splits=1;
	user_vel_splits=1;
	user_split_time=2.0f;
	user_low_keyboa=24;
	user_hig_keyboa=95;
	user_ste_keyboa=12;
	user_force_mono=0;
	user_normalization=1;

	// freezing info reset
	wave_processing=0;
	wave_total=0;
	wave_counter=0;
	wave_buf[0]=0;

	// program basic initialization
	for(int p=0;p<NUM_PROGRAMS;p++)
	{
		highlife_program[p].pzones=NULL;
		highlife_program[p].num_zones=0;
	}

	// reset bank
	tool_init_bank();

	// reset sample wave sclipboard
	user_clip_sample_size=0;
	user_clip_sample_channels=0;
	user_clip_sample=NULL;
}

CHighLife::~CHighLife ()
{
	// reset memstream out allocating object
	ms_out.Reset();

	// reset bank (init)
	tool_init_bank();

	// reset clipboard
	sed_clipboard_reset();
}

//==============================================================================
int CHighLife::getNumPrograms()
{
    return NUM_PROGRAMS;
}

int CHighLife::getCurrentProgram ()
{
	return user_program;
}

void CHighLife::setCurrentProgram (int program)
{
	if (user_program != program)
	{
		//set_suspended (1);
		
		plug_all_sounds_off();
		user_program=program;
		user_sed_adapt=1;
		
		//set_suspended (0);
		
		/* TODO 
		if (editor)
		    ((CHighLifeEditor*)editor)->gui_refresh();
		*/
	}
}

const String CHighLife::getProgramName (int index)
{
	return String (highlife_program[user_program].name);
}

void CHighLife::changeProgramName (int index, const String& newName)
{
	sprintf (highlife_program[index].name, (const char*) newName);
}

//==============================================================================
const String CHighLife::getInputChannelName (const int channelIndex) const
{
    return "In" + String (channelIndex + 1);
}

const String CHighLife::getOutputChannelName (const int channelIndex) const
{
    return "Out" + String (channelIndex + 1);
}

bool CHighLife::isInputChannelStereoPair (int index) const
{
    return false;
}

bool CHighLife::isOutputChannelStereoPair (int index) const
{
    return true;
}

//==============================================================================
void CHighLife::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    keyboard_state.reset();	
}

void CHighLife::releaseResources()
{

}
void CHighLife::processBlock (AudioSampleBuffer& output, MidiBuffer& midiMessages)
{
    float* outdata [NUM_OUTPUTS];
    outdata[0] = output.getSampleData (0);
    outdata[1] = output.getSampleData (1);
    
    int blockSize = output.getNumSamples();
    int replace = 1;

	// enter critical section
	crs_lock();

	if (suspended_state)
	{
	    output.clear ();

	    crs_unlock();
	    return;
	}

    // if any midi messages come in, use them to update the keyboard state object. This
    // object sends notification to the UI component about key up/down changes
    keyboard_state.processNextMidiBuffer (midiMessages,
                                          0, blockSize,
                                          true);

	// audio pointer
	int audio_ptr=0;

	// check if there're events
	/*
    if(host_plug && user_route_midi)
	{
		host_plug->dispatcher(host_plug,effProcessEvents,0,0,ev,0.0f);
	}
	else
	{
	}
	*/
	
    const unsigned char* midiData;
    int numBytesOfMidiData, samplePosition;
    MidiBuffer::Iterator ev (midiMessages);

    while (ev.getNextEvent (midiData, numBytesOfMidiData, samplePosition))
    {
		// get next block length to process
		int const block_frames = samplePosition - audio_ptr;

        // dsp dump
		if (block_frames > 0)
		{
			// block process
			plug_block_process (outdata, block_frames, replace);

			// increment output data buffers pointers
			for (int o = 0; o < NUM_OUTPUTS; o++)
				outdata[o] += block_frames;

			// increment audio ptr counter
			audio_ptr += block_frames;
		}

		// process event
		int const status = midiData[0] & 0xF0;
//			int const lchann = midiData[0] & 0xF; // unused
		int const param1 = midiData[1] & 0x7F;
		int const param2 = midiData[2] & 0x7F;

		// note off
		if(status==0x80)
			plug_note_off(param1,param2);
		
		// note on
		if(status==0x90)
		{
			// check note on velocity
			if(param2>0)
				plug_note_on(param1,param2);
			else
				plug_note_off(param1,64);
		}

		// update last polyphonic aftertouch pressure
		if(status==0xA0)
			midi_state.midi_polyaft=param2;

		// controller
		if(status==0xB0)
		{
			// update update midi cc value
			midi_state.midi_cc[param1]=param2;

			// modwheel
			if(param1==0x01)
				setParameter(2,float(param2)/127.0f);

			// all sounds off
			if(param1==0x78)
				plug_all_sounds_off();

			// all notes off
			if(param1==0x7B)
			{
				for(int v=0;v<MAX_POLYPHONY;v++)
					voice[v].release();

				for(int k=0;k<128;k++)
					user_keyb_sta[k]=0;
			}
		}

		// update program change
		if(status==0xC0)
			setCurrentProgram (param1);

		// update channel aftertouch pressure
		if(status==0xD0)
			midi_state.midi_chanaft=param1;

		// pitchbend
		if(status==0xE0) {
			midi_state.midi_bend = (param1 + (param2 << 7)) - 8192;
		}
	}

	// get remaining sample frames
	int const remaining_frames = blockSize - audio_ptr;

	// process remaining buffer when no samples
	if (remaining_frames)
		plug_block_process (outdata, remaining_frames, true);

	// leave critical section
	crs_unlock();
}

//==============================================================================
void CHighLife::getStateInformation (MemoryBlock& destData)
{
	// clear mem stream allocator
	ms_out.Reset();

	// save preset or full bank
	if (/*isPreset*/ 0)
	{
		// save only selected preset
		plug_save_program(user_program,&ms_out);
	}
	else
	{
		// save full bank
		for(int p=0;p<NUM_PROGRAMS;p++)
			plug_save_program(p,&ms_out);
	}

	// set data pointer
	destData.append (ms_out.GetData(), ms_out.GetSize());
}

void CHighLife::setStateInformation (const void* data, int sizeInBytes)
{	
	// mem stream in
	CMemStreamIn ms_in (data, sizeInBytes);

	// read preset or full bank
	if (/*isPreset*/ 0)
	{
		// read only selected preset
		plug_load_program(user_program,&ms_in);
	}
	else
	{
		// read full bank
		for(int p=0;p<NUM_PROGRAMS;p++)
			plug_load_program(p,&ms_in);
	}
}

//==============================================================================
MidiKeyboardState* CHighLife::getKeyboardstate ()
{
    return &keyboard_state;
}

//==============================================================================
AudioProcessorEditor* CHighLife::createEditor()
{
//    return new CHighLifeEditor (this);
return 0;
}

AudioProcessor* JUCE_CALLTYPE createPluginFilter (const String& commandLine)
{
    return new CHighLife();
}


//==============================================================================
float get_float_param(int const i_param,int const range)
{
	float const step_size=1.0f/(float)range;
	return (float(i_param)*step_size)+(step_size*0.5f);
}

float get_lfo_hz(float const ws)
{
	float const inv=2.0f/get_lfo_rate(ws);
	return 44100.0f/inv;
}

float get_lfo_rate(float const ws)
{
	return powf(2.0f,-(1.0f-ws)*14.0f)*0.01f;
}

int get_int_param(float const f_param,int const range)
{
	float const f_res=f_param*float(range);

	int result=int(f_res);
	
	if(result>=range)
		result=range-1;
	
	return result;
}

float get_morph(RTPAR* p,float const x)
{
	float c=p->value+p->sense*x;
	if(c<=0.0f)c=0.0f;
	if(c>=1.0f)c=1.0f;
	return c;
}

float get_env_rate(float const ws)
{
	// instant check
	if(ws<=0.0f)return 1.0f;
	
	// calculate
	return powf(2.0f,-ws*14.0f)*0.01f;
}

void format_note (int const key, char* buf)
{
	// keynote strings
	static const char str_notes[] = {"C-C#D-D#E-F-F#G-G#A-A#B-"};

    const int octaveIndex = (key % 12) * 2;
	buf[0] = str_notes[octaveIndex + 0];
	buf[1] = str_notes[octaveIndex + 1];
	buf[2] = '0' + (key / 12);
	buf[3] = 0;
}

