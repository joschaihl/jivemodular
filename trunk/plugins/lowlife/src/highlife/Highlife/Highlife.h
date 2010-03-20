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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// discoDSP Highlife Include File
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __HIGHLIFE_HEADER_H__
#define __HIGHLIFE_HEADER_H__

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MAX_BUFFER_SIZE		4096
#define MAX_POLYPHONY		32
//#define MAX_POLYPHONY		1
#define NUM_OUTPUTS			2
#define NUM_PROGRAMS		128
#define NUM_PARAMETERS		256
#define MAX_DEL_SIZE		65536
#define MAX_CHO_SIZE		4096

#define DISABLE_VST_HOST    1

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// just for porting !
#include "../JucePluginCharacteristics.h"
#include "../StandardHeader.h"

// mem stream includes
#include "../MemStream/jxstream.h"

// highlife includes
#include "HighLifeRiffWave.h"
#include "HighLifeVoice.h"
#include "HighLifeEditor.h"
#include "HighLifeFx.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CHighLife : public AudioProcessor,
                  public ChangeBroadcaster
{
public:

	CHighLife ();
	~CHighLife ();

public:

    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
	void processBlock (AudioSampleBuffer& output, MidiBuffer& midiMessages);

    const String getName() const { return JucePlugin_Name; }
    bool acceptsMidi() const { return JucePlugin_WantsMidiInput; }
    bool producesMidi() const { return JucePlugin_ProducesMidiOutput; }

    const String getInputChannelName (const int channelIndex) const;
    const String getOutputChannelName (const int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;

    int getNumParameters();
    float getParameter (int index);
    void  setParameter (int index, float newValue);
    const String getParameterName (int index);
    const String getParameterText (int index);

    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);

    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);

    AudioProcessorEditor* createEditor();
    MidiKeyboardState* getKeyboardstate ();

    juce_UseDebuggingNewOperator

public:
	void crs_lock ();
	void crs_unlock ();

public:
    void set_suspended (int const block);
    
public:
	void plug_save_program(int const index,CAllocatingMemStreamOut* pmo);
	void plug_load_program(int const index,CMemStreamIn* pmi);
	void plug_block_process(float **outputs,int const block_samples,bool const replace);
	int	 plug_get_free_voice(void);
	void plug_ramp_all_voices(void);
	void plug_note_on(uint8 const key, uint8 const vel);
	void plug_note_off(uint8 const key, uint8 const vel);
	void plug_all_sounds_off(void);
	void plug_trigger(bool const legato,int const state);

public:
	HIGHLIFE_ZONE* tool_alloc_zone(HIGHLIFE_PROGRAM* pprg);
	void tool_alloc_wave(HIGHLIFE_ZONE* pz,int const num_channels,int const num_samples);
	void tool_init_rtpar(RTPAR* pp,float const value);
	void tool_init_zone(HIGHLIFE_ZONE* pz);
	void tool_init_program(HIGHLIFE_PROGRAM* pprg);
	void tool_init_bank(void);
	void tool_delete_wave(HIGHLIFE_ZONE* pz);
	void tool_add_zone(HIGHLIFE_PROGRAM* pprg);
	void tool_delete_zone(HIGHLIFE_PROGRAM* pprg,int const zone_index);
	void tool_delete_all_zones(HIGHLIFE_PROGRAM* pprg);
	void tool_load_sample(HIGHLIFE_ZONE* pz,const File& file);
	void tool_sample_import_dlg();
	void tool_program_import_dlg();
	void tool_sample_browse_dlg(HIGHLIFE_ZONE* pz);
	int	 tool_get_num_bars(int const num_samples,int const sample_rate);
	void tool_init_dsp_buffer(float* pbuf,int const numsamples,int dfix);
	void tool_mix_dsp_buffer(float* pbuf_src,float* pbuf_dest,int const numsamples);
	void tool_copy_dsp_buffer(float* pbuf_src,float* pbuf_dest,int const numsamples);

#ifndef DISABLE_VST_HOST
public:
	bool host_instance_vst(const char* dll_path);
	void host_open_vst_dialog();
	void host_instance_free(void);
	void host_plug_midi_send(uint8 md0, uint8 md1, uint8 md2, uint8 md3, int const deltaframe);
	void host_generate_preset();
	void host_open_plugin_editor();
	void host_close_plugin_editor(void);
	void host_process_mix (float* psamplesl, float* psamplesr, int const num_samples);
