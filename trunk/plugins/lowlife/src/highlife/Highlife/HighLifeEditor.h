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
// HighLife Editor Header                                                                                                              //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef __HIGHLIFE_EDITOR_HEADER_H__
#define __HIGHLIFE_EDITOR_HEADER_H__

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "../StandardHeader.h"

// ddsp includes
#include "HighLifeGuiMenu.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CHighLife;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define K_EDITOR_WIDTH		776
#define K_EDITOR_HEIGHT		384
#define K_EDITOR_KEY_HEIGHT 42

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CHighLifeEditor : public AudioProcessorEditor,
                        public ChangeListener,
                        public Timer
{
public:

	CHighLifeEditor (CHighLife *effect);
	~CHighLifeEditor ();

public:
    virtual void 	changeListenerCallback (void *objectThatHasChanged);
    virtual void 	mouseMove (const MouseEvent &e);
    virtual void 	mouseDown (const MouseEvent &e);
    virtual void 	mouseDrag (const MouseEvent &e);
    virtual void 	mouseUp (const MouseEvent &e);
    virtual void    mouseWheelMove (const MouseEvent &e, float wheelX, float wheelY);
    virtual void 	paint (Graphics &g);
    virtual void 	timerCallback ();
    virtual void    resized ();

public:
	bool gui_verify_coord (int x, int y, int zx, int zy, int cx, int cy);
	void gui_bitmap (Graphics* const hdc,int const x,int const y,int const w,int const h,int const xo,int const yo,Image* const phbitmap);
	bool gui_bitmap_ab (Graphics* const hdc,int const x,int const y,int const w,int const h,int const xo,int const yo,Image* const phbitmap,int const trans);
	void gui_solid_rect (Graphics* const hdc,int const x,int const y,int const w,int const h,const Colour& color);
	void gui_invert (Graphics* const hdc,int const x,int const y,int const w,int const h);
	int	 gui_get_int_mode (void);
	void gui_set_int_mode (int const im);
	void gui_tweak_int (int& i_param, int const amount, int const min, int const max);
	void gui_sed_center_around_sel (void);
	int	 gui_sed_sample_to_coord (int const sample);
	int	 gui_sed_coord_to_sample (int const x);
	void gui_sed_fix_sel (int const wave_len, int const snap);

	void gui_mouse_down (int const xm_pos, int const ym_pos, const MouseEvent &e);
	void gui_mouse_move (int const xm_pos, int const ym_pos, const MouseEvent &e);
	void gui_mouse_up (int const xm_pos, int const ym_pos, const MouseEvent &e);
	void gui_command (int const lw_wpar);
	void gui_timer ();
	void gui_refresh ();

public:
	void gui_paint_wheel (Graphics* const hdc, int const x, int const y, int value);
	void gui_paint_knob_12 (Graphics* const hdc, int const x, int const y, float const value);
	void gui_paint_knob_sin (Graphics* const hdc, int const x, int const y, float const value);
	void gui_paint_knob (Graphics* const hdc, int const x, int const y, RTPAR* pp);
	void gui_paint_led (Graphics* const hdc, int const x, int const y, int const mode);
	void gui_paint_but (Graphics* const hdc, int const x, int const y, int const m);
	void gui_paint_fnt (Graphics* const hdc, int x, int y, const char* pchar, int const trunc);
	void gui_paint_dig (Graphics* const hdc, int const x, int const y, int const v);
	void gui_paint_dig_cent (Graphics* const hdc, int const x, int const y, int const v);
	void gui_paint (Graphics* const hdc);

public:

	CHighLife*	    fx;

    // keyboard
	MidiKeyboardComponent* midi_keyboard;

	// gui menus
	CGuiMenu menu_file;
	CGuiMenu menu_view;
	CGuiMenu menu_options;
	CGuiMenu menu_hl_program;
	CGuiMenu menu_vst_program;
	CGuiMenu menu_vel_splits;
	CGuiMenu menu_spl_length;
	CGuiMenu menu_zone;
	CGuiMenu menu_zone_loop;
	CGuiMenu menu_zone_trigger;
	CGuiMenu menu_sample_edit;
	CGuiMenu menu_sample_loop;
	CGuiMenu menu_sample_ampl;
	CGuiMenu menu_sample_filt;
	CGuiMenu menu_sample_spfx;
	CGuiMenu menu_sample_cuep;

	// gui bitamp handlers
	Image* hbitmap_k12;
	Image* hbitmap_but;
//	Image* hbitmap_cfg;
	Image* hbitmap_fnt;
	Image* hbitmap_gui;
	Image* hbitmap_ged;
	Image* hbitmap_kna;
	Image* hbitmap_knb;
	Image* hbitmap_led;
	Image* hbitmap_whl;
	Image* hbitmap_lop;
	Image* hbitmap_kbd;
	Image* hbitmap_dig;
	
	// gui cursor handlers
	MouseCursor	hcursor_diag;
	MouseCursor	hcursor_move;
	MouseCursor	hcursor_arro;
	MouseCursor	hcursor_szwe;
	MouseCursor	hcursor_szns;
	MouseCursor	hcursor_beam;
};

#endif

