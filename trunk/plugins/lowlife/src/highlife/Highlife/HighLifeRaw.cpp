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
// HighLife RAW Implementation                                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::raw_load (HIGHLIFE_ZONE* pz, const File& file)
{
	char filetitle[MAX_PATH];
    strncpy (filetitle, (const char*) file.getFileNameWithoutExtension (), MAX_PATH);
	
	char buf[MAX_PATH];
	sprintf(buf,"Load '%s' As Stereo",filetitle);

    if (AlertWindow::showYesNoCancelBox (AlertWindow::QuestionIcon, T("discoDSP HighLife - RAW Loader"), buf) == 1)
		pz->num_channels=2;
	else
		pz->num_channels=1;

	// num samples holder
	pz->num_samples=0;

	// get file length
	FILE* pfile = fopen ((const char*) file.getFullPathName(),"rb");

	if(pfile)
	{
		fseek(pfile,0,SEEK_END);
		pz->num_samples=ftell(pfile)/(2*pz->num_channels);
		fclose(pfile);
	}
	else
	{
	    return;
	}

	// alloc wavedata in zone
	tool_alloc_wave(pz,pz->num_channels,pz->num_samples);

	// read file raw data
	pfile = fopen((const char*) file.getFullPathName(),"rb");

	if(pfile)
	{
		for(int s=0;s<pz->num_samples;s++)
		{
			for(int c=0;c<pz->num_channels;c++)
			{
				short i_sample;
				fread(&i_sample,sizeof(short),1,pfile);
				pz->ppwavedata[c][s+WAVE_PAD]=float(i_sample)/32768.0f;
			}
		}
		
		fclose(pfile);
	}
}
