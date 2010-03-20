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
// HighLife Tools Implementation                                                                                                       //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_init_rtpar(RTPAR* pp,float const value)
{
	pp->value=value;
	pp->sense=0.0f;
	pp->ctrln=1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_init_zone(HIGHLIFE_ZONE* pz)
{
	// init input state
	HIGHLIFE_INPUT_STATE* pic_lo=&pz->lo_input_range;
	HIGHLIFE_INPUT_STATE* pic_hi=&pz->hi_input_range;

	// init ic keyrange
	pic_lo->midi_key=0;
	pic_hi->midi_key=119; // TODO - what if we are too high ?

	// init ic velrange
	pic_lo->midi_vel=0;
	pic_hi->midi_vel=127;

	// init ic controllers
	for(int cc=0;cc<128;cc++)
	{
		pic_lo->midi_cc[cc]=0;
		pic_hi->midi_cc[cc]=127;
	}

	// init ic bend
	pic_lo->midi_bend=-8192;
	pic_hi->midi_bend=8192;
	
	// init ic channel aftertouch
	pic_lo->midi_chanaft=0;
	pic_hi->midi_chanaft=127;

	// init ic poly aftertouch
	pic_lo->midi_polyaft=0;
	pic_hi->midi_polyaft=127;

	// format new zone
	sprintf(pz->name,"Untitled");
//	sprintf(pz->path,"Not Available");
   pz->path[0] = 0;
   
	// prepare sample properties
	pz->num_channels=0;
	pz->num_samples=0;
	pz->sample_rate=44100;
	pz->sample_reserved=0;
	
	// expansion
	pz->res_group=0;
	pz->res_offby=0;
	pz->res_future0=0;
	pz->res_future1=0;
	
	// reset cue points
	for(int c=0;c<MAX_CUE_POINTS;c++)
		pz->cue_pos[c]=0;

	pz->num_cues=0;

	// reset sample loop
	pz->loop_start=0;
	pz->loop_end=0;
	pz->loop_mode=0;
	pz->loop_reserved=0;

	// init midi properties
	pz->midi_coarse_tune=0;
	pz->midi_root_key=60;
	pz->midi_fine_tune=0;
	pz->midi_trigger=0;
	pz->midi_keycents=100;

	// zone parameters
	pz->mp_gain=0;
	pz->mp_pan=0;
	pz->mp_synchro=0;
	pz->mp_num_ticks=4;
	
	// null pointers
	pz->ppwavedata=NULL;

	pz->vel_amp=1.0f;
	pz->vel_mod=0.0f;
	pz->sys_pit_rng=200.0f/2400.0f;
	pz->mod_lfo_syn=0;
	
	// glide reset
	pz->glide_auto=0.0f;
	tool_init_rtpar(&pz->glide,0.0f);
	
	// filter reset
	tool_init_rtpar(&pz->flt_cut_frq,1.0f);
	tool_init_rtpar(&pz->flt_res_amt,0.0f);
	pz->flt_type=0.0f;
	pz->flt_kbd_trk=0.0f;
	pz->flt_vel_trk=0.0f;
	
	// mod lfo reset
	tool_init_rtpar(&pz->mod_lfo_phs,1.0f);
	tool_init_rtpar(&pz->mod_lfo_rat,0.7f);
	tool_init_rtpar(&pz->mod_lfo_amp,0.0f);
	tool_init_rtpar(&pz->mod_lfo_cut,0.5f);
	tool_init_rtpar(&pz->mod_lfo_res,0.5f);
	tool_init_rtpar(&pz->mod_lfo_pit,0.5f);
	
	// mod env reset
	tool_init_rtpar(&pz->mod_env_att,0.0f);
	tool_init_rtpar(&pz->mod_env_dec,0.5f);
	tool_init_rtpar(&pz->mod_env_sus,0.5f);
	tool_init_rtpar(&pz->mod_env_rel,0.1f);
	tool_init_rtpar(&pz->mod_env_pit,0.5f);
	tool_init_rtpar(&pz->mod_env_cut,0.5f);

	// amplifier reset
	tool_init_rtpar(&pz->amp_env_att,0.0f);
	tool_init_rtpar(&pz->amp_env_dec,0.5f);
	tool_init_rtpar(&pz->amp_env_sus,1.0f);
	tool_init_rtpar(&pz->amp_env_rel,0.2f);
	tool_init_rtpar(&pz->amp_env_amt,1.0f);

	// reset fx send levels
	tool_init_rtpar(&pz->efx_chr_lev,0.0f);
	tool_init_rtpar(&pz->efx_del_lev,0.0f);
	tool_init_rtpar(&pz->efx_rev_lev,0.0f);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_init_program(HIGHLIFE_PROGRAM* pprg)
{
	// delete all zones
	tool_delete_all_zones(pprg);

	// memset zero
	memset((void*)pprg,0,sizeof(HIGHLIFE_PROGRAM));

	// default program name
	sprintf(pprg->name,"---");

	// reset daft fx
	pprg->efx_dft=0;
	pprg->efx_dft_frq=0.4f;

	// reset fx chorus
	pprg->efx_chr=0;
	pprg->efx_chr_del=0.3f;
	pprg->efx_chr_fdb=0.0f;
	pprg->efx_chr_rat=0.3f;
	pprg->efx_chr_mod=0.5f;
	
	// reset delay fx
	pprg->efx_del=0;
	pprg->efx_del_del=0.5f;
	pprg->efx_del_fdb=0.5f;
	pprg->efx_del_cro=0;
	pprg->efx_del_syn=0;
	
	// reset reverb fx
	pprg->efx_rev=0;
	pprg->efx_rev_roo=0.75f;
	pprg->efx_rev_wid=0.75f;
	pprg->efx_rev_dam=0.25f;

	// reset rck fx
	pprg->efx_rck=0;
	
	// default playmode (polyphonic)
	pprg->ply_mode=2;	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_init_bank(void)
{
	for(int p=0;p<NUM_PROGRAMS;p++)
		tool_init_program(&highlife_program[p]);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_delete_wave(HIGHLIFE_ZONE* pz)
{
	// delete channels wavedata
	for(int c=0;c<pz->num_channels && pz->ppwavedata;c++)
		delete[] pz->ppwavedata[c];

	// delete channels
	delete[] pz->ppwavedata;
	pz->ppwavedata=NULL;

	// zero samples and zero channels
	pz->num_channels=0;
	pz->num_samples=0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_add_zone (HIGHLIFE_PROGRAM* pprg)
{
	// get program numzones
	int const nz = pprg->num_zones;

	// create new zone array
	HIGHLIFE_ZONE* pnz = new HIGHLIFE_ZONE [nz + 1];

	// copy all except current zone
	for (int z = 0; z < nz; z++)
	{
		pnz[z] = pprg->pzones[z];
	}

    // initialize the zone
    tool_init_zone (& pnz[nz]);

	// set new zone array
	delete[] pprg->pzones;
	
	// set new zone array
	pprg->pzones = pnz;
	pprg->num_zones = nz + 1;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_delete_zone(HIGHLIFE_PROGRAM* pprg,int const zone_index)
{
	// get program numzones
	int const nz = pprg->num_zones;

	// create new zone array
	if (nz > 0)
	{
		// create new zone array
		HIGHLIFE_ZONE* pnz=new HIGHLIFE_ZONE[nz-1];

		// copy all except current zone
		int zc = 0;
		for (int z = 0; z < nz; z++)
		{
			if(z != zone_index)
				pnz[zc++] = pprg->pzones[z];
		}

		// delete the wavedata of the zone to delete
		tool_delete_wave (&pprg->pzones[zone_index]);

		// set new zone array
		delete[] pprg->pzones;
		
		// set new zone array
		pprg->pzones = pnz;
		pprg->num_zones = nz - 1;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_delete_all_zones(HIGHLIFE_PROGRAM* pprg)
{
	// delete all wavedata
	for(int w=0;w<pprg->num_zones;w++)
		tool_delete_wave(&pprg->pzones[w]);

	// delete zone array
	delete[] pprg->pzones;
	pprg->pzones=NULL;

	// set 0 zones
	pprg->num_zones=0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CHighLife::tool_get_num_bars(int const num_samples,int const sample_rate)
{
	int const samples_per_bar=(sample_rate*240)/120;
	double d_bars_in_sample=double(num_samples)/double(samples_per_bar);

	double d_b_floor=floor(d_bars_in_sample);
	double d_b_ceili=ceil(d_bars_in_sample);

	double d_l=d_bars_in_sample-d_b_floor;
	double d_r=d_b_ceili-d_bars_in_sample;

	if(d_l>d_r)
		return int(d_b_ceili);

	return int(d_b_floor);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HIGHLIFE_ZONE* CHighLife::tool_alloc_zone(HIGHLIFE_PROGRAM* pprg)
{
	// create new zone array
	HIGHLIFE_ZONE* pnew_zones=new HIGHLIFE_ZONE[pprg->num_zones+1];

	// copy old zones to new zone array
	for(int z=0;z<pprg->num_zones;z++)
		pnew_zones[z]=pprg->pzones[z];

	// delete old wave array
	delete[] pprg->pzones;
	pprg->pzones=NULL;
	
	// assign new zone data
	pprg->pzones=pnew_zones;

	// get new zone pointer
	HIGHLIFE_ZONE* pnz=&pnew_zones[pprg->num_zones];

	// reset zone
	tool_init_zone(pnz);

	// increment num waves
	pprg->num_zones++;

	// return new wave pointer
	return pnz;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_alloc_wave(HIGHLIFE_ZONE* pz,int const num_channels,int const num_samples)
{
	// set num channels and samples
	pz->num_channels=num_channels;
	pz->num_samples=num_samples;
	
	// get num pad samples
	int const num_pad_samples=pz->num_samples+(WAVE_PAD*2);

	// allocate channels
	pz->ppwavedata=new float*[pz->num_channels];

	// init allocate silenced samples
	for(int c=0;c<pz->num_channels;c++)
	{
		pz->ppwavedata[c]=new float[num_pad_samples];
		tool_init_dsp_buffer(pz->ppwavedata[c],num_pad_samples,0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_load_sample (HIGHLIFE_ZONE* pz, const File& file)
{
	// format zone name
    strncpy (pz->name, (const char*) file.getFileNameWithoutExtension (), MAX_PATH);

	// format zone path
	sprintf (pz->path, (const char*) file.getFullPathName ());

	// check file extension and open preferred format
	if (file.getFileExtension ().compareIgnoreCase (T(".wav")) == 0)
	{
		// wav file
		wav_load (pz, file);
	}
#if 0
	else if(file.getFileExtension ().compareIgnoreCase (T(".mp3")) == 0)
	{
		// mp3 file
		mp3_load (pz, file);
	}
	else if(file.getFileExtension ().compareIgnoreCase (T(".ogg")) == 0)
	{
		// ogg file
		ogg_load (pz, file);
	}
	else
	{
		// raw file
		raw_load (pz, file);
	}
#endif

	// analyze and set num ticks (for synchro)
	pz->mp_num_ticks = tool_get_num_bars (pz->num_samples, pz->sample_rate) * 16;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_sample_import_dlg()
{
    FileChooser myChooser ("discoDSP HighLife - Import Sample File(s)",
                           File::nonexistent,
                           "*.wav;*.ogg;*.mp3;*.raw");

#if 0
    if (myChooser.browseForMultipleFilesToOpen ())
#else
    if (myChooser.browseForFileToOpen ())
#endif
    {
#if 0
        const OwnedArray<File>* files = &myChooser.getResults ();

		// get num files
		int const num_files = files->size ();
#else
        File result = myChooser.getResult();
		int const num_files = 1;
#endif

		// check all files
		bool check=true;

#if 0
		for(int f=0;f<num_files;f++)
		{
		    File* file = files->getUnchecked (f);

		    if (! file->existsAsFile ())
		    {
		        check = false;
		        break;
		    }
		}
#else
        check = result.existsAsFile ();
#endif       

		// open if some files selected
		if(num_files && check)
		{
			// wave spare flag
			bool dm_wave_spare=false;

			// ask for autospread
			if(num_files > 1
			   && AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon,
			                                       T("discoDSP HighLife"),
			                                       T("Use Auto-Drum-Machine Spread?")) != 1)
		    {
				dm_wave_spare=true;
	        }
 
			// enter critical section
			set_suspended (1);

			// get program and wavetable pointers
			HIGHLIFE_PROGRAM* pprog=&highlife_program[user_program];

			// format program title
#if 0
		    File* file = files->getUnchecked (0);
#endif
            strncpy (pprog->name, (const char*) result.getFileNameWithoutExtension (), 31);

			// load each wavefile
			for(int wf=0;wf<num_files;wf++)
			{
#if 0
				File* sampleFile = files->getUnchecked (wf);
#else
                File sampleFile = result;
#endif

				// append new zones to program
				HIGHLIFE_ZONE* pz = tool_alloc_zone(pprog);
				tool_load_sample (pz, sampleFile);

				// drum machine spare
				if(dm_wave_spare)
				{
					// compute mapping keystart
					int spare_start=120-num_files;

					// clamp
					if(spare_start>60)spare_start=60;
					if(spare_start<0)spare_start=0;

					int const i_keys_per_drumhit=1; // user_kbd_splits;
					int const dm_key=spare_start+wf*i_keys_per_drumhit;
					pz->lo_input_range.midi_key=dm_key;
					pz->hi_input_range.midi_key=dm_key+(i_keys_per_drumhit-1);
					pz->midi_root_key=dm_key+(i_keys_per_drumhit/2);
				}
			}

			// sample editor should adapt
			user_sed_zone=pprog->num_zones-1;
			user_sed_adapt=1;

			// leave critical setion
			set_suspended (0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_program_import_dlg()
{
    FileChooser myChooser ("discoDSP HighLife - Import Program File",
                           File::nonexistent,
                           "*.akp;*.sfz;*.sf2;*.gig;*dls");

    if (myChooser.browseForFileToOpen ())
    {
        File file = myChooser.getResult ();

        if (file.existsAsFile ())
        {
	        if (file.getFileExtension ().compareIgnoreCase (T(".akp")) == 0)
	        {
		        akp_import(file);
	        }
	        else if(file.getFileExtension ().compareIgnoreCase (T(".sfz")) == 0)
	        {
		        sfz_import(file);
	        }
	        else if(file.getFileExtension ().compareIgnoreCase (T(".sf2")) == 0)
	        {
		        sf2_import (file);
	        }
	        else if(file.getFileExtension ().compareIgnoreCase (T(".gig")) == 0)
	        {
		        gig_import (file);
	        }
	        else if(file.getFileExtension ().compareIgnoreCase (T(".dls")) == 0)
	        {
		        dls_import (file);
	        }
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_sample_browse_dlg(HIGHLIFE_ZONE* pz)
{
    FileChooser myChooser ("discoDSP HighLife - Browse Sample File",
                           File::nonexistent,
                           "*.wav;*.ogg;*.mp3;*.raw");

    if (myChooser.browseForFileToOpen ())
    {
        File result = myChooser.getResult ();

        if (result.existsAsFile ())
        {
			// enter critical section
			set_suspended (1);

			// load new wave in zone
			tool_delete_wave (pz);
			tool_load_sample (pz, result);

			// sample editor should adapt
			user_sed_adapt=1;

			// leave critical section
			set_suspended (0);
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_init_dsp_buffer(float* pbuf,int const numsamples,int dfix)
{
	if(dfix)
	{
		float const f_ad=1.0e-14f;

		for(int s=0;s<numsamples;s++)
			pbuf[s]=rand()*f_ad;
	}
	else
	{
		for(int s=0;s<numsamples;s++)
			pbuf[s]=0.0f;
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_mix_dsp_buffer(float* pbuf_src,float* pbuf_dest,int const numsamples)
{
	for(int s=0;s<numsamples;s++)
		pbuf_dest[s]+=pbuf_src[s];
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::tool_copy_dsp_buffer(float* pbuf_src,float* pbuf_dest,int const numsamples)
{
	for(int s=0;s<numsamples;s++)
		pbuf_dest[s]=pbuf_src[s];
}

