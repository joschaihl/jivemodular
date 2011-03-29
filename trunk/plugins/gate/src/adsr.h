/* adsr.h - Simple ADSR envelope generator
 *
 * Copyright (c) 2010 Michael Gruhn
 * All rights reserved.
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 * PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS
 * ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

#ifndef ADSR_H
#define ADSR_H


#include <math.h>

#define ADSR_SILENCE 0.00001 /* -100dBFS */


#define ADSR_IDLE 0 
#define ADSR_ATTACK 1
#define ADSR_DECAY 2
#define ADSR_SUSTAIN 3
#define ADSR_RELEASE 0 // aka waiting for another ATTACK!

typedef struct adsr_t
{
	int state;
	unsigned s;
	float env;
	float coef;
	float env_target;
	unsigned time;
	
	float d_coef;
	float r_coef;
	float sustain;
	unsigned d_time;
} adsr_t;


inline static void adsr_trigger(adsr_t *adsr, unsigned att_spls,
	unsigned dec_spls, float sus_lev, unsigned rel_spls)
{

	att_spls += 1;
	dec_spls += 1;
	rel_spls += 1;

	adsr->coef = 1.f-exp(-1.f/(float)att_spls);
	adsr->d_coef = 1.f-exp(-1.f/(float)dec_spls);
	adsr->r_coef = 1.f-exp(-1.f/(float)rel_spls);

	adsr->env_target = 1.581;/*976706869327*/ /* == 1/(1-exp(-1)) - something */
	adsr->sustain = sus_lev;

	adsr->time = att_spls;
	adsr->d_time = att_spls+dec_spls;

	adsr->state = ADSR_ATTACK;
	adsr->s = 0;
	adsr->env = ADSR_SILENCE*1.00001f;

}


inline static void adsr_release(adsr_t *adsr)
{
	adsr->state = ADSR_RELEASE;
	adsr->coef = adsr->r_coef;
	adsr->env_target = 0.;
	/* in case adsr->s overflows and stage switch in process gets triggered again */
	adsr->d_coef = adsr->r_coef;
}


inline static float adsr_process(adsr_t *adsr)
{
	if(adsr->state == ADSR_ATTACK && adsr->s >= adsr->time )
	{
		adsr->env_target = adsr->sustain;
		adsr->coef = adsr->d_coef;
		adsr->time = adsr->d_time;
	}
	++adsr->s;
	adsr->env += adsr->coef*(adsr->env_target - adsr->env);
    return adsr->env;
}


#endif /* ADSR_H */


