/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *  
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#ifndef _FLUIDSYNTH_GEN_H
#define _FLUIDSYNTH_GEN_H

#ifdef __cplusplus
extern "C" {
#endif

/*
fluid_gen_info_t fluid_gen_info[] = {
        // number/name             init  scale         min        max         def 
        { GEN_STARTADDROFS,           1,     1,       0.0f,     1e10f,       0.0f },
        { GEN_ENDADDROFS,             1,     1,     -1e10f,      0.0f,       0.0f },
        { GEN_STARTLOOPADDROFS,       1,     1,     -1e10f,     1e10f,       0.0f },
        { GEN_ENDLOOPADDROFS,         1,     1,     -1e10f,     1e10f,       0.0f },
        { GEN_STARTADDRCOARSEOFS,     0,     1,       0.0f,     1e10f,       0.0f },
        { GEN_MODLFOTOPITCH,          1,     2,  -12000.0f,  12000.0f,       0.0f },
        { GEN_VIBLFOTOPITCH,          1,     2,  -12000.0f,  12000.0f,       0.0f },
        { GEN_MODENVTOPITCH,          1,     2,  -12000.0f,  12000.0f,       0.0f },
        { GEN_FILTERFC,               1,     2,    1500.0f,  13500.0f,   13500.0f },
        { GEN_FILTERQ,                1,     1,       0.0f,    960.0f,       0.0f },
        { GEN_MODLFOTOFILTERFC,       1,     2,  -12000.0f,  12000.0f,       0.0f },
        { GEN_MODENVTOFILTERFC,       1,     2,  -12000.0f,  12000.0f,       0.0f },
        { GEN_ENDADDRCOARSEOFS,       0,     1,     -1e10f,      0.0f,       0.0f },
        { GEN_MODLFOTOVOL,            1,     1,    -960.0f,    960.0f,       0.0f },
        { GEN_UNUSED1,                0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_CHORUSSEND,             1,     1,       0.0f,   1000.0f,       0.0f },
        { GEN_REVERBSEND,             1,     1,       0.0f,   1000.0f,       0.0f },
        { GEN_PAN,                    1,     1,    -500.0f,    500.0f,       0.0f },
        { GEN_UNUSED2,                0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_UNUSED3,                0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_UNUSED4,                0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_MODLFODELAY,            1,     2,  -12000.0f,   5000.0f,  -12000.0f },
        { GEN_MODLFOFREQ,             1,     4,  -16000.0f,   4500.0f,       0.0f },
        { GEN_VIBLFODELAY,            1,     2,  -12000.0f,   5000.0f,  -12000.0f },
        { GEN_VIBLFOFREQ,             1,     4,  -16000.0f,   4500.0f,       0.0f },
        { GEN_MODENVDELAY,            1,     2,  -12000.0f,   5000.0f,  -12000.0f },
        { GEN_MODENVATTACK,           1,     2,  -12000.0f,   8000.0f,  -12000.0f },
        { GEN_MODENVHOLD,             1,     2,  -12000.0f,   5000.0f,  -12000.0f },
        { GEN_MODENVDECAY,            1,     2,  -12000.0f,   8000.0f,  -12000.0f },
        { GEN_MODENVSUSTAIN,          0,     1,       0.0f,   1000.0f,       0.0f },
        { GEN_MODENVRELEASE,          1,     2,  -12000.0f,   8000.0f,  -12000.0f },
        { GEN_KEYTOMODENVHOLD,        0,     1,   -1200.0f,   1200.0f,       0.0f },
        { GEN_KEYTOMODENVDECAY,       0,     1,   -1200.0f,   1200.0f,       0.0f },
        { GEN_VOLENVDELAY,            1,     2,  -12000.0f,   5000.0f,  -12000.0f },
        { GEN_VOLENVATTACK,           1,     2,  -12000.0f,   8000.0f,  -12000.0f },
        { GEN_VOLENVHOLD,             1,     2,  -12000.0f,   5000.0f,  -12000.0f },
        { GEN_VOLENVDECAY,            1,     2,  -12000.0f,   8000.0f,  -12000.0f },
        { GEN_VOLENVSUSTAIN,          0,     1,       0.0f,   1440.0f,       0.0f },
        { GEN_VOLENVRELEASE,          1,     2,  -12000.0f,   8000.0f,  -12000.0f },
        { GEN_KEYTOVOLENVHOLD,        0,     1,   -1200.0f,   1200.0f,       0.0f },
        { GEN_KEYTOVOLENVDECAY,       0,     1,   -1200.0f,   1200.0f,       0.0f },
        { GEN_INSTRUMENT,             0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_RESERVED1,              0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_KEYRANGE,               0,     0,       0.0f,    127.0f,       0.0f },
        { GEN_VELRANGE,               0,     0,       0.0f,    127.0f,       0.0f },
        { GEN_STARTLOOPADDRCOARSEOFS, 0,     1,     -1e10f,     1e10f,       0.0f },
        { GEN_KEYNUM,                 1,     0,       0.0f,    127.0f,      -1.0f },
        { GEN_VELOCITY,               1,     1,       0.0f,    127.0f,      -1.0f },
        { GEN_ATTENUATION,            1,     1,       0.0f,   1440.0f,       0.0f },
        { GEN_RESERVED2,              0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_ENDLOOPADDRCOARSEOFS,   0,     1,     -1e10f,     1e10f,       0.0f },
        { GEN_COARSETUNE,             0,     1,    -120.0f,    120.0f,       0.0f },
        { GEN_FINETUNE,               0,     1,     -99.0f,     99.0f,       0.0f },
        { GEN_SAMPLEID,               0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_SAMPLEMODE,             0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_RESERVED3,              0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_SCALETUNE,              0,     1,       0.0f,   1200.0f,     100.0f },
        { GEN_EXCLUSIVECLASS,         0,     0,       0.0f,      0.0f,       0.0f },
        { GEN_OVERRIDEROOTKEY,        1,     0,       0.0f,    127.0f,      -1.0f },
        { GEN_PITCH,                  1,     0,       0.0f,    127.0f,       0.0f }
};
*/

  /** List of generator numbers
      Soundfont 2.01 specifications section 8.1.3 */
enum fluid_gen_type {
  GEN_STARTADDROFS,
  GEN_ENDADDROFS,
  GEN_STARTLOOPADDROFS,
  GEN_ENDLOOPADDROFS,
  GEN_STARTADDRCOARSEOFS,
  GEN_MODLFOTOPITCH,
  GEN_VIBLFOTOPITCH,
  GEN_MODENVTOPITCH,
  GEN_FILTERFC,
  GEN_FILTERQ,
  GEN_MODLFOTOFILTERFC,
  GEN_MODENVTOFILTERFC,
  GEN_ENDADDRCOARSEOFS,
  GEN_MODLFOTOVOL,
  GEN_UNUSED1,
  GEN_CHORUSSEND,
  GEN_REVERBSEND,
  GEN_PAN,
  GEN_UNUSED2,
  GEN_UNUSED3,
  GEN_UNUSED4,
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
  GEN_VOLENVDELAY,
  GEN_VOLENVATTACK,
  GEN_VOLENVHOLD,
  GEN_VOLENVDECAY,
  GEN_VOLENVSUSTAIN,
  GEN_VOLENVRELEASE,
  GEN_KEYTOVOLENVHOLD,
  GEN_KEYTOVOLENVDECAY,
  GEN_INSTRUMENT,
  GEN_RESERVED1,
  GEN_KEYRANGE,
  GEN_VELRANGE,
  GEN_STARTLOOPADDRCOARSEOFS,
  GEN_KEYNUM,
  GEN_VELOCITY,
  GEN_ATTENUATION,
  GEN_RESERVED2,
  GEN_ENDLOOPADDRCOARSEOFS,
  GEN_COARSETUNE,
  GEN_FINETUNE,
  GEN_SAMPLEID,
  GEN_SAMPLEMODE,
  GEN_RESERVED3,
  GEN_SCALETUNE,
  GEN_EXCLUSIVECLASS,
  GEN_OVERRIDEROOTKEY,

  /* the initial pitch is not a "standard" generator. It is not
   * mentioned in the list of generator in the SF2 specifications. It
   * is used, however, as the destination for the default pitch wheel
   * modulator. */
  GEN_PITCH,
  GEN_LAST 
};


  /*
   *  fluid_gen_t
   *  Sound font generator
   */
typedef struct _fluid_gen_t
{
  unsigned char flags; /* is it used or not */
  double val;          /* The nominal value */
  double mod;          /* Change by modulators */
  double nrpn;         /* Change by NRPN messages */
} fluid_gen_t;

enum fluid_gen_flags
{
  GEN_UNUSED,
  GEN_SET,
  GEN_ABS_NRPN
};

  /** Reset an array of generators to the SF2.01 default values */
FLUIDSYNTH_API int fluid_gen_set_default_values(fluid_gen_t* gen);



#ifdef __cplusplus
}
#endif
#endif /* _FLUIDSYNTH_GEN_H */

