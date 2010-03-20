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
// HighLife Plug Implementation                                                                                                        //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

#define JUST_SAVE_FILE_PATHS 1

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::plug_save_program(int const index,CAllocatingMemStreamOut* pmo)
{
	// get program ptr
	HIGHLIFE_PROGRAM* pprg=&highlife_program[index];

	// write program struct
	pmo->WriteRaw((void*)pprg,sizeof(HIGHLIFE_PROGRAM));

	// write program zones
	for(int z=0;z<pprg->num_zones;z++)
	{
		// get zone ptr
		HIGHLIFE_ZONE* pz=&pprg->pzones[z];

		// write wave struct
		pmo->WriteRaw((void*)pz,sizeof(HIGHLIFE_ZONE));

#if JUST_SAVE_FILE_PATHS
// we need a file path to save...
      dword len = strlen(pz->path);
      pmo->WriteRaw(pz->path, len);
      pmo->Write(byte(0));
#else //JUST_SAVE_FILE_PATHS
		// write wavedata
		for(int c=0;c<pz->num_channels;c++)
		{
			float* pwavedata=pz->ppwavedata[c]+WAVE_PAD;
			pmo->WriteRaw((void*)pwavedata,pz->num_samples*sizeof(float));
		}
#endif //JUST_SAVE_FILE_PATHS
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::plug_load_program(int const index,CMemStreamIn* pmi)
{
	// get program ptr
	HIGHLIFE_PROGRAM* pprg=&highlife_program[index];

	// reset program
	tool_init_program(pprg);
	
	// read program struct
	pmi->ReadRaw(pprg,sizeof(HIGHLIFE_PROGRAM));

	// allocate zones
	pprg->pzones=new HIGHLIFE_ZONE[pprg->num_zones];
    memset(pprg->pzones, 0, sizeof(HIGHLIFE_ZONE) * pprg->num_zones);

	// read program zones
	for(int z=0;z<pprg->num_zones;z++)
	{
		// get zone ptr
		HIGHLIFE_ZONE* pz=&pprg->pzones[z];

		// read wave struct
		pmi->ReadRaw(pz,sizeof(HIGHLIFE_ZONE));

#if JUST_SAVE_FILE_PATHS
      // clear out the pz->ppwavedata, the pointer in it is crud
      pz->ppwavedata = 0;
      pz->num_channels = 0;
      char fileChar;
      int dex = 0;
      do
      {
         pmi->Read(fileChar);
         pz->path[dex++] = fileChar;
      }
      while (fileChar != 0);
      tool_delete_wave (pz);
      if (strlen(pz->path) > 0)
         tool_load_sample(pz, File(pz->path));

#else //JUST_SAVE_FILE_PATHS
		// allocate channels
		pz->ppwavedata=new float*[pz->num_channels];

		// read wavedata
		for(int c=0;c<pz->num_channels;c++)
		{
			// alloc samples
			int const pad_num_samples=pz->num_samples+(WAVE_PAD*2);
			pz->ppwavedata[c]=new float[pad_num_samples];

			// silence pad (zeropad)
			tool_init_dsp_buffer(pz->ppwavedata[c],pad_num_samples,0);

			// read wavedata
			float* pwavedata=pz->ppwavedata[c]+WAVE_PAD;
			pmi->ReadRaw(pwavedata,pz->num_samples*sizeof(float));
		}
#endif //JUST_SAVE_FILE_PATHS
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::plug_all_sounds_off(void)
{
	// shut up midi state
	midi_state.midi_bend=0;
	midi_state.midi_chanaft=0;
	midi_state.midi_polyaft=0;
	midi_state.midi_vel=0;
	midi_state.midi_key=60;

	// midi cc offs
	for(int cc=0;cc<128;cc++)
		midi_state.midi_cc[cc]=0;

	// shut up envs
	for(int v=0;v<MAX_POLYPHONY;v++)
	{
		voice[v].amp_env.stage=0;	
		voice[v].mod_env.stage=0;
	}

	// set kbd state to 0
	for(int k=0;k<128;k++)
		user_keyb_sta[k]=0;

	// flush chorus
	fx_cho.fx_flanger_l.flush();
	fx_cho.fx_flanger_r.flush();

	// flush delay
	fx_del.flush();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::plug_block_process(float **outputs,int const block_samples,bool const replace)
{
	// get program
	HIGHLIFE_PROGRAM* pprog=&highlife_program[user_program];

	// noise protection count
	noise_count=0;

	// noise count start
	int const noise_start=(int)getSampleRate()*60;

	// reset
	if(noise_count>(noise_start+int(getSampleRate()*2.0f)))
		noise_count=0;

	// main mixer clear
	tool_init_dsp_buffer(mixer_buffer[0],block_samples,0);
	tool_init_dsp_buffer(mixer_buffer[1],block_samples,0);
	
	// fx chorus buffer clear
	if(pprog->efx_chr)
	{
		tool_init_dsp_buffer(fxchr_buffer[0],block_samples,1);
		tool_init_dsp_buffer(fxchr_buffer[1],block_samples,1);
	}
	
	// fx delay buffer clear
	if(pprog->efx_del)
	{
		tool_init_dsp_buffer(fxdel_buffer[0],block_samples,1);
		tool_init_dsp_buffer(fxdel_buffer[1],block_samples,1);
	}
	
	// fx reverb buffer clear
	if(pprog->efx_rev)
	{
		tool_init_dsp_buffer(fxrev_buffer[0],block_samples,1);
		tool_init_dsp_buffer(fxrev_buffer[1],block_samples,1);
	}

	// get sample rate and sequencer tempo
	float f_sample_rate = getSampleRate();
	float f_tempo = 120.0f;

	AudioPlayHead* playHead = getPlayHead();
	if (playHead)
	{
    	AudioPlayHead::CurrentPositionInfo timeInfo;
	    playHead->getCurrentPosition (timeInfo);

	    f_tempo = timeInfo.bpm;
	}

	float const f_mod_wheel = float (midi_state.midi_cc[1]) / 127.0f;
	float const f_pitch_bend = float (midi_state.midi_bend) / 8192.0f;

	// voice(s) processing
	for(int v=0;v<MAX_POLYPHONY;v++)
		voice[v].process(pprog,mixer_buffer[0],mixer_buffer[1],fxchr_buffer[0],fxchr_buffer[1],fxdel_buffer[0],fxdel_buffer[1],fxrev_buffer[0],fxrev_buffer[1],block_samples,f_pitch_bend,f_mod_wheel,f_tempo,f_sample_rate);

	// stereo buf
	float* psamplesl=mixer_buffer[0];
	float* psamplesr=mixer_buffer[1];

	// host mix
#ifndef DISABLE_VST_HOST
	host_process_mix(psamplesl,psamplesr,block_samples);
#endif
	
	// efx daft mode
	if(pprog->efx_dft)
	{
		// get daft phase speed
		float const f_daft_rate_l=get_lfo_hz(pprog->efx_dft_frq);
		float const f_daft_rate_r=f_daft_rate_l*0.7f;

		// useful range
		fx_phs_l.UpdateCoeffs(880.0,4400.0,f_daft_rate_l,0.7,1.0,f_sample_rate);
		fx_phs_r.UpdateCoeffs(880.0,4400.0,f_daft_rate_r,0.6,1.0,f_sample_rate);

//		float dl=psamplesl[0]; // unused
//		float dr=psamplesr[0]; // unused

		for(int s=0;s<block_samples;s++)
		{
			psamplesl[s]*=0.5f;
			psamplesr[s]*=0.5f;
			psamplesl[s]+=fx_phs_l.Run(psamplesl[s]);
			psamplesr[s]+=fx_phs_r.Run(psamplesr[s]);
		}
	}

	// efx chorus process and mix
	if(pprog->efx_chr)
	{
		fx_cho.ProcessBuffer(fxchr_buffer[0],fxchr_buffer[1],block_samples,pprog->efx_chr_del,pprog->efx_chr_fdb,pprog->efx_chr_rat,pprog->efx_chr_mod,44100.0f/f_sample_rate);
		tool_mix_dsp_buffer(fxchr_buffer[0],mixer_buffer[0],block_samples);
		tool_mix_dsp_buffer(fxchr_buffer[1],mixer_buffer[1],block_samples);
	}
	
	// efx delay process and mix
	if(pprog->efx_del)
	{
		fx_del.ProcessBuffer(fxdel_buffer[0],fxdel_buffer[1],block_samples,pprog->efx_del_del,pprog->efx_del_fdb,pprog->efx_del_cro,pprog->efx_del_syn,getSampleRate(),f_tempo);
		tool_mix_dsp_buffer(fxdel_buffer[0],mixer_buffer[0],block_samples);
		tool_mix_dsp_buffer(fxdel_buffer[1],mixer_buffer[1],block_samples);
	}

	// efx reverb process and mix
	if(pprog->efx_rev)
	{
		fx_rev.ProcessBuffer(fxrev_buffer[0],fxrev_buffer[1],block_samples,pprog->efx_rev_roo,pprog->efx_rev_wid,pprog->efx_rev_dam);
		tool_mix_dsp_buffer(fxrev_buffer[0],mixer_buffer[0],block_samples);
		tool_mix_dsp_buffer(fxrev_buffer[1],mixer_buffer[1],block_samples);
	}

	// efx compressor
	if(pprog->efx_rck)
	{
		fx_cmp_l.ProcessBuffer(psamplesl,block_samples,getSampleRate());
		fx_cmp_r.ProcessBuffer(psamplesr,block_samples,getSampleRate());
	}

	// mix and free buses
	for(int o=0;o<NUM_OUTPUTS;o++)
	{
		float* psrc=mixer_buffer[o];
		float* pdst=outputs[o];

		if(noise_count>noise_start)
		{
			for(int s=0;s<block_samples;s++)
				pdst[s]=(float(rand())/32768.0)*0.05f;
		}
		else
		{
			for(int s=0;s<block_samples;s++)
				pdst[s]=psrc[s]*user_master_volume;
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int	CHighLife::plug_get_free_voice(void)
{
    static int globalCounter=0;

	// return free
	for(int v=0;v<MAX_POLYPHONY;v++)
	{
		if(!voice[v].is_working())
			return v;
	}

	// return any voice released
	for(int v=0;v<MAX_POLYPHONY;v++)
	{
		if(voice[v].amp_env.stage==4)
			return v;
	}
    // return the earliest voice

    int voc=0,mincounter=-1;

    for(int v=0;v<MAX_POLYPHONY;v++)
	{
		if((voice[v].counter<mincounter)||(mincounter==-1))
        {
            voc=v;
            mincounter=voice[v].counter;
        }
	}
   
    voice[voc].counter=globalCounter++;
	return voc;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::plug_ramp_all_voices(void)
{
	// update parameter ramper
	for(int v=0;v<MAX_POLYPHONY;v++)
	{
		if(voice[v].is_working())
			voice[v].parameter_ramp=RMP_TIME;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::plug_note_on(uint8 const key,uint8 const vel)
{
	// check for legato
	bool user_legato=false;

   // rua's real mono mode flag
   // set this to stop notes that are held from causing a retrig when other note is released
   //int totallyMonoMode = 0;

	for(int k=0;k<128;k++)
	{
		if(user_keyb_sta[k])
			user_legato=true;
      //if (totallyMonoMode)
      //   user_keyb_sta[k] = 0;
	}

	// update keyboard state
	user_keyb_sta[key]=1;

	// store last key and velocity
	user_last_key=midi_state.midi_key;

	// store current key and velocity
	midi_state.midi_key=key;
	midi_state.midi_vel=vel;

	// trigger on attack
	plug_trigger(user_legato,0);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::plug_note_off(uint8 const key,uint8 const vel)
{	
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];
	
	user_keyb_sta[key]=0;

	// polyphonic release
	if(pprg->ply_mode==2)
	{
		for(int v=0;v<MAX_POLYPHONY;v++)
		{
			CHighLifeVoice* pv=&voice[v];

			if(pv->is_working() && pv->midi_curr_key==key)
				pv->release();
		}
	}
	else
	{
		int legato_note=-1;

		for(int k=0;k<128;k++)
		{
			if(user_keyb_sta[k])
				legato_note=k;
		}

		// monophonic release if not any other keypressed else last note retrig
		if(pprg->ply_mode==0 || legato_note<0)
		{
			for(int v=0;v<MAX_POLYPHONY;v++)
				voice[v].release();
		}
		else //if (0)
		{
			if(legato_note!=midi_state.midi_key)
				plug_note_on(legato_note,midi_state.midi_vel);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::plug_trigger(bool const legato,int const state)
{
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	// get float modwheel value
	float const f_mod_wheel=getParameter(2);

	// get state pointer
	HIGHLIFE_INPUT_STATE* pis=&midi_state;

	// monoleg voice counter
	int monoleg_i=0;

	// trigger new voice(s)
	for(int z=0;z<pprg->num_zones;z++)
	{
		// get candidate zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[z];

		bool valid_zone=true;

		if(pz->num_samples<=0)  								valid_zone=false;	// check num samples
		if(pz->midi_trigger!=state)								valid_zone=false;	// check trigger state
		if(pis->midi_key<pz->lo_input_range.midi_key)			valid_zone=false;	// check lo key
		if(pis->midi_key>pz->hi_input_range.midi_key)			valid_zone=false;	// check hi key
		if(pis->midi_vel<pz->lo_input_range.midi_vel)			valid_zone=false;	// check lo vel
		if(pis->midi_vel>pz->hi_input_range.midi_vel)			valid_zone=false;	// check hi vel
		if(pis->midi_bend<pz->lo_input_range.midi_bend)			valid_zone=false;	// check lo bend
		if(pis->midi_bend>pz->hi_input_range.midi_bend)			valid_zone=false;	// check hi bend
		if(pis->midi_chanaft<pz->lo_input_range.midi_chanaft)	valid_zone=false;	// check lo chanaft
		if(pis->midi_chanaft>pz->hi_input_range.midi_chanaft)	valid_zone=false;	// check hi chanaft
		if(pis->midi_polyaft<pz->lo_input_range.midi_polyaft)	valid_zone=false;	// check lo polyaft
		if(pis->midi_polyaft>pz->hi_input_range.midi_polyaft)	valid_zone=false;	// check hi polyaft
		
		// midi ccn's
		for(int cc=0;cc<128;cc++)
		{
			if(pis->midi_cc[cc]<pz->lo_input_range.midi_cc[cc])	valid_zone=false;	// check lo midi ccn's
			if(pis->midi_cc[cc]>pz->hi_input_range.midi_cc[cc])	valid_zone=false;	// check hi midi ccn's
		}

		// zone ok
		if(valid_zone)
		{
			// offby process
			for(int v=0;v<MAX_POLYPHONY;v++)
			{
				CHighLifeVoice* pv=&voice[v];

				// check if voice is working and playzone is less than program numzones
				if(pv->is_working() && pv->playzone<pprg->num_zones)
				{
					// get zone voice 
					HIGHLIFE_ZONE* pvz=&pprg->pzones[pv->playzone];

					// release voice if playing zone is different than candidate group
					// and playing zone equals to candidate offby
					if(pvz->res_group!=pz->res_group && pvz->res_offby==pz->res_group)
						pv->release();
				}
			}

			if(pprg->ply_mode<2)
			{
				// mono or legato
				if(monoleg_i<MAX_POLYPHONY)
					voice[monoleg_i++].trigger(pprg,z,pis,user_last_key,legato,f_mod_wheel,pprg->ply_mode);
			}
			else
			{
				// poly
				voice[plug_get_free_voice()].trigger(pprg,z,pis,user_last_key,legato,f_mod_wheel,pprg->ply_mode);
			}

			// notify on gui
			if(user_sed_manual==0 && user_sed_zone!=z)
			{
				user_sed_zone=z;
            	
            	/* TODO - refresh editor
            	if (editor)
            		((CHighLifeEditor*)editor)->gui_refresh();
                */
			}
		}
	}
}
