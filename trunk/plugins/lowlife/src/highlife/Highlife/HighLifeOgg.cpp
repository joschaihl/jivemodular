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
// HighLife OGG Implementation                                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "vorbis/codec.h"
#include "vorbis/vorbisfile.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

char ogg_buf[16384];

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::ogg_load (HIGHLIFE_ZONE* pz, const File& file)
{
    vorbis_info* info;
    OggVorbis_File oggFile;
    File tempFile (File::getSpecialLocation (File::tempDirectory).getChildFile ("highlifetemp.raw"));

    int num_channels = 0;
    int num_samples = 0;
    
    FILE* pfile = fopen ((const char*) file.getFullPathName(), "rb");
    if (pfile)
    {
	    FILE* pfout = fopen ((const char*) tempFile.getFullPathName (),"wb");

        if (pfout && ov_open_callbacks (pfile, &oggFile, NULL, 0, OV_CALLBACKS_NOCLOSE) == 0)
        {
            info = ov_info (&oggFile, -1);
            num_channels = info->channels;
            num_samples = ov_pcm_total (&oggFile, -1);

            int eof = 0;
            int current_section = -1;
            while (! eof)
            {
                int size = ov_read (&oggFile, ogg_buf, sizeof(ogg_buf), 0, 2, 1, &current_section);
                if (size == 0) {
                  eof = 1;
                } else if (size < 0) {
                } else {
                  fwrite (ogg_buf, 1, size, pfout);
                }
            }

            fclose (pfout);

            ov_clear (&oggFile);
        }
        else
        {
            printf("ov_open_callbacks() failed, file isn't vorbis?");
        }

        fclose (pfile);

        // open temporal file and convert samples
        tool_alloc_wave (pz, num_channels, num_samples);
	    pfile = fopen ((const char*) tempFile.getFullPathName (), "rb");

	    for (int s = 0; s < num_samples; s++)
	    {
		    for (int c = 0; c < num_channels; c++)
		    {
			    short i_sample = 0;
			    fread (&i_sample, 1, sizeof(short), pfile);
			    pz->ppwavedata[c][s + WAVE_PAD] = float (i_sample) / 32768.0f;
		    }
	    }

	    // close file
	    fclose (pfile);

	    // delete temporal file
	    tempFile.deleteFile ();
    }
    else
    {
        printf("cannot open vorbis file ?");
    }
}

/*
BOOL LoadVorbisBuffer(const char *name, ALuint buffer)
{
        BOOL success = FALSE;
        FILE *fh;
        vorbis_info *info;
        OggVorbis_File oggFile;

        if (alIsBuffer(buffer) == AL_FALSE)
        {
                dbgmsg("LoadVorbisBuffer() - called with bad AL buffer!");
                return FALSE;
        }

        fh = fopen(name, "rb");

        if (fh)
        {
                if (ov_open_callbacks(fh, &oggFile, NULL, 0, file_callbacks) == 0)
                {
                        info = ov_info(&oggFile, -1);

                        DWORD len = ov_pcm_total(&oggFile, -1) * info->channels * 2;    // always 16 bit data

                        BYTE *data = (BYTE *) malloc(len);

                        if (data)
                        {
                                int bs = -1;
                                DWORD todo = len;
                                BYTE *bufpt = data;

                                while (todo)
                                {
                                        int read = ov_read(&oggFile, (char *) bufpt, todo, OGG_ENDIAN, 2, 1, &bs);
                                        todo -= read;
                                        bufpt += read;
                                }

                                alBufferData(buffer, (info->channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16, data, len, info->rate);
                                success = TRUE;

                                free(data);
                        }
                        else
                                dbgmsg("LoadVorbisBuffer() - couldn't allocate decode buffer");

                        ov_clear(&oggFile);
                }
                else
                {
                        fclose(fh);
                        dbgmsg("LoadVorbisBuffer() - ov_open_callbacks() failed, file isn't vorbis?");
                }
        }
        else
                dbgmsg("LoadVorbisBuffer() - couldn't open file!");


        return success;
}
*/

