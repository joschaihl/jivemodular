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
// HighLife SFZ Implementation                                                                                                         //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"

#ifndef strlwr

    #include <string.h>
    #include <ctype.h>

    inline char* strlwr (char * a)
    {
        char *ret = a;
        while (*a != '\0') {
            if (isupper (*a))
        	*a = tolower (*a);
            ++a;
        }
        return ret;
    }

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sfz_import(const File& file)
{
	// enter critical section
	set_suspended (1);

	// open each file
	int const c_prg = user_program;
		
	if(c_prg<NUM_PROGRAMS)
	{
		// get program pointer and initialize it
		HIGHLIFE_PROGRAM* pprg=&highlife_program[c_prg];
		tool_init_program(pprg);

		// zone pointer holder
		HIGHLIFE_ZONE* pz=NULL;

		// parser vars
		int parser_lokey=0;
		int parser_hikey=127;
		int parser_lovel=0;
		int parser_hivel=127;
		int parser_tune=0;
		int parser_transpose=0;
		float parser_volume=0.0f;
		float parser_pan=0.0f;
		int	parser_loopstart=-1;
		int	parser_loopend=-1;
		int	parser_loopmode=-1;
		int parser_root_key=-1;

		// init program name
        strncpy (pprg->name, (const char*) file.getFileNameWithoutExtension (), 31);

		// open the file
		FILE* pfile = fopen ((const char*) file.getFullPathName (), "rb");

		// file opened ok
		if(pfile)
		{
			// line holder
			char line[512];
			char word[512];

			// process lines
			while(!feof(pfile))
			{
				// get line
				if(fgets(line,512,pfile)!=NULL)
				{
					// check if line is not comment
					if(line[0]!='/')
					{
						int wc=0;
						int lp=0;

						word[0]=0;

						// get line width
						int const line_width=strlen(line);
						
						// parse words from line
						for(int lc=0;lc<=line_width;lc++)
						{
							word[wc]=line[lc];

							// new opcode found
							if(word[wc]==' ' || word[wc]==0 || word[wc]=='\n' || word[wc]=='\r')
							{
								// set word
								word[wc]=0;

								// restart word letter count
								wc=0;
								
								// parse opcode
								strlwr(word);

								// opcode search
								if(strncmp(word,"<region>",8)==0)				// new <region>
								{
									// set parser data to current wave
									if(pz!=NULL)
									{
										pz->lo_input_range.midi_key=parser_lokey;
										pz->hi_input_range.midi_key=parser_hikey;
										pz->lo_input_range.midi_vel=parser_lovel;
										pz->hi_input_range.midi_vel=parser_hivel;
										pz->midi_fine_tune=parser_tune;
										pz->midi_coarse_tune=parser_transpose;
										pz->mp_gain=int(parser_volume);
										pz->mp_pan=int(parser_pan*0.5f);

										// override loop points
										if(parser_loopmode>=0)
										{
											pz->loop_mode=parser_loopmode;
										
											if(parser_loopstart>=0)
												pz->loop_start=parser_loopstart;
										
											if(parser_loopend>=0)
												pz->loop_end=parser_loopend;
										}

										// override keyroot
										if(parser_root_key>=0)
											pz->midi_root_key=parser_root_key;
									}

									// alloc new zone
									pz=tool_alloc_zone(pprg);
									
									// re-init parser data
									parser_lokey=0;
									parser_hikey=127;
									parser_lovel=0;
									parser_hivel=127;
									parser_tune=0;
									parser_transpose=0;
									parser_volume=0.0f;
									parser_pan=0.0f;
									parser_loopstart=-1;
									parser_loopend=-1;
									parser_loopmode=-1;
									parser_root_key=-1;
								}
								else if(strncmp(word,"sample",6)==0)		// sample
								{	
									// get wavefilename (supports spaces)
									char wavefilename[MAX_PATH];
									sprintf(wavefilename,&line[lp+7]);
									int const wfl=strlen(wavefilename);

									// fix wave filename
									for(int fc=0;fc<wfl;fc++)
									{
										if(wavefilename[fc]=='\r' || wavefilename[fc]=='\n')
											wavefilename[fc]=0;
									}

									// load sample
                                    File waveFile = file.getParentDirectory().getChildFile (wavefilename);
									tool_load_sample (pz, waveFile);
								}
								else if(strncmp(word,"lokey",5)==0)			// lokey
								{
									char* pstrval=&word[6];
									parser_lokey=atoi(pstrval);
								}
								else if(strncmp(word,"hikey",5)==0)			// hikey
								{
									char* pstrval=&word[6];
									parser_hikey=atoi(pstrval);
								}
								else if(strncmp(word,"lovel",5)==0)			// lovel
								{
									char* pstrval=&word[6];
									parser_lovel=atoi(pstrval);
								}
								else if(strncmp(word,"hivel",5)==0)			// hivel
								{
									char* pstrval=&word[6];
									parser_hivel=atoi(pstrval);
								}
								else if(strncmp(word,"offset",6)==0)		// offset
								{
									char* pstrval=&word[7];
									pz->cue_pos[0]=atoi(pstrval);
									pz->num_cues=1;
								}
								else if(strncmp(word,"tune",4)==0)			// tune
								{
									char* pstrval=&word[5];
									parser_tune=atoi(pstrval);
								}
								else if(strncmp(word,"pitch_keytrack",14)==0)	// pitch keytrack
								{
									char* pstrval=&word[15];
									pz->midi_keycents=atoi(pstrval);
								}
								else if(strncmp(word,"pitch_keycenter",15)==0)	// pitch keycenter
								{
									char* pstrval=&word[16];
									parser_root_key=atoi(pstrval);
								}
								else if(strncmp(word,"transpose",9)==0)		// transpose
								{
									char* pstrval=&word[10];
									parser_transpose=atoi(pstrval);
								}
								else if(strncmp(word,"volume",6)==0)		// volume
								{
									char* pstrval=&word[7];
									parser_volume=(float)atof(pstrval);
								}
								else if(strncmp(word,"pan",3)==0)			// pan
								{
									char* pstrval=&word[4];
									parser_pan=(float)atof(pstrval);
								}
								else if(strncmp(word,"loopstart",9)==0)		// loopstart
								{
									char* pstrval=&word[10];
									parser_loopstart=atoi(pstrval);
								}
								else if(strncmp(word,"loopend",7)==0)		// loopend
								{
									char* pstrval=&word[8];
									parser_loopend=atoi(pstrval);
								}
								else if(strncmp(word,"loopmode",8)==0)		// loopmode
								{
									char* pstrval=&word[9];

									// no loop
									if(strncmp(pstrval,"no_loop",7)==0)
										parser_loopmode=0;

									// loop forward
									if(strncmp(pstrval,"loop_continuous",15)==0)
										parser_loopmode=1;
									
									// loop bidirectional
									if(strncmp(pstrval,"loop_bidirectional",18)==0)
										parser_loopmode=2;

									// loop backward
									if(strncmp(pstrval,"loop_backward",13)==0)
										parser_loopmode=3;

									// loop forward sustained
									if(strncmp(pstrval,"loop_sustain",12)==0)
										parser_loopmode=4;
								}
								else if(strncmp(word,"locc",4)==0)			// locc
								{
									char ccstr[4];
									ccstr[0]=0;
									char* pstrval_ccn=&word[4];
									char* pstrval=&word[5];

									// one digit cc
									if(pstrval_ccn[1]=='=')
									{
										ccstr[0]=pstrval_ccn[0];
										ccstr[1]=0;
										pstrval=&pstrval_ccn[2];
									}

									// two digit cc
									if(pstrval_ccn[2]=='=')
									{
										ccstr[0]=pstrval_ccn[0];
										ccstr[1]=pstrval_ccn[1];
										ccstr[2]=0;
										pstrval=&pstrval_ccn[3];
									}

									// three digit cc
									if(pstrval_ccn[3]=='=')
									{
										ccstr[0]=pstrval_ccn[0];
										ccstr[1]=pstrval_ccn[1];
										ccstr[2]=pstrval_ccn[2];
										ccstr[3]=0;
										pstrval=&pstrval_ccn[4];
									}

									int const ccn=atoi(ccstr);

									if(ccn>=0 && ccn<128)
										pz->lo_input_range.midi_cc[ccn]=atoi(pstrval);
								}
								else if(strncmp(word,"hicc",4)==0)			// hicc
								{
									char ccstr[4];
									ccstr[0]=0;
									char* pstrval_ccn=&word[4];
									char* pstrval=&word[5];

									// one digit cc
									if(pstrval_ccn[1]=='=')
									{
										ccstr[0]=pstrval_ccn[0];
										ccstr[1]=0;
										pstrval=&pstrval_ccn[2];
									}

									// two digit cc
									if(pstrval_ccn[2]=='=')
									{
										ccstr[0]=pstrval_ccn[0];
										ccstr[1]=pstrval_ccn[1];
										ccstr[2]=0;
										pstrval=&pstrval_ccn[3];
									}

									// three digit cc
									if(pstrval_ccn[3]=='=')
									{
										ccstr[0]=pstrval_ccn[0];
										ccstr[1]=pstrval_ccn[1];
										ccstr[2]=pstrval_ccn[2];
										ccstr[3]=0;
										pstrval=&pstrval_ccn[4];
									}

									int const ccn=atoi(ccstr);

									if(ccn>=0 && ccn<128)
										pz->hi_input_range.midi_cc[ccn]=atoi(pstrval);
								}
								else if(strncmp(word,"lobend",6)==0)			// lobend
								{
									char* pstrval=&word[7];
									pz->lo_input_range.midi_bend=atoi(pstrval);
								}
								else if(strncmp(word,"hibend",6)==0)			// hibend
								{
									char* pstrval=&word[7];
									pz->hi_input_range.midi_bend=atoi(pstrval);
								}
								else if(strncmp(word,"lochanaft",9)==0)			// lochanaft
								{
									char* pstrval=&word[10];
									pz->lo_input_range.midi_chanaft=atoi(pstrval);
								}
								else if(strncmp(word,"hichanaft",9)==0)			// hichanaft
								{
									char* pstrval=&word[10];
									pz->hi_input_range.midi_chanaft=atoi(pstrval);
								}
								else if(strncmp(word,"lopolyaft",9)==0)			// lopolyaft
								{
									char* pstrval=&word[10];
									pz->lo_input_range.midi_polyaft=atoi(pstrval);
								}
								else if(strncmp(word,"hipolyaft",9)==0)			// hipolyaft
								{
									char* pstrval=&word[10];
									pz->hi_input_range.midi_polyaft=atoi(pstrval);
								}
								else if(strncmp(word,"trigger=release",15)==0)	// trigger release
								{
									pz->midi_trigger=1;
								}
								else if(strncmp(word,"group",5)==0)				// group
								{
									char* pstrval=&word[6];
									pz->res_group=atoi(pstrval);
								}
								else if(strncmp(word,"offby",5)==0)				// offby
								{
									char* pstrval=&word[6];
									pz->res_offby=atoi(pstrval);
								}

								// update last pos
								lp=lc;
							}
							else
							{
								wc++;
							}
						}
					}
					else
					{
						// comment line
					}
				}
			}

			// parse last region
			if(pz!=NULL)
			{
				pz->lo_input_range.midi_key=parser_lokey;
				pz->hi_input_range.midi_key=parser_hikey;
				pz->lo_input_range.midi_vel=parser_lovel;
				pz->hi_input_range.midi_vel=parser_hivel;
				pz->midi_fine_tune=parser_tune;
				pz->midi_coarse_tune=parser_transpose;
				pz->mp_gain=int(parser_volume);
				pz->mp_pan=int(parser_pan*0.5f);	

				// override loop points
				if(parser_loopmode>=0)
				{
					pz->loop_mode=parser_loopmode;

					if(parser_loopstart>=0)
						pz->loop_start=parser_loopstart;

					if(parser_loopend>=0)
						pz->loop_end=parser_loopend;
				}

				// override keyroot
				if(parser_root_key>=0)
					pz->midi_root_key=parser_root_key;
			}

			// close file
			fclose(pfile);
		}
	}

	// sample editor should adapt
	user_sed_adapt=1;
	user_sed_zone=0;

	// enter critical section
    set_suspended (0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLife::sfz_export()
{
	// get program and wavetable pointers
	HIGHLIFE_PROGRAM* pprog=&highlife_program[user_program];

	// filename holder
	char dirpath[MAX_PATH];
	sprintf(dirpath,pprog->name);

    FileChooser myChooser ("discoDSP HighLife - Export Sfz Program",
                           File::nonexistent,
                           "*.*");

	if(myChooser.browseForFileToSave(true))
	{
        File directory (myChooser.getResult());

        if (directory.existsAsFile ())
            directory = directory.getParentDirectory ();
        
        if (! directory.exists())
            directory.createDirectory ();

        File sfzFile = directory.getChildFile (String (pprog->name) + ".sfz");

		// create and save sfz file
		FILE* pfile = fopen (sfzFile.getFullPathName (),"w");

        printf ("%s\n", (const char*) sfzFile.getFullPathName ());

		// stream out file
		if(pfile)
		{
			// comment file
			fprintf(pfile,"/////////////////////////////////////////////////////////////////////////////\n");
			fprintf(pfile,"/// sfz definition file \n");
			fprintf(pfile,"/// copyright rgc:audio 2004 \n");
			fprintf(pfile,"/// -------------------------------------------------------------------------\n");
			fprintf(pfile,"/// Application: HighLife \n");
			fprintf(pfile,"/// Vendor: discoDSP \n");
			fprintf(pfile,"/// -------------------------------------------------------------------------\n");
			fprintf(pfile,"/// Program Name: %s \n",pprog->name);
			fprintf(pfile,"/// Num. Regions: %d \n",pprog->num_zones);
			
			// comment end
			fprintf(pfile,"\n");
			fprintf(pfile,"\n");
			fprintf(pfile,"\n");

			// write zone (regions in sfz)
			for(int z=0;z<pprog->num_zones;z++)
			{
				HIGHLIFE_ZONE* pz=&pprog->pzones[z];

				// region comments
				fprintf(pfile,"/////////////////////////////////////////////////////////////////////////////\n");
				fprintf(pfile,"// Region Name: %s \n",pz->name);
				fprintf(pfile,"\n");
				
				// region opcodes
				fprintf(pfile,"<region>\n");
				fprintf(pfile,"sample=%s_%.4d_%s.wav\n",pprog->name,z,pz->name);

				// write key range
				if(pz->lo_input_range.midi_key>0 || pz->hi_input_range.midi_key<127)
					fprintf(pfile,"lokey=%d hikey=%d\n",pz->lo_input_range.midi_key,pz->hi_input_range.midi_key);

				// write vel range
				if(pz->lo_input_range.midi_vel>0 || pz->hi_input_range.midi_vel<127)
					fprintf(pfile,"lovel=%d hivel=%d\n",pz->lo_input_range.midi_vel,pz->hi_input_range.midi_vel);
				
				// write controllers
				for(int cc=0;cc<128;cc++)
				{
					int const locc=pz->lo_input_range.midi_cc[cc];
					int const hicc=pz->hi_input_range.midi_cc[cc];

					// only if change
					if(locc>0 || hicc<127)
						fprintf(pfile,"locc%d=%d hicc%d=%d\n",cc,locc,cc,hicc);
				}
	
				// write bend range
				if(pz->lo_input_range.midi_bend>-8192 || pz->hi_input_range.midi_bend<8192)
					fprintf(pfile,"lobend=%d hibend=%d\n",pz->lo_input_range.midi_bend,pz->hi_input_range.midi_bend);
	
				// write chanaft range
				if(pz->lo_input_range.midi_chanaft>0 || pz->hi_input_range.midi_chanaft<127)
					fprintf(pfile,"lochanaft=%d hichanaft=%d\n",pz->lo_input_range.midi_chanaft,pz->hi_input_range.midi_chanaft);
				
				// write polyaft range
				if(pz->lo_input_range.midi_polyaft>0 || pz->hi_input_range.midi_polyaft<127)
					fprintf(pfile,"lopolyaft=%d hipolyaft=%d\n",pz->lo_input_range.midi_polyaft,pz->hi_input_range.midi_polyaft);
	
				// write trigger mode
				if(pz->midi_trigger==1)
					fprintf(pfile,"trigger=release");
	
				// write group
				if(pz->res_group)
					fprintf(pfile,"group=%d\n",pz->res_group);
		
				// write off_by
				if(pz->res_offby)
					fprintf(pfile,"off_by=%d\n",pz->res_offby);
	
				// performance separators
				fprintf(pfile,"\n");

				// write offset
				if(pz->cue_pos[0] && pz->num_cues>0)
					fprintf(pfile,"offset=%d\n",pz->cue_pos[0]);

				// write loop mode
				if(pz->loop_mode)
				{
					if(pz->loop_mode==0)fprintf(pfile,"loopmode=no_loop\n");
					if(pz->loop_mode==1)fprintf(pfile,"loopmode=loop_continuous\n");
					if(pz->loop_mode==2)fprintf(pfile,"loopmode=loop_bidirectional\n");
					if(pz->loop_mode==3)fprintf(pfile,"loopmode=loop_backward\n");
					if(pz->loop_mode==4)fprintf(pfile,"loopmode=loop_sustain\n");

					// write loop start and loop end
					fprintf(pfile,"loop_start=%d loop_end=%d\n",pz->loop_start,pz->loop_end);
				}

				// write tune
				if(pz->midi_fine_tune)
					fprintf(pfile,"tune=%d\n",pz->midi_fine_tune);

				// write root key
				if(pz->midi_root_key!=60)
					fprintf(pfile,"pitch_keycenter=%d\n",pz->midi_root_key);

				// write keycents
				if(pz->midi_keycents!=100)
					fprintf(pfile,"pitch_keytrack=%d\n",pz->midi_keycents);

				// write transpose
				if(pz->midi_coarse_tune)
					fprintf(pfile,"transpose=%d\n",pz->midi_coarse_tune);
	
				// write volume
				if(pz->mp_gain)
					fprintf(pfile,"volume=%.1f\n",float(pz->mp_gain));

				// write pan
				if(pz->mp_pan)
					fprintf(pfile,"pan=%.1f\n",float(pz->mp_pan)*2.0f);

				// region end
				fprintf(pfile,"\n");
				fprintf(pfile,"\n");
				fprintf(pfile,"\n");
			}

			fclose(pfile);
		}

		// save zones (sfz regions) wav files
		for(int z=0;z<pprog->num_zones;z++)
		{
			HIGHLIFE_ZONE* pz=&pprog->pzones[z];

			char wavpath[MAX_PATH];
			sprintf (wavpath, "%s_%.4d_%s.wav", pprog->name, z, pz->name);
            File waveFile = directory.getChildFile (wavpath);

            printf ("%s\n", (const char*) waveFile.getFullPathName ());

			wav_save (&pprog->pzones[z], waveFile);
		}
	}
}