#endif

public:
    void mp3_load (HIGHLIFE_ZONE* pz, const File& file);
	void wav_load (HIGHLIFE_ZONE* pz, const File& file);
	void ogg_load (HIGHLIFE_ZONE* pz, const File& file);
	void raw_load (HIGHLIFE_ZONE* pz, const File& file);

public:
	void wav_save (HIGHLIFE_ZONE* pz, const File& file);
	void wav_export();

public:
	void akp_import (const File& file);
    void gig_import (const File& file);
    void dls_import (const File& file);
	void sf2_import (const File& file);
	void sfz_import (const File& file);

public:
	void sfz_export();

public:
	void sed_sel_vol_change(float const a,float const b);
	void sed_sel_1st_order_iir(float const fc);
	void sed_sel_normalize(void);
	void sed_sel_dc_remove(void);
	void sed_sel_reverse(void);
	void sed_sel_rectify(void);
	void sed_sel_mathdrive(int const mode);
	void sed_sel_cut_marker(int& position);
	void sed_sel_cut(void);
	void sed_sel_copy(void);
	void sed_sel_paste(void);
	void sed_sel_trim_marker(int& position);
	void sed_sel_trim(void);
	void sed_crossfade_loop(float* pbuffer,int const loop_start,int const loop_length);
	void sed_normalize(float* pbuffer,int const numsamples);
	void sed_clipboard_reset(void);
	void sed_add_cue(void);
	void sed_sel_remove_cues(void);
    void sed_time_stretch (double const pitchShift, double const timeRatio);

public:
	// critical section
	CriticalSection critical_section;
	int     suspended_state;

	// user gui vars
	int		user_gui_page;
	int		user_last_key;
	int		user_keyb_sta[128];
	int		user_pressed;
	int		user_program;
	float	user_master_volume;
	int		user_ox;
	int		user_oy;
	int		user_delta_y_accum;

	// user wave clipboard
	int		user_clip_sample_size;
	int		user_clip_sample_channels;
	float** user_clip_sample;

	// gui flags
	bool	gui_recent_update;

	// sample editor
	int		user_sed_manual;
	int		user_sed_zone;
	int		user_sed_cue;
	int		user_editor_mode;
	int		user_sed_offset;
	int		user_sed_spp;
	int		user_sed_adapt;
	int		user_sed_sel_sta;
	int		user_sed_sel_len;

	// freezing user settings
	int		user_prg_splits;
	int		user_vel_splits;
	float	user_split_time;
	int		user_ste_keyboa;
	int		user_low_keyboa;
	int		user_hig_keyboa;
	int		user_route_midi;	
	int		user_force_mono;
	int		user_normalization;

	// wave processing counter info
	int		wave_processing;
	int		wave_total;
	int		wave_counter;
	char	wave_buf[1024];

	// bank
	HIGHLIFE_PROGRAM highlife_program[NUM_PROGRAMS];

	// midi state
	HIGHLIFE_INPUT_STATE midi_state;
	
	// highlife voices
	CHighLifeVoice	voice[MAX_POLYPHONY];
	int				vt_opos[MAX_POLYPHONY];

	// fx
	CFxChorus		fx_cho;
	CFxDelay		fx_del;
	CFxReverb		fx_rev;
	CFxPhaser		fx_phs_l;
	CFxPhaser		fx_phs_r;
	CFxCompressor	fx_cmp_l;
	CFxCompressor	fx_cmp_r;

	// demo noise counter
	int		noise_count;

	// mix buffers
	float	mixer_buffer[NUM_OUTPUTS][MAX_BUFFER_SIZE];
	float	fxchr_buffer[NUM_OUTPUTS][MAX_BUFFER_SIZE];
	float	fxdel_buffer[NUM_OUTPUTS][MAX_BUFFER_SIZE];
	float	fxrev_buffer[NUM_OUTPUTS][MAX_BUFFER_SIZE];

	// host
#ifndef DISABLE_VST_HOST
	AEffect*		host_plug;
	bool			host_plug_can_process;
	int				host_plug_queue_size;
	VstMidiEvent	host_plug_event[MAX_BLOCK_EVENTS];
#endif

    // keyboard state
    MidiKeyboardState keyboard_state;

	// input/output
	CAllocatingMemStreamOut	ms_out;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
float get_float_param(int const i_param,int const range);
float get_lfo_hz(float const ws);
void format_note (int const key, char* buf);

#endif

