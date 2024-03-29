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
// HighLife GUI Implementation                                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"
#include "HighLifeEditor.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sample editor/map editor area coords
int	const area_x=117;
int const area_y=196;
int	const area_w=632;
int const area_h=172;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_wheel(Graphics* const hdc,int const x,int const y,int value)
{
	if(value<0)value=0;
	if(value>128)value=128;

	gui_bitmap(hdc,x,y,12,50,value*12,0,hbitmap_whl);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_knob_12(Graphics* const hdc,int const x,int const y,float const value)
{
	float const frame_a=value*128.0f;	
	int a=int(frame_a);

	if(a<0)a=0;
	if(a>128)a=128;

	gui_bitmap(hdc,x,y,12,12,a*12,0,hbitmap_k12);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_knob_sin(Graphics* const hdc,int const x,int const y,float const value)
{
	float const frame_a=value*128.0f;	
	int a=int(frame_a);

	if(a<0)a=0;
	if(a>128)a=128;

	gui_bitmap(hdc,x,y,24,24,a*24,0,hbitmap_knb);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_knob(Graphics* const hdc,int const x,int const y,RTPAR* pp)
{
	float const f_frame_a=(pp->value)*128.0f;
	float const f_frame_b=(pp->value+pp->sense)*128.0f;

	int a=int(f_frame_a);
	int b=int(f_frame_b);

	if(a<0)a=0;
	if(a>128)a=128;
	if(b<0)b=0;
	if(b>128)b=128;

	gui_bitmap(hdc,x,y,24,24,a*24,0,hbitmap_knb);

	if(a!=b)
		gui_bitmap_ab(hdc,x,y,24,24,b*24,0,hbitmap_kna,96);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_led(Graphics* const hdc,int const x,int const y,int const mode)
{
	gui_bitmap(hdc,x,y,8,8,0,mode*8,hbitmap_led);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_but(Graphics* const hdc,int const x,int const y,int const m)
{
	gui_bitmap(hdc,x,y,13,10,0,m*10,hbitmap_but);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_fnt(Graphics* const hdc,int x,int y,const char* pchar,int const trunc)
{
	int c=0;
	int strl=strlen(pchar);

	if(strl>trunc)
		strl=trunc;

	while(c<strl)
	{
		int const a=(pchar[c]-32)*6;
		
		hdc->drawImage (hbitmap_fnt, x, y, 5, 10, a, 0, 5, 10);
		x+=6;
		c++;
	}

	while(c<trunc)
	{
		hdc->drawImage (hbitmap_fnt, x, y, 5, 10, 0, 0, 5, 10);
		x+=6;
		c++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_dig(Graphics* const hdc,int const x,int const y,int const v)
{
	gui_bitmap(hdc,x+42,y,6,6,(v%10)*6,0,hbitmap_dig);
	gui_bitmap(hdc,x+36,y,6,6,((v/10)%10)*6,0,hbitmap_dig);
	gui_bitmap(hdc,x+30,y,6,6,((v/100)%10)*6,0,hbitmap_dig);
	gui_bitmap(hdc,x+24,y,6,6,((v/1000)%10)*6,0,hbitmap_dig);
	gui_bitmap(hdc,x+18,y,6,6,((v/10000)%10)*6,0,hbitmap_dig);
	gui_bitmap(hdc,x+12,y,6,6,((v/100000)%10)*6,0,hbitmap_dig);
	gui_bitmap(hdc,x+6,y,6,6,((v/1000000)%10)*6,0,hbitmap_dig);
	gui_bitmap(hdc,x+0,y,6,6,((v/10000000)%10)*6,0,hbitmap_dig);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint_dig_cent(Graphics* const hdc,int const x,int const y,int const v)
{
	gui_bitmap(hdc,x+6,y,6,7,(v%10)*6,0,hbitmap_dig);
	gui_bitmap(hdc,x+0,y,6,7,((v/10)%10)*6,0,hbitmap_dig);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CHighLifeEditor::gui_get_int_mode(void)
{
	return fx->voice[0].get_int_mode();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::gui_paint(Graphics* const hdc)
{
	// program pointer
	HIGHLIFE_PROGRAM* pprg=&fx->highlife_program[fx->user_program];

	// buffer holder
	char buf[256];

#ifndef DISABLE_VST_HOST
	// main led
	if(fx->host_plug)
	{
		// vst effect info
		fx->host_plug->dispatcher(fx->host_plug,effGetEffectName,0,0,buf,0.0f);
		gui_paint_fnt(hdc,120,66,buf,26);

		// vst program info
		int const curr_vst_prg=fx->host_plug->dispatcher(fx->host_plug,effGetProgram,0,0,NULL,0.0f);
		char buf_prg[256];
		sprintf(buf_prg,"---");
		fx->host_plug->dispatcher(fx->host_plug,effGetProgramName,0,0,buf_prg,0.0f);
		sprintf(buf,"%.3d %s",curr_vst_prg+1,buf_prg);
		gui_paint_fnt(hdc,120,75,buf,26);
	}
	else
	{
		sprintf(buf,"VSTi Not Instanced");
		gui_paint_fnt(hdc,120,66,buf,26);

        String paramName = fx->getParameterName(0);
		
		gui_paint_fnt(hdc,120,75,(const char*) paramName,26);
	}
#else
	sprintf(buf,"Highlife sampler");
	gui_paint_fnt(hdc,120,66,buf,26);

    String paramName = fx->getParameterName(0);
	
	gui_paint_fnt(hdc,120,75,(const char*) paramName,26);
#endif

	// freeze properties leds
	sprintf(buf,"%d",fx->user_prg_splits); gui_paint_fnt(hdc,145,122,buf,3);			// kbd splits
	sprintf(buf,"%d",fx->user_vel_splits); gui_paint_fnt(hdc,201,122,buf,3);			// velo splits
	sprintf(buf,"%f",fx->user_split_time); gui_paint_fnt(hdc,258,122,buf,3);			// freeze split time
	format_note(fx->user_low_keyboa, buf); gui_paint_fnt(hdc,145,137,buf,3);		// freeze lo note rng
	format_note(fx->user_hig_keyboa, buf); gui_paint_fnt(hdc,201,137,buf,3);		// freeze hi note rng
	sprintf(buf,"%d",fx->user_ste_keyboa); gui_paint_fnt(hdc,258,137,buf,3);			// freeze step

	// main buttons
	gui_paint_but(hdc,297,67,fx->user_pressed==(NUM_PARAMETERS+0));					// load vsti plugin
	gui_paint_but(hdc,334,67,fx->user_pressed==(NUM_PARAMETERS+1));					// load wav
	gui_paint_but(hdc,297,96,fx->user_pressed==(NUM_PARAMETERS+2));					// plugin gui
	gui_paint_but(hdc,334,96,fx->user_route_midi*2);								// plugin midi
	gui_paint_but(hdc,297,128,fx->user_pressed==(NUM_PARAMETERS+3));				// plugin freeze
	gui_paint_but(hdc,334,128,fx->user_pressed==(NUM_PARAMETERS+4));				// unload plugin

	// master volume
	gui_paint_knob_sin(hdc,40,56,fx->user_master_volume);

	// pitchbend and mod wheel
	gui_paint_wheel(hdc,36,312,(fx->midi_state.midi_bend+8192)/128);
	gui_paint_wheel(hdc,56,312,fx->midi_state.midi_cc[1]);

	// program performance parameters in all views (only playmode)
	gui_paint_led(hdc,62,260,pprg->ply_mode==2);
	gui_paint_led(hdc,62,270,pprg->ply_mode==1);
	gui_paint_led(hdc,62,280,pprg->ply_mode==0);

    // manual mode
	gui_paint_led(hdc,747,54,fx->user_sed_manual==0);

	// zone performance parameters in all views
	if(fx->user_sed_zone>=0 && fx->user_sed_zone<pprg->num_zones)
	{
		// get zone pointer
		HIGHLIFE_ZONE* pz=&pprg->pzones[fx->user_sed_zone];

		// pitchbend range
		gui_paint_knob_sin(hdc,40,125,pz->sys_pit_rng);

		// glide led
		int const i_glide_auto=get_int_param(pz->glide_auto,NUM_GLIDE_MODES);
		gui_paint_led(hdc,23,206,i_glide_auto);

		// glide amount
		gui_paint_knob(hdc,40,198,&pz->glide);
	}
	else
	{
		gui_paint_knob_sin(hdc,40,125,0.0f);
		gui_paint_led(hdc,23,206,0);
		gui_paint_knob_sin(hdc,40,198,0.0f);
	}

	// performance view
	if(fx->user_gui_page==0)
	{
		// program parameters in performance view
		gui_paint_but(hdc,143,289,pprg->efx_dft*2);			// efx daft mode
		gui_paint_knob_12(hdc,116,288,pprg->efx_dft_frq);	// efx daft phaser speed
		gui_paint_but(hdc,129,316,pprg->efx_rck*2);			// efx rock da discoteque mode
		gui_paint_knob_12(hdc,220,288,pprg->efx_chr_del);	// efx chr delay
		gui_paint_knob_12(hdc,260,288,pprg->efx_chr_fdb);	// efx chr feedback
		gui_paint_knob_12(hdc,300,288,pprg->efx_chr_rat);	// efx chr rate
		gui_paint_knob_12(hdc,340,288,pprg->efx_chr_mod);	// efx chr mod
		gui_paint_knob_12(hdc,220,315,pprg->efx_del_del);	// efx del delay
		gui_paint_knob_12(hdc,260,315,pprg->efx_del_fdb);	// efx del feedback
		gui_paint_but(hdc,299,316,pprg->efx_del_cro*2);		// efx del cross
		gui_paint_but(hdc,339,316,pprg->efx_del_syn*2);		// efx del syn
		gui_paint_knob_12(hdc,220,342,pprg->efx_rev_roo);	// efx rev room
		gui_paint_knob_12(hdc,260,342,pprg->efx_rev_wid);	// efx rev width
		gui_paint_knob_12(hdc,300,342,pprg->efx_rev_dam);	// efx rev damp

		// program fx switches
		gui_paint_led(hdc,169,290,pprg->efx_chr);			// efx chr on
		gui_paint_led(hdc,169,317,pprg->efx_del);			// efx del on
		gui_paint_led(hdc,169,344,pprg->efx_rev);			// efx rev on
		
		// zone performance parameters
		if(fx->user_sed_zone>=0 && fx->user_sed_zone<pprg->num_zones)
		{
			// get zone pointer
			HIGHLIFE_ZONE* pz=&pprg->pzones[fx->user_sed_zone];

			// filter
			int const filter=get_int_param(pz->flt_type,NUM_FILTER_MODES);
			gui_paint_led(hdc,317,216,filter==0);
			gui_paint_led(hdc,301,195,filter==1);
			gui_paint_led(hdc,301,205,filter==2);
			gui_paint_led(hdc,329,195,filter==3);
			gui_paint_led(hdc,329,205,filter==4);

			// mod lfo sync
			gui_paint_led(hdc,440,204,int(pz->mod_lfo_syn));

			// vel amp and vel mod
			gui_paint_knob_12(hdc,723,86,pz->vel_amp);
			gui_paint_knob_12(hdc,723,122,pz->vel_mod);

			// filter tracking
			gui_paint_knob_12(hdc,241,185,pz->flt_kbd_trk);
			gui_paint_knob_12(hdc,241,221,pz->flt_vel_trk);

			// filter cutoff / resonance
			gui_paint_knob(hdc,120,196,&pz->flt_cut_frq);
			gui_paint_knob(hdc,186,196,&pz->flt_res_amt);

			// amplitude envelope
			gui_paint_knob(hdc,391,97,&pz->amp_env_att);
			gui_paint_knob(hdc,457,97,&pz->amp_env_dec);
			gui_paint_knob(hdc,523,97,&pz->amp_env_sus);
			gui_paint_knob(hdc,587,97,&pz->amp_env_rel);
			gui_paint_knob(hdc,653,97,&pz->amp_env_amt);

			// mod lfo
			gui_paint_knob(hdc,391,196,&pz->mod_lfo_phs);
			gui_paint_knob(hdc,457,196,&pz->mod_lfo_rat);
			gui_paint_knob(hdc,523,196,&pz->mod_lfo_amp);
			gui_paint_knob(hdc,587,196,&pz->mod_lfo_cut);
			gui_paint_knob(hdc,653,196,&pz->mod_lfo_res);
			gui_paint_knob(hdc,717,196,&pz->mod_lfo_pit);

			// mod envelope
			gui_paint_knob(hdc,391,307,&pz->mod_env_att);
			gui_paint_knob(hdc,457,307,&pz->mod_env_dec);
			gui_paint_knob(hdc,523,307,&pz->mod_env_sus);
			gui_paint_knob(hdc,587,307,&pz->mod_env_rel);
			gui_paint_knob(hdc,653,307,&pz->mod_env_cut);
			gui_paint_knob(hdc,717,307,&pz->mod_env_pit);
			
			// effects level
			gui_paint_knob_12(hdc,180,288,pz->efx_chr_lev.value);
			gui_paint_knob_12(hdc,180,315,pz->efx_del_lev.value);
			gui_paint_knob_12(hdc,180,342,pz->efx_rev_lev.value);
		}
		else
		{
			// filter off
			gui_paint_led(hdc,317,216,1);
			gui_paint_led(hdc,301,195,0);
			gui_paint_led(hdc,301,205,0);
			gui_paint_led(hdc,329,195,0);
			gui_paint_led(hdc,329,205,0);

			// mod lfo sync
			gui_paint_led(hdc,440,204,0);

			// vel amp and vel mod
			gui_paint_knob_12(hdc,723,86,0.0f);
			gui_paint_knob_12(hdc,723,122,0.0f);

			// effects level
			gui_paint_knob_12(hdc,180,288,0.0f);
			gui_paint_knob_12(hdc,180,315,0.0f);
			gui_paint_knob_12(hdc,180,342,0.0f);

			// filter tracking
			gui_paint_knob_12(hdc,241,185,0.0f);
			gui_paint_knob_12(hdc,241,221,0.0f);

			// off filter cutoff / resonance
			gui_paint_knob_sin(hdc,120,196,0.0f);
			gui_paint_knob_sin(hdc,186,196,0.0f);

			// off amplitude envelope
			gui_paint_knob_sin(hdc,391,97,0.0f);
			gui_paint_knob_sin(hdc,457,97,0.0f);
			gui_paint_knob_sin(hdc,523,97,0.0f);
			gui_paint_knob_sin(hdc,587,97,0.0f);
			gui_paint_knob_sin(hdc,653,97,0.0f);

			// off mod lfo
			gui_paint_knob_sin(hdc,391,196,0.0f);
			gui_paint_knob_sin(hdc,457,196,0.0f);
			gui_paint_knob_sin(hdc,523,196,0.0f);
			gui_paint_knob_sin(hdc,587,196,0.0f);
			gui_paint_knob_sin(hdc,653,196,0.0f);
			gui_paint_knob_sin(hdc,717,196,0.0f);

			// off mod envelope
			gui_paint_knob_sin(hdc,391,307,0.0f);
			gui_paint_knob_sin(hdc,457,307,0.0f);
			gui_paint_knob_sin(hdc,523,307,0.0f);
			gui_paint_knob_sin(hdc,587,307,0.0f);
			gui_paint_knob_sin(hdc,653,307,0.0f);
			gui_paint_knob_sin(hdc,717,307,0.0f);
		}
	}

	// editor
	if(fx->user_gui_page==1)
	{
		// get edit area bg color and paint background
		Colour const area_bg (0xFF37282B);

		gui_solid_rect(hdc,area_x,area_y,area_w,area_h,area_bg);

		// zone properties
		if(fx->user_sed_zone>=0 && fx->user_sed_zone<pprg->num_zones)
		{
			// get zone pointer
			HIGHLIFE_ZONE* pz=&pprg->pzones[fx->user_sed_zone];

			gui_paint_fnt(hdc,416,75,pz->name,19);												// zone name
			gui_paint_fnt(hdc,416,91,pz->path,19);												// path
			sprintf(buf,"%d",pz->mp_gain);gui_paint_fnt(hdc,416,107,buf,3);						// gain
			sprintf(buf,"%d",pz->mp_pan);gui_paint_fnt(hdc,466,107,buf,3);						// pan
			sprintf(buf,"%d",pz->midi_coarse_tune);gui_paint_fnt(hdc,416,122,buf,3);			// coarse tune
			sprintf(buf,"%d",pz->midi_fine_tune);gui_paint_fnt(hdc,466,122,buf,3);				// fine tune
			sprintf(buf,"%d",pz->midi_keycents);gui_paint_fnt(hdc,506,122,buf,4);				// keycents
			sprintf(buf,"%d",pz->res_group);gui_paint_fnt(hdc,416,137,buf,3);					// group
			sprintf(buf,"%d",pz->res_offby);gui_paint_fnt(hdc,466,137,buf,3);					// offby
			if(pz->midi_trigger==0)sprintf(buf,"ATCK");											// attack
			if(pz->midi_trigger==1)sprintf(buf,"RLSE");											// release
			gui_paint_fnt(hdc,506,137,buf,4);													// trigger mode
			sprintf(buf,"%d",pz->num_channels);gui_paint_fnt(hdc,574,75,buf,3);					// channels
			sprintf(buf,"%d",pz->sample_rate);gui_paint_fnt(hdc,622,75,buf,8);					// rate
			sprintf(buf,"%d",pz->num_samples);gui_paint_fnt(hdc,700,75,buf,8);					// size
			if(pz->loop_mode==0)sprintf(buf,"OFF");												// loop off
			if(pz->loop_mode==1)sprintf(buf,"FWD");												// loop forward
			if(pz->loop_mode==2)sprintf(buf,"BID");												// loop bidirectional
			if(pz->loop_mode==3)sprintf(buf,"BCK");												// loop backward
			if(pz->loop_mode==4)sprintf(buf,"SUS");												// loop forward sustained
			gui_paint_fnt(hdc,574,91,buf,3);													// loop type
			sprintf(buf,"%d",pz->loop_start);gui_paint_fnt(hdc,622,91,buf,8);					// loop start
			sprintf(buf,"%d",pz->loop_end);gui_paint_fnt(hdc,700,91,buf,8);						// loop end
			format_note(pz->midi_root_key,buf);gui_paint_fnt(hdc,574,107,buf,3);			// root key
			format_note(pz->lo_input_range.midi_key,buf);gui_paint_fnt(hdc,652,107,buf,3);	// midi low key
			format_note(pz->hi_input_range.midi_key,buf);gui_paint_fnt(hdc,700,107,buf,3);	// midi hig key
			sprintf(buf,"%d",pz->mp_num_ticks);gui_paint_fnt(hdc,574,122,buf,3);				// sync num ticks
			gui_paint_but(hdc,602,122,pz->mp_synchro*2);										// synchro gate
			sprintf(buf,"%d",pz->lo_input_range.midi_vel);gui_paint_fnt(hdc,652,122,buf,3);		// lo vel
			sprintf(buf,"%d",pz->hi_input_range.midi_vel);gui_paint_fnt(hdc,700,122,buf,3);		// hi vel
		}
		else
		{
			sprintf(buf,"----");
			gui_paint_fnt(hdc,416,75,buf,19);	// zone name
			gui_paint_fnt(hdc,416,91,buf,19);	// path
			gui_paint_fnt(hdc,416,107,buf,3);	// gain
			gui_paint_fnt(hdc,466,107,buf,3);	// pan
			gui_paint_fnt(hdc,416,122,buf,3);	// coarse tune
			gui_paint_fnt(hdc,466,122,buf,3);	// fine tune
			gui_paint_fnt(hdc,506,122,buf,4);	// keycents
			gui_paint_fnt(hdc,416,137,buf,3);	// group
			gui_paint_fnt(hdc,466,137,buf,3);	// offby
			gui_paint_fnt(hdc,506,137,buf,4);	// trigger mode
			gui_paint_fnt(hdc,574,75,buf,3);	// channels
			gui_paint_fnt(hdc,622,75,buf,8);	// rate
			gui_paint_fnt(hdc,700,75,buf,8);	// size
			gui_paint_fnt(hdc,574,91,buf,3);	// loop type
			gui_paint_fnt(hdc,622,91,buf,8);	// loop start
			gui_paint_fnt(hdc,700,91,buf,8);	// loop end
			gui_paint_fnt(hdc,574,107,buf,3);	// root key
			gui_paint_fnt(hdc,652,107,buf,3);	// midi low key
			gui_paint_fnt(hdc,700,107,buf,3);	// midi hig key
			gui_paint_fnt(hdc,574,122,buf,3);	// sync num ticks
			gui_paint_but(hdc,602,122,0);		// synchro gate
			gui_paint_fnt(hdc,652,122,buf,3);	// lo vel
			gui_paint_fnt(hdc,700,122,buf,3);	// hi vel
		}

		// zone map view
		if(fx->user_editor_mode==0)
		{
			// reset sample editor
			// user_sed_offset=0;
			// user_sed_spp=32;
			// user_sed_zone=0;
		}

		// sample view
		if(fx->user_editor_mode==1 && fx->user_sed_zone>=0 && fx->user_sed_zone<pprg->num_zones)
		{
			// select sample viewport region
            hdc->saveState ();
            hdc->reduceClipRegion (area_x,area_y,area_w,area_h);

			// get zone pointer
			HIGHLIFE_ZONE* pz=&pprg->pzones[fx->user_sed_zone];

			// adapt
			if(fx->user_sed_adapt)
			{
				fx->user_sed_offset=0;
				fx->user_sed_spp=(pz->num_samples/area_w)+1;
				fx->user_sed_adapt=0;
				fx->user_sed_sel_sta=0;
				fx->user_sed_sel_len=0;
			}

			// get samplelength
			int const num_samples=pz->num_samples;

				// channel loop
			for(int c=0;c<pz->num_channels;c++)
			{
				// get dc point
				int const area_hw=area_h/(pz->num_channels*2);
				int const area_dc=area_y+area_hw+(area_hw*2)*c;
				
				// paint 6 db lines
				gui_solid_rect(hdc,area_x,area_dc-area_hw/2,area_w,1, Colour (0xFF4A3538));
				gui_solid_rect(hdc,area_x,area_dc+area_hw/2,area_w,1, Colour (0xFF4A3538));

				// get sample wavedata
				float* pwavedata=pz->ppwavedata[c]+WAVE_PAD;

				// sample indexer
				int s=fx->user_sed_offset;

				// wave colour
                Colour const wave_co (0xFF006CE1);

				// display
				if(fx->user_sed_spp>16)
				{
					for(int x=0;x<area_w;x++)
					{
						if((s+fx->user_sed_spp)<=num_samples)
						{
							float peak_min=0.0f;
							float peak_max=-0.0f;

							float* ppwd=pwavedata+s;

							for(int pc=0;pc<fx->user_sed_spp;pc++)
							{
								float const sample=ppwd[pc];
								if(sample>peak_max)peak_max=sample;
								if(sample<peak_min)peak_min=sample;
							}

							float const siz=(peak_max-peak_min)*float(area_hw);
							float const pos=float(area_dc)-float(peak_max)*float(area_hw);

							int const wd_x=area_x+x;
							int const wd_y=int(pos);
							int const wd_h=int(siz);

							gui_solid_rect(hdc,wd_x,wd_y-1,1,1,wave_co.darker (1.0));
							gui_solid_rect(hdc,wd_x,wd_y,1,wd_h+1,wave_co);
							gui_solid_rect(hdc,wd_x,wd_y+wd_h,1,1,wave_co.darker (1.0));

							s+=fx->user_sed_spp;
						}
					}
				}
				else
				{
					for(int x=0;x<area_w;x++)
					{
						if((s+fx->user_sed_spp)<=num_samples)
						{
							float* ppwd=pwavedata+s;

							for(int pc=0;pc<fx->user_sed_spp;pc++)
							{
								float const sample=ppwd[pc];
								float const pos=float(area_dc)-float(sample)*float(area_hw);

                                hdc->setColour (wave_co);
                                hdc->setPixel (area_x+x, int(pos));
							}

							s+=fx->user_sed_spp;
						}
					}
				}

				// paint dc area
				gui_solid_rect(hdc,area_x,area_dc-1,area_w,1, area_bg.brighter (1.0f));
			}

			// draw sync ticks
			if(pz->mp_synchro && pz->mp_num_ticks)
			{
				for(int btick=0;btick<pz->mp_num_ticks;btick++)
				{
					// get tick pos
					double const d_tick_len=double(pz->num_samples)/double(pz->mp_num_ticks);
					double const d_tick_pos=double(btick)*d_tick_len;
					int const i_tick_pos=gui_sed_sample_to_coord(int(d_tick_pos));

					// default tick color
                    Colour csl (0xFF4A3538);

					// hilite beat and 2beat tick color
					if((btick&0x3)==0) csl = Colour (0xFFDED1C6 - 0xFF606060);
					if((btick&0x7)==0) csl = Colour (0xFFDED1C6 - 0xFF404040);
				
					// measure tick
					if((btick&0xF)==0)
					{
						// solid fill rect
						gui_solid_rect(hdc,i_tick_pos,area_y,1,area_h,Colour (0xFFDED1C6));
					}
					else
					{
						// dotted tick paint
						for(int ts=0;ts<area_h;ts+=2)
							gui_solid_rect(hdc,i_tick_pos,area_y+ts,1,1,csl);
					}
				}
			}

			// outbound
			int const outbound_pos=gui_sed_sample_to_coord(pz->num_samples);

			if(outbound_pos<(area_x+area_w))
			{
				gui_solid_rect(hdc,outbound_pos,area_y,area_w-(outbound_pos-area_x),area_h,Colour (0xFF44403C));
				gui_solid_rect(hdc,outbound_pos,area_y,1,area_h,area_bg.brighter (1.0f));
			}

			// paint loop bars and offset bar
			if(pz->loop_mode)
			{
				int const ls=gui_sed_sample_to_coord(pz->loop_start);
				int const le=gui_sed_sample_to_coord(pz->loop_end);
		
				gui_solid_rect(hdc,ls,area_y,1,area_h,Colour (0xFFF5D8C5));
				gui_solid_rect(hdc,le,area_y,1,area_h,Colour (0xFFF5D8C5));
		
				gui_bitmap(hdc,ls,area_y+area_h-12,12,12,0,0,hbitmap_lop);
				gui_bitmap(hdc,le-12,area_y+area_h-12,12,12,12,0,hbitmap_lop);
			}

			// paint offset bar
			for(int ci=0;ci<pz->num_cues;ci++)
			{
				int const of=gui_sed_sample_to_coord(pz->cue_pos[ci]);
				gui_solid_rect(hdc,of,area_y,1,area_h,Colour (0xFFCEF5C5));
				gui_bitmap(hdc,of,area_y+area_h-9,14,9,24,0,hbitmap_lop);
				gui_paint_dig_cent(hdc,of+1,area_y+area_h-8,ci);
			}

			// selection
			int const ss=gui_sed_sample_to_coord(fx->user_sed_sel_sta);
			int const sl=fx->user_sed_sel_len/fx->user_sed_spp;
			gui_invert(hdc,ss,area_y,sl,area_h);

			// set full region
            hdc->restoreState ();

			// sel start / end
			gui_paint_dig(hdc,629,188,fx->user_sed_sel_sta);
			gui_paint_dig(hdc,690,188,fx->user_sed_sel_len);
		}
	}

	// display highlife program
	sprintf(buf,"%.3d %s",fx->user_program+1,fx->highlife_program[fx->user_program].name);
	gui_paint_fnt(hdc,120,89,buf,26);

	// status
/*
	char nam[64];
	char dis[64];
*/

	if(fx->user_pressed && fx->user_pressed<NUM_PARAMETERS)
	{
		// ctrl next param
		bool can_ctrl=false;

		if(fx->user_pressed>=32)
			can_ctrl=true;

		// tweak
/*      // XXX - TODO - must set a global variable
		if(GetKeyState(VK_CONTROL)<0 && can_ctrl)
		{
			fx->getParameterName(fx->user_pressed+1,nam);
			fx->getParameterDisplay(fx->user_pressed+1,dis);
		}
		else
		{
			fx->getParameterName(fx->user_pressed,nam);
			fx->getParameterDisplay(fx->user_pressed,dis);
		}
*/

        String paramName = fx->getParameterName (fx->user_pressed);
        String paramText = fx->getParameterText (fx->user_pressed);

		sprintf(buf,"%s = %s",(const char*) paramName, (const char*) paramText);
	}
	else
	{
		// display selected zone
		if(fx->user_sed_zone>=0 && fx->user_sed_zone<pprg->num_zones)
		{
			HIGHLIFE_ZONE* pz=&pprg->pzones[fx->user_sed_zone];
			sprintf(buf,"%.3d %s (%dK)",fx->user_sed_zone+1,pz->name,(pz->num_samples*pz->num_channels*4)/1024);
		}
		else
		{
			sprintf(buf,"No Zone(s)");
		}
	}

	// param info
	gui_paint_fnt(hdc,120,103,buf,26);

	// wavetable generator processing bar
	if(fx->wave_processing==1)
		gui_paint_fnt(hdc,120,103,fx->wave_buf,26);}
