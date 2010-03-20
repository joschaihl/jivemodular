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
// HighLife DLS Implementation                                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../LibGig/DLS.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::dls_import (const File& file)
{
	// enter critical section
	set_suspended (1);

    RIFF::File* riff = new RIFF::File ((const char*) file.getFullPathName ());
    DLS::File*  dls  = new DLS::File (riff);

/*
    int p = 0;

    gig::Instrument* instrument = dls->GetFirstInstrument();
    
    while (instrument)
    {
        int const c_prg = user_program + p;

        if (c_prg >= NUM_PROGRAMS)
            break;

        // get program pointer and initialize it
        HIGHLIFE_PROGRAM* pprg = &highlife_program[c_prg];
        tool_init_program (pprg);
        strncpy (pprg->name, instrument->pInfo->Name.c_str(), 32);

        // zone pointer holder
        HIGHLIFE_ZONE* pz = NULL;

        gig::Region* region = instrument->GetFirstRegion();
        while (region)
        {
            gig::Sample* sample = region->GetSample();
            if (sample)
            {
                pz = tool_alloc_zone (pprg);

                strncpy (pz->name, sample->pInfo->Name.c_str(), MAX_PATH);

                pz->lo_input_range.midi_key = region->KeyRange.low;
				pz->hi_input_range.midi_key = region->KeyRange.high;
				pz->lo_input_range.midi_vel = region->VelocityRange.low;
				pz->hi_input_range.midi_vel = region->VelocityRange.high;
				pz->midi_trigger = 0; // attack phase	
				// pz->midi_root_key = 0; // where in gig ?
				pz->midi_fine_tune = sample->FineTune;
				// pz->midi_coarse_tune = 0; // where in gig ?
				// pz->midi_keycents = 0; // where in gig ?
				pz->mp_gain = 0;
				pz->mp_pan = 0;

			    // override loop points
//			    if (sample->loopstart >= 0 || sample->loopend >= 0)
//			    {
//				    pz->loop_mode = 1; // parser_loopmode;

//				    if (sample->loopstart >= 0)
//					    pz->loop_start = sample->loopstart;

//				    if (sample->loopend >= 0)
//					    pz->loop_end = sample->loopend;
//			    }

                long neededsize = sample->BitDepth == 24 ?  sample->SamplesTotal * sample->Channels * sizeof(int)
                                                         :  sample->SamplesTotal * sample->FrameSize;

                uint8_t* pWave  = new uint8_t[neededsize];
                int* pIntWave = (int*) pWave;
                long BufferSize = neededsize;

                gig::buffer_t decompressionBuffer;
                decompressionBuffer.Size = 0;
                unsigned long decompressionBufferSize = 0;

                if (sample->Compressed) {
                    if (decompressionBufferSize < sample->SamplesTotal) {
                        gig::Sample::DestroyDecompressionBuffer (decompressionBuffer);
                        decompressionBuffer = gig::Sample::CreateDecompressionBuffer (sample->SamplesTotal);
                        decompressionBufferSize = sample->SamplesTotal;
                    }
                    sample->Read (pWave, sample->SamplesTotal, &decompressionBuffer);
                } else {
                    sample->Read (pWave, sample->SamplesTotal);
                }
                
                if (pWave) {
                    if (sample->BitDepth == 24) {
                        int n = sample->SamplesTotal * sample->Channels;
                        for (int i = n - 1 ; i >= 0 ; i--) {
//                              pIntWave[i] = pWave[i * 3] << 8 | pWave[i * 3 + 1] << 16 | pWave[i * 3 + 2] << 24;
                            pIntWave[i] = pWave[i * 3] | pWave[i * 3 + 1] << 8 | pWave[i * 3 + 2] << 16;
                        }
                    }
                    
                    int num_channels = sample->Channels;
                    int num_samples = sample->SamplesTotal;
                    
                    if (num_samples > 0)
                    {
                        tool_alloc_wave (pz, num_channels, num_samples);
                        const float scale = 1.0f / 0x7fffffff;

                        for (int c = 0; c < num_channels; c++)
                        {
                            for (int s = 0; s < num_samples; s++)
                            {
                                pz->ppwavedata[c][s + WAVE_PAD] = scale * float (pIntWave[s]);
                            }
                        }
                    }
                }

                sample->ReleaseSampleData();
                
                if (pWave) delete[] pWave;
            }
            
            region = instrument->GetNextRegion();
        }

        instrument = dls->GetNextInstrument();
        ++p;
    }
    
	// sample editor should adapt
	user_sed_adapt=1;
	user_sed_zone=0;

*/

    delete dls;
    delete riff;

	// enter critical section
    set_suspended (0);
}


