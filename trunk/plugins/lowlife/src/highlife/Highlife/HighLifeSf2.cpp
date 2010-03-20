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
// HighLife SF2 Implementation                                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// #include "../FluidSynth/include/fluidsynth.h"

#include "../FluidSynth/src/fluid_defsfont.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sf2_import(const File& file)
{
	// enter critical section
	set_suspended (1);

    fluid_defsfont_t* sfont = new_fluid_defsfont ();
    fluid_defsfont_load (sfont, (const char*) file.getFullPathName ());
    fluid_defsfont_load_sampledata (sfont);

    fluid_defpreset_t* preset = sfont->preset;

    int p = 0;

    do
    {
        int const c_prg = user_program + p;

        if (! preset)
            break;
    
        if (c_prg >= NUM_PROGRAMS)
            break;

        printf ("start preset %s \n", preset->name);

        // get program pointer and initialize it
        HIGHLIFE_PROGRAM* pprg = &highlife_program[c_prg];
        tool_init_program (pprg);

        // zone pointer holder
        HIGHLIFE_ZONE* pz = NULL;

        strncpy (pprg->name, preset->name, 32);

        fluid_preset_zone_t* zone = preset->zone;

        if (zone)
        {
            printf ("start instrument \n");
        
            // start instrument	
            fluid_inst_t* inst = zone->inst;

            if (inst)
            {
                // start zones
                fluid_inst_zone_t* realzone = inst->zone;

                int count = 0;

                do
                {
                    printf ("zone %d \n", count++);

                    fluid_sample_t* sample = realzone->sample;

                    pz = tool_alloc_zone (pprg);
				    pz->lo_input_range.midi_key = realzone->keylo;
					pz->hi_input_range.midi_key = realzone->keyhi;
					pz->lo_input_range.midi_vel = realzone->vello;
					pz->hi_input_range.midi_vel = realzone->velhi;
					pz->midi_trigger = 0; // attack phase	
					pz->midi_root_key = sample->origpitch; // original root key note
					pz->midi_fine_tune = 0; // where in sf2 ?
					pz->midi_coarse_tune = 0; // where in sf2 ?
					pz->midi_keycents = sample->pitchadj; // in cents
					pz->mp_gain = 0; // 0 decibels
					pz->mp_pan = 0; // center

                    strncpy (pz->name, realzone->name, MAX_PATH);

                    // set generators
                    if (realzone->gen [GEN_VOLENVATTACK].flags & GEN_SET) {
                        pz->amp_env_att.value = realzone->gen [GEN_VOLENVATTACK].val;
                        DBG ("GEN_VOLENVATTACK: " + String (realzone->gen [GEN_VOLENVATTACK].val));
                    }

                    if (realzone->gen [GEN_VOLENVDECAY].flags & GEN_SET) {
                        pz->amp_env_dec.value = realzone->gen [GEN_VOLENVDECAY].val;
                        DBG ("GEN_VOLENVDECAY: " + String (realzone->gen [GEN_VOLENVDECAY].val));
                    }

                    if (realzone->gen [GEN_VOLENVSUSTAIN].flags & GEN_SET) {
                        pz->amp_env_sus.value = realzone->gen [GEN_VOLENVSUSTAIN].val;
                        DBG ("GEN_VOLENVSUSTAIN: " + String (realzone->gen [GEN_VOLENVSUSTAIN].val));
                    }

                    if (realzone->gen [GEN_VOLENVRELEASE].flags & GEN_SET) {
                        pz->amp_env_rel.value = realzone->gen [GEN_VOLENVRELEASE].val + 12000.0f / 20000.0f;
                        DBG ("GEN_VOLENVRELEASE: " + String (pz->amp_env_rel.value));
                    }

                    if (realzone->gen [GEN_FILTERFC].flags & GEN_SET) {
                        pz->flt_type = 1;
                        pz->flt_cut_frq.value = realzone->gen [GEN_FILTERFC].val;
                        DBG ("GEN_FILTERFC: " + String (realzone->gen [GEN_FILTERFC].val));
                    }

                    if (realzone->gen [GEN_FILTERQ].flags & GEN_SET) {
                        pz->flt_type = 1;
                        pz->flt_res_amt.value = realzone->gen [GEN_FILTERQ].val;
                        DBG ("GEN_FILTERQ: " + String (realzone->gen [GEN_FILTERQ].val));
                    }

                    if (realzone->gen [GEN_CHORUSSEND].flags & GEN_SET) {
                        pz->efx_chr_lev.value = 1.0f;
                        DBG ("GEN_CHORUSSEND: " + String (realzone->gen [GEN_CHORUSSEND].val));
                    }

                    if (realzone->gen [GEN_REVERBSEND].flags & GEN_SET) {
                        pz->efx_rev_lev.value = 1.0f;
                        DBG ("GEN_REVERBSEND: " + String (realzone->gen [GEN_REVERBSEND].val));
                    }

/*
                    pz->flt_kbd_trk = ?;
                    pz->flt_vel_trk = ?;

                    pz->efx_del_lev = ?;

                    pz->mod_lfo_syn = ?;
                    pz->mod_lfo_phs = ?;
                    pz->mod_lfo_rat = ?;
                    pz->mod_lfo_amp = ?;
                    pz->mod_lfo_cut = ?;
                    pz->mod_lfo_res = ?;
                    pz->mod_lfo_pit = ?;

                    pz->mod_env_att = ?;
                    pz->mod_env_dec = ?;
                    pz->mod_env_sus = ?;
                    pz->mod_env_rel = ?;
                    pz->mod_env_cut = ?;
                    pz->mod_env_pit = ?;
*/

/*
  GEN_MODLFOTOPITCH,
  GEN_VIBLFOTOPITCH,
  GEN_MODENVTOPITCH,
  GEN_MODLFOTOFILTERFC,
  GEN_MODENVTOFILTERFC,
  GEN_MODLFOTOVOL,
  GEN_PAN,
  GEN_MODLFODELAY,
  GEN_MODLFOFREQ,
  GEN_VIBLFODELAY,
  GEN_VIBLFOFREQ,
  GEN_MODENVDELAY,
  GEN_MODENVATTACK,
  GEN_MODENVHOLD,
  GEN_MODENVDECAY,
  GEN_MODENVSUSTAIN,
  GEN_MODENVRELEASE,
  GEN_KEYTOMODENVHOLD,
  GEN_KEYTOMODENVDECAY,
  GEN_KEYTOVOLENVHOLD,
  GEN_KEYTOVOLENVDECAY,
  GEN_KEYRANGE,
  GEN_VELRANGE,
  GEN_KEYNUM,
  GEN_VELOCITY,
  GEN_ATTENUATION,
  GEN_COARSETUNE,
  GEN_FINETUNE,
  GEN_SAMPLEID,
  GEN_SAMPLEMODE,
  GEN_SCALETUNE,
  GEN_OVERRIDEROOTKEY,
*/

/*
                    
*/
                    const int num_samples = sample->end - sample->start;
                    const int loopstart = sample->loopstart - sample->start;
                    const int loopend = sample->loopend - sample->start;

				    // override loop points
				    if (loopstart > 0 && loopend > 0)
				    {
					    pz->loop_mode = 0; // 1; // forward
					    pz->loop_start = loopstart;
					    pz->loop_end = loopend;
				    }

                    // copy over sample data (could be optimized)
                    if (num_samples > 0)
                    {
                        tool_alloc_wave (pz, 1, num_samples);
/*                            
                        fluid_defsfont_load_sampledata_into (sfont, 
                                                             sample->start,
                                                             num_samples,
                                                             pz->ppwavedata,
                                                             0,
                                                             num_samples,
                                                             WAVE_PAD);
*/
                        const float scale = 1.0f / 32768.0f;
                        for (int s = 0; s < num_samples; s++)
                        {
                            pz->ppwavedata[0][s + WAVE_PAD] = float (sample->data[sample->start + s]) * scale;
                        }
                    }
                
                } while ((realzone = realzone->next) != 0);

                // end zones
            }

            // end instrument					
        }               
                    
        ++p;        
    }
    while ((preset = preset->next) != 0);

    delete_fluid_defsfont (sfont);

	// sample editor should adapt
	user_sed_adapt=1;
	user_sed_zone=0;

	// enter critical section
    set_suspended (0);
}


