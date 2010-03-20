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
// HighLife GIG Implementation                                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../LibGig/gig.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::gig_import (const File& file)
{
	// enter critical section
	set_suspended (1);

    RIFF::File* riff = new RIFF::File ((const char*) file.getFullPathName ());
    gig::File*  gig  = new gig::File (riff);

    int p = 0;

    gig::Instrument* instrument = gig->GetFirstInstrument();
    
    while (instrument)
    {
        int const c_prg = user_program + p;

        if (c_prg >= NUM_PROGRAMS)
            break;

        // get program pointer and initialize it
        HIGHLIFE_PROGRAM* pprg = &highlife_program[c_prg];
        tool_init_program (pprg);
        strncpy (pprg->name, instrument->pInfo->Name.c_str(), 32);

        printf ("instrument: %s \n", instrument->pInfo->Name.c_str());

        // zone pointer holder
        HIGHLIFE_ZONE* pz = NULL;

        gig::Region* region = instrument->GetFirstRegion();
        while (region)
        {
            for (int i = 0; i < region->Dimensions; i++) {
                printf ("Dimension %d \n", i);
                gig::dimension_def_t DimensionDef = region->pDimensionDefinitions[i];
            }
       
            gig::Sample* sample = region->GetSample();
            if (sample)
            {
                pz = tool_alloc_zone (pprg);

                strncpy (pz->name, sample->pInfo->Name.c_str(), MAX_PATH);
                printf ("sample: %s \n", sample->pInfo->Name.c_str());
                
                pz->lo_input_range.midi_key = region->KeyRange.low;
				pz->hi_input_range.midi_key = region->KeyRange.high;
				pz->lo_input_range.midi_vel = region->VelocityRange.low;
				pz->hi_input_range.midi_vel = region->VelocityRange.high;
				pz->midi_trigger = 0; // attack phase	
				// pz->midi_root_key = 0; // where in gig ?
				// pz->midi_fine_tune = sample->FineTune;
				// pz->midi_coarse_tune = 0; // where in gig ?
				// pz->midi_keycents = 0; // where in gig ?
				pz->mp_gain = 0;
				pz->mp_pan = 0;

                float value = 0.0f, sense = 0.0f;
                for (int i = 0; i < region->DimensionRegions; i++)
                {
                    gig::DimensionRegion* pDimensionRegion = region->pDimensionRegions[i];
                    if (! pDimensionRegion) break;
                    // printf ("Dimension Region %d -> %d\n", i, pDimensionRegion->pSample == region->GetSample());
                    
                    if (pDimensionRegion->VCFEnabled)
                    {
                        switch (pDimensionRegion->VCFType) {
                        case gig::vcf_type_lowpass:      pz->flt_type = 1; break;
                        case gig::vcf_type_lowpassturbo: pz->flt_type = 1; break;
                        case gig::vcf_type_bandpass:     pz->flt_type = 3; break;
                        case gig::vcf_type_highpass:     pz->flt_type = 2; break;
                        case gig::vcf_type_bandreject:   pz->flt_type = 4; break;
                        }
                        
                        value = pDimensionRegion->VCFCutoff / 22100.0f;
                        pz->flt_cut_frq.value = value;
                        pz->flt_cut_frq.sense = value * 2.0f - 1.0f;
                        pz->flt_cut_frq.ctrln = pDimensionRegion->VCFCutoffController;

                        value = pDimensionRegion->VCFResonance;
                        pz->flt_res_amt.value = value;
                        pz->flt_res_amt.sense = value * 2.0f - 1.0f;
                        pz->flt_res_amt.ctrln = pDimensionRegion->VCFResonanceController;
                    }         

/*
                    value = pDimensionRegion->EG1Attack / 60.0f;
                    pz->amp_env_att.value = value;
                    pz->amp_env_att.sense = value * 2.0f - 1.0f;
                    value = pDimensionRegion->EG1Decay1 / 60.0f;
	                pz->amp_env_dec.value = value;
                    pz->amp_env_dec.sense = value * 2.0f - 1.0f;
                    value = pDimensionRegion->EG1Sustain / 1000.0f;
	                pz->amp_env_sus.value = value;
                    pz->amp_env_sus.sense = value * 2.0f - 1.0f;
                    value = pDimensionRegion->EG1Release / 60.0f;
	                pz->amp_env_rel.value = value;
                    pz->amp_env_rel.sense = value * 2.0f - 1.0f;

                    // pz->amp_env_amt = 1.0;

                    value = pDimensionRegion->EG2Attack / 60.0f;
                    pz->mod_env_att.value = value;
                    pz->mod_env_att.sense = value * 2.0f - 1.0f;
                    value = pDimensionRegion->EG2Decay1 / 60.0f;
	                pz->mod_env_dec.value = value;
	                pz->mod_env_dec.sense = value * 2.0f - 1.0f;
                    value = pDimensionRegion->EG2Sustain / 1000.0f;
	                pz->mod_env_sus.value = value;
	                pz->mod_env_sus.sense = value * 2.0f - 1.0f;
                    value = pDimensionRegion->EG2Release / 60.0f;
	                pz->mod_env_rel.value = value;
	                pz->mod_env_rel.sense = value * 2.0f - 1.0f;
*/

//                    pz->vel_amp = pDimensionRegion->VelocityResponseDepth;
//                    pz->mp_pan = (float) pDimensionRegion->Pan;
                    pz->midi_root_key = pDimensionRegion->UnityNote;
//                    pz->midi_keycents = pDimensionRegion->FineTune;

                    break;
                }

			    // override loop points
			    if (sample->Loops) {
			        switch (sample->LoopType) {
                        case gig::loop_type_normal:         pz->loop_mode = 1; break;
                        case gig::loop_type_bidirectional:  pz->loop_mode = 2; break;
                        case gig::loop_type_backward:       pz->loop_mode = 3; break;
                    }
                    pz->loop_start = sample->LoopStart;
                    pz->loop_end = sample->LoopEnd;
                    // pSample->LoopFraction
                    // pSample->LoopPlayCount
                }

                // override group
                int iGroup = 1;
                for (gig::Group* pGroup = gig->GetFirstGroup(); pGroup; pGroup = gig->GetNextGroup(), iGroup++) {
                    if (pGroup == sample->GetGroup()) break;
                }
                pz->res_group = iGroup;

                // read sample 
                long needed_size = sample->BitDepth == 24 ?  sample->SamplesTotal * sample->Channels * sizeof(int)
                                                          :  sample->SamplesTotal * sample->FrameSize;

                uint8_t* pWave  = new uint8_t[needed_size];
                int* pIntWave = (int*) pWave;
                short* pShortWave = (short*) pWave;

                if (sample->Compressed) {
                    printf ("compressed sample \n");

                    gig::buffer_t decompressionBuffer;
                    decompressionBuffer.Size = 0;
                    unsigned long decompressionBufferSize = 0;
                    
                    decompressionBuffer = gig::Sample::CreateDecompressionBuffer (sample->SamplesTotal);
                    decompressionBufferSize = sample->SamplesTotal;
                    sample->Read (pWave, sample->SamplesTotal, &decompressionBuffer);
                    gig::Sample::DestroyDecompressionBuffer (decompressionBuffer);
                } else {
                    printf ("uncompressed sample \n");

                    sample->Read (pWave, sample->SamplesTotal);
                }
                
                if (pWave && sample->SamplesTotal > 0)
                {
                    tool_alloc_wave (pz, sample->Channels, sample->SamplesTotal);

                    if (sample->BitDepth == 24)
                    {
                        int n = sample->SamplesTotal * sample->Channels;
                        for (int i = n - 1 ; i >= 0 ; i--) {
                            // pIntWave[i] = pWave[i * 3] << 8 | pWave[i * 3 + 1] << 16 | pWave[i * 3 + 2] << 24;
                            pIntWave[i] = pWave[i * 3] | pWave[i * 3 + 1] << 8 | pWave[i * 3 + 2] << 16;
                        }
                        
                        const float scale = 1.0f / 0x7fffffff;
                        for (int c = 0; c < sample->Channels; c++) {
                            for (int s = 0; s < sample->SamplesTotal; s++) {
                                pz->ppwavedata[c][s + WAVE_PAD] = scale * float (pIntWave[s]);
                            }
                        }
                    }
                    else
                    {
                        const float scale = 1.0f / 0x7fff;
                        for (int c = 0; c < sample->Channels; c++) {
                            for (int s = 0; s < sample->SamplesTotal; s++) {
                                pz->ppwavedata[c][s + WAVE_PAD] = scale * float (pShortWave[s]);
                            }
                        }
                    }
                    
                    delete[] pWave;
                }
            }

            region = instrument->GetNextRegion();
        }

        instrument = gig->GetNextInstrument();
        ++p;
    }
    
	// sample editor should adapt
	user_sed_adapt=1;
	user_sed_zone=0;

	// enter critical section
    set_suspended (0);

    delete gig;
    delete riff;
}


