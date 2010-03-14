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
// HighLife Sample Editor Ops Implementation                                                                                           //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// #include "RubberBandStretcher.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_vol_change(float const a,float const b)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			float const f_step=(b-a)/float(user_sed_sel_len);
			float val=a;

			for(int s=0;s<user_sed_sel_len;s++)
			{
				int const sample_pos=WAVE_PAD+user_sed_sel_sta+s;

				for(int c=0;c<pz->num_channels;c++)
					pz->ppwavedata[c][sample_pos]*=val;
				
				val+=f_step;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_normalize(float* pbuffer,int const numsamples)
{
	// check max peak
	float max_peak=0.1f;

	for(int s=0;s<numsamples;s++)
	{
		float const peak=fabsf(pbuffer[s]);

		if(peak>max_peak)
			max_peak=peak;
	}

	// normalization
	float const normal_coef=1.0f/max_peak;

	for(int s=0;s<numsamples;s++)
		pbuffer[s]*=normal_coef;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_normalize(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// scan highest peak
			float f_highest_peak=0.0f;

			for(int s=0;s<user_sed_sel_len;s++)
			{
				int const sample_pos=WAVE_PAD+user_sed_sel_sta+s;

				for(int c=0;c<pz->num_channels;c++)
				{
					float const f_energy=fabsf(pz->ppwavedata[c][sample_pos]);
				
					if(f_energy>f_highest_peak)
						f_highest_peak=f_energy;
				}
			}

			// normalize
			if(f_highest_peak>0.0f)
			{
				float const f_gain=1.0f/f_highest_peak;
				sed_sel_vol_change(f_gain,f_gain);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_1st_order_iir(float const fc)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// filter each channel
			for(int c=0;c<pz->num_channels;c++)
			{			
				float* pwavedat=pz->ppwavedata[c]+WAVE_PAD+user_sed_sel_sta;
				
				CFxSimpleFilter flt;

				for(int s=0;s<user_sed_sel_len;s++)
					pwavedat[s]=flt.process_sample(pwavedat[s],fc);
			}
		}
	}

	// post normalization
	sed_sel_normalize();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_dc_remove(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// dc block each channel
			for(int c=0;c<pz->num_channels;c++)
			{			
				float* pwavedat=pz->ppwavedata[c]+WAVE_PAD+user_sed_sel_sta;
				float dc_sum=0;
				
				for(int s=0;s<user_sed_sel_len;s++)
					dc_sum+=pwavedat[s];

				dc_sum/=float(user_sed_sel_len);

				for(int s=0;s<user_sed_sel_len;s++)
					pwavedat[s]-=dc_sum;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_crossfade_loop(float* pbuffer,int const loop_start,int const loop_length)
{
	// apply crossfader
	for(int s=0;s<loop_length;s++)
	{
		// linear coefficient
		float const coef=(float)s/(float)loop_length;

		// after loop start fade out
		pbuffer[loop_start+s]*=(1.0f-coef);

		// pre-loop start data fade in
		int const prev_data=(loop_start-loop_length)+s;
		pbuffer[loop_start+s]+=pbuffer[prev_data]*coef;
	}

	// after loop samples (for interpolation error)
	for(int s=0;s<4;s++)
		pbuffer[loop_start+s+loop_length]=pbuffer[loop_start+s];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_reverse(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// filter each channel
			for(int c=0;c<pz->num_channels;c++)
			{			
				float* pwavedat=pz->ppwavedata[c]+WAVE_PAD+user_sed_sel_sta;

				for(int s=0;s<user_sed_sel_len/2;s++)
				{
					int const i=user_sed_sel_len-(s+1);
					float a=pwavedat[s];
					float b=pwavedat[i];
					pwavedat[s]=b;
					pwavedat[i]=a;
				}
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_rectify(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// filter each channel
			for(int c=0;c<pz->num_channels;c++)
			{			
				float* pwavedat=pz->ppwavedata[c]+WAVE_PAD+user_sed_sel_sta;

				for(int s=0;s<user_sed_sel_len;s++)
					pwavedat[s]=fabsf(pwavedat[s]);
			}

			// dc fix
			sed_sel_dc_remove();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_mathdrive(int const mode)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	float const f_pi=atanf(1.0f)*4.0f;

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// filter each channel
			for(int c=0;c<pz->num_channels;c++)
			{			
				float* pwavedat=pz->ppwavedata[c]+WAVE_PAD+user_sed_sel_sta;

				for(int s=0;s<user_sed_sel_len;s++)
				{
					// sin drive
					if(mode==0)
						pwavedat[s]=sinf(pwavedat[s]*f_pi)/f_pi;
					
					// tanh drive
					if(mode==1)
						pwavedat[s]=tanhf(pwavedat[s]*f_pi)/f_pi;
					
					// spectral halve
					if(mode==2)
						pwavedat[s]*=float(int(s&1));
				}
			}

			// dc fix
			sed_sel_dc_remove();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_clipboard_reset(void)
{
	// delete wavedata
	for(int c=0;c<user_clip_sample_channels;c++)
		delete[] user_clip_sample[c];

	// delete clip channels array
	delete[] user_clip_sample;
	user_clip_sample=NULL;

	// reset sample wave sclipboard
	user_clip_sample_size=0;
	user_clip_sample_channels=0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_cut_marker(int& position)
{
	if(position>=(user_sed_sel_sta+user_sed_sel_len))
	{
		position-=user_sed_sel_len;
	}
	else if(position>=user_sed_sel_sta)
	{
		if(user_sed_sel_sta>0)
			position=user_sed_sel_sta-1;
		else
			position=0;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_cut(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// enter critical section
			set_suspended (1);

			// cut cue points
			for(int ci=0;ci<MAX_CUE_POINTS;ci++)
				sed_sel_cut_marker(pz->cue_pos[ci]);

			// cut loop points
			sed_sel_cut_marker(pz->loop_start);
			sed_sel_cut_marker(pz->loop_end);

			// first copy to clipboard
			sed_sel_copy();

			// compute length after cut
			int const len_cut=pz->num_samples-user_sed_sel_len;
			int const len_cut_pad=len_cut+(WAVE_PAD*2);

			// cut each channel
			for(int c=0;c<pz->num_channels;c++)
			{
				float* pnewwave=new float[len_cut_pad];
				tool_init_dsp_buffer(pnewwave,len_cut_pad,0);

				// copy before selection
				for(int s=0;s<user_sed_sel_sta;s++)
					pnewwave[WAVE_PAD+s]=pz->ppwavedata[c][WAVE_PAD+s];

				// copy after selection
				for(int s=0;s<(pz->num_samples-(user_sed_sel_sta+user_sed_sel_len));s++)
					pnewwave[WAVE_PAD+s+user_sed_sel_sta]=pz->ppwavedata[c][WAVE_PAD+s+user_sed_sel_sta+user_sed_sel_len];

				// delete previous wave
				delete[] pz->ppwavedata[c];

				// assign new wave
				pz->ppwavedata[c]=pnewwave;
			}

			// set no selection
			user_sed_sel_len=0;

			// assign new size
			pz->num_samples=len_cut;
		
			// leave critical section
			set_suspended (0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_copy(void)
{
	// reset previous selection
	sed_clipboard_reset();
	
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// create clipboard channel array
			user_clip_sample=new float*[pz->num_channels];
			
			// create clipboard wave buffers and copy each channel to clipboard buffers
			for(int c=0;c<pz->num_channels;c++)
			{
				user_clip_sample[c]=new float[user_sed_sel_len];
				tool_copy_dsp_buffer(pz->ppwavedata[c]+WAVE_PAD+user_sed_sel_sta,user_clip_sample[c],user_sed_sel_len);
			}

			// notify new clipboard data
			user_clip_sample_channels=pz->num_channels;
			user_clip_sample_size=user_sed_sel_len;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_paste(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		// verify we have clip data
		if(user_clip_sample_size>0 && user_clip_sample_channels>0)
		{
			// enter critical section
			set_suspended (1);

			// adapt cue points
			for(int ci=0;ci<MAX_CUE_POINTS;ci++)
			{
				if(pz->cue_pos[ci]>=user_sed_sel_sta)
					pz->cue_pos[ci]+=user_clip_sample_size;
			}

			// adapt loop start
			if(pz->loop_start>=user_sed_sel_sta)
				pz->loop_start+=user_clip_sample_size;
			
			// adapt loop end
			if(pz->loop_end>=user_sed_sel_sta)
				pz->loop_end+=user_clip_sample_size;

			// compute length after append clipboard size
			int const len_ext=pz->num_samples+user_clip_sample_size;
			int const len_ext_pad=len_ext+(WAVE_PAD*2);

			// paste each channel
			for(int c=0;c<pz->num_channels;c++)
			{
				float* pnewwave=new float[len_ext_pad];
				tool_init_dsp_buffer(pnewwave,len_ext_pad,0);

				// copy before sel start
				for(int s=0;s<user_sed_sel_sta;s++)
					pnewwave[WAVE_PAD+s]=pz->ppwavedata[c][WAVE_PAD+s];

				// clipboard channel source
				int const ccsc=c%user_clip_sample_channels;
				
				// paste clipboard buffer
				for(int s=0;s<user_clip_sample_size;s++)
					pnewwave[WAVE_PAD+s+user_sed_sel_sta]=user_clip_sample[ccsc][s];

				// copy after sel start
				for(int s=0;s<(pz->num_samples-user_sed_sel_sta);s++)
					pnewwave[WAVE_PAD+user_sed_sel_sta+s+user_clip_sample_size]=pz->ppwavedata[c][WAVE_PAD+user_sed_sel_sta+s];

				// delete previous wave
				delete[] pz->ppwavedata[c];

				// assign new wave
				pz->ppwavedata[c]=pnewwave;
			}

			// set selection
			user_sed_sel_len=user_clip_sample_size;

			// assign new size
			pz->num_samples=len_ext;

			// leave critical section
			set_suspended (0);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_trim_marker(int& position)
{
	if(position<user_sed_sel_sta)
		position=0;
	
	if(position>=(user_sed_sel_sta+user_sed_sel_len))
		position=user_sed_sel_len-1;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_trim(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		if(user_sed_sel_len>0)
		{
			// enter critical section
			set_suspended (1);

			// trim markers
			for(int ci=0;ci<MAX_CUE_POINTS;ci++)
				sed_sel_trim_marker(pz->cue_pos[ci]);

			// trim markers
			sed_sel_trim_marker(pz->loop_start);
			sed_sel_trim_marker(pz->loop_end);

			// compute length after trim
			int const len_trim=user_sed_sel_len;
			int const len_trum_pad=len_trim+(WAVE_PAD*2);

			// trim each channel
			for(int c=0;c<pz->num_channels;c++)
			{
				float* pnewwave=new float[len_trum_pad];
				tool_init_dsp_buffer(pnewwave,len_trum_pad,0);

				// copy clipboard buffer
				for(int s=0;s<user_sed_sel_len;s++)
					pnewwave[WAVE_PAD+s]=pz->ppwavedata[c][WAVE_PAD+user_sed_sel_sta+s];

				// delete previous wave
				delete[] pz->ppwavedata[c];

				// assign new wave
				pz->ppwavedata[c]=pnewwave;
			}

			// assign new size
			pz->num_samples=len_trim;

			// set sel to start
			user_sed_sel_sta=0;
	
			// leave critical section
			set_suspended (0);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_add_cue(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		// add cue
		if(pz->num_cues<99)
		{
			pz->cue_pos[pz->num_cues]=user_sed_sel_sta;
			pz->num_cues++;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_sel_remove_cues(void)
{
	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		// enter critical section
		set_suspended (1);

		// cue array
		int cue_array[MAX_CUE_POINTS];
		int cue_array_num=0;

		// make temp cue array with zone cues out of selection
		for(int c=0;c<pz->num_cues;c++)
		{
			if(pz->cue_pos[c]<user_sed_sel_sta && pz->cue_pos[c]>=(user_sed_sel_sta+user_sed_sel_len))
				cue_array[cue_array_num++]=pz->cue_pos[c];
		}

		// copy temp cue array to to zone cues
		for(int c=0;c<cue_array_num;c++)
			pz->cue_pos[c]=cue_array[c];

		// set new num cues
		pz->num_cues=cue_array_num;

		// leave critical section
		set_suspended (0);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sed_time_stretch (double const pitchShift, double const timeRatio)
{
#ifdef HIGHLIFE_HAS_RUBBERBAND

	// get program
	HIGHLIFE_PROGRAM* pprg=&highlife_program[user_program];

	if(user_sed_zone >= 0 && user_sed_zone < pprg->num_zones)
	{
		// get zone
		HIGHLIFE_ZONE* pz=&pprg->pzones[user_sed_zone];

		// enter critical section
		set_suspended (1);

        double ratio = 1.0;
        double frequencyShift = 1.0;

        ratio *= timeRatio; // if (haveTimesFromOriginal)

//        if (haveTotalTime)
//        {
//            double m = 0.0;
//            if (m != 0.0) ratio /= m;
//        }

        if (pitchShift != 0.0)
            frequencyShift *= pow (2.0, pitchShift / 12);

        RubberBand::RubberBandStretcher::Options options = 0 |
                                RubberBand::RubberBandStretcher::OptionThreadingAuto |
                                RubberBand::RubberBandStretcher::OptionTransientsMixed |
                                RubberBand::RubberBandStretcher::OptionWindowLong;

        RubberBand::RubberBandStretcher ts (pz->sample_rate,
                                            pz->num_channels,
                                            options,
                                            ratio,
                                            frequencyShift);
        
        ts.setExpectedInputDuration (pz->num_samples);

        int ibs = 1024;

        float **obf = new float *[pz->num_channels];
        float **ibuf = new float *[pz->num_channels];
        for (int i = 0; i < pz->num_channels; ++i)
            ibuf[i] = new float[ibs];

        int frame = 0, countIn = 0, countOut = 0;

        // studying
        while (frame < pz->num_samples)
        {
            int count = jmax (0, jmin (ibs, pz->num_samples - frame));
        
            for (int c = 0; c < pz->num_channels; ++c)
                for (int i = 0; i < count; ++i)
                    ibuf[c][i] = pz->ppwavedata[c][WAVE_PAD + i + frame];

            bool final = (frame + ibs >= pz->num_samples);

            ts.study (ibuf, count, final);

            frame += ibs;
        }

        frame = 0;
    
        File tempFile (File::getSpecialLocation (File::tempDirectory).getChildFile ("highlifetemp.raw"));
        FILE* pfout = fopen ((const char*) tempFile.getFullPathName (),"wb");

        while (frame < pz->num_samples)
        {
            int count = jmax (0, jmin (ibs, pz->num_samples - frame));

            countIn += count;

            for (int i = 0; i < count; ++i)
                for (int c = 0; c < pz->num_channels; ++c)
                    ibuf[c][i] = pz->ppwavedata[c][WAVE_PAD + i + frame];

            bool final = (frame + ibs >= pz->num_samples);

            ts.process (ibuf, count, final);

            int avail = ts.available();
            if (avail > 0)
            {
                for (int i = 0; i < pz->num_channels; ++i)
                    obf[i] = new float[avail];

                ts.retrieve (obf, avail);

                countOut += avail;

                for (int i = 0; i < avail; ++i)
                    for (int c = 0; c < pz->num_channels; ++c)
                        fwrite (&obf[c][i], 1, sizeof(float), pfout);
               
                for (int i = 0; i < pz->num_channels; ++i)
                    delete[] obf[i];
            }

            frame += ibs;
        }

        for (int i = 0; i < pz->num_channels; ++i)
            delete[] ibuf[i];
        delete [] ibuf;
        delete [] obf;
        
        fclose (pfout);

        // allocate the new wave
        tool_alloc_wave (pz, pz->num_channels, countOut);
	    pfout = fopen ((const char*) tempFile.getFullPathName (), "rb");

	    for (int s = 0; s < pz->num_samples; s++)
	    {
		    for (int c = 0; c < pz->num_channels; c++)
		    {
			    float f_sample = 0;
			    fread (&f_sample, 1, sizeof(float), pfout);
			    pz->ppwavedata[c][s + WAVE_PAD] = f_sample;
		    }
	    }

	    // close file
	    fclose (pfout);

	    // delete temporal file
	    tempFile.deleteFile ();

		// leave critical section
		set_suspended (0);
	}
#else
    AlertWindow::showMessageBox (AlertWindow::WarningIcon,
                                 T("Warning"),
                                 T("The use of time stretching / pitch shifting has been disabled due to a code rewrite !"));
#endif
}

