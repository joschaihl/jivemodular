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
// HighLife Editor Implementation                                                                                                      //
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Highlife.h"
#include "HighLifeEditor.h"
#include "../Resources/Resources.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class HighlifeLookAndFeel : public LookAndFeel
{
public:

    HighlifeLookAndFeel()
    {
    }

    virtual ~HighlifeLookAndFeel()
    {
    }

    const Font getPopupMenuFont()
    {
        return Font (14.0f);
    }

    juce_UseDebuggingNewOperator
};

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHighLifeEditor::CHighLifeEditor (CHighLife *effect)
    : AudioProcessorEditor(effect),
      fx (effect)
{
    // define look and feel
    static HighlifeLookAndFeel laf;
    LookAndFeel::setDefaultLookAndFeel (&laf);

    // define cursors
    hcursor_diag=MouseCursor (MouseCursor::TopLeftCornerResizeCursor);
	hcursor_move=MouseCursor (MouseCursor::CrosshairCursor);
	hcursor_arro=MouseCursor (MouseCursor::NormalCursor);
	hcursor_szwe=MouseCursor (MouseCursor::LeftRightResizeCursor);
	hcursor_szns=MouseCursor (MouseCursor::UpDownResizeCursor);
	hcursor_beam=MouseCursor (MouseCursor::IBeamCursor);

    // main menus set id
    menu_file.SetID(16128);
    menu_view.SetID(16256);
    menu_options.SetID(16384);

    // big lcd menus set id
    menu_hl_program.SetID(16384+1024);
    menu_vst_program.SetID(16384+1280);

    // freezer menus set id
    menu_vel_splits.SetID(16384+512);
    menu_spl_length.SetID(16384+768);

    // zone menus set id
    menu_zone.SetID(16384+1536);
    menu_zone_loop.SetID(16032);
    menu_zone_trigger.SetID(16064);

    // sample editor menus set id
    menu_sample_edit.SetID(16384+256);
    menu_sample_loop.SetID(16384+264);
    menu_sample_ampl.SetID(16384+272);
    menu_sample_filt.SetID(16384+280);
    menu_sample_spfx.SetID(16384+288);
    menu_sample_cuep.SetID(16384+306);

	// menu file
	menu_file.AddTextOption("New Program...");
	menu_file.AddTextOption("New Bank...");
	menu_file.AddSeparator();
	menu_file.AddTextOption("Import Sample(s)...");
	menu_file.AddTextOption("Import Akai/Sfz/Sf2/Gig Program...");
	menu_file.AddSeparator();
	menu_file.AddTextOption("Export Wav File(s)...");
	menu_file.AddTextOption("Export Sfz Program...");

	// menu view
	menu_view.AddTextOption("Performance");
	menu_view.AddTextOption("Editor");
	
	// menu options
	menu_options.AddTextOption("Panic");
	menu_options.AddSeparator();
	menu_options.AddTextOption("Change sample on midi note");
	menu_options.AddSeparator();
	menu_options.AddTextOption("Engine: Hermite (Realtime)");
	menu_options.AddTextOption("Engine: Sinc-64 (Bounce)");
	menu_options.AddTextOption("Engine: Sinc-512 (Mastering)");

	// menu vel splits options
	menu_vel_splits.AddTextOption("1");
	menu_vel_splits.AddTextOption("2");
	menu_vel_splits.AddTextOption("4");
	menu_vel_splits.AddTextOption("8");
	menu_vel_splits.AddTextOption("16");
	menu_vel_splits.AddTextOption("32");
	menu_vel_splits.AddTextOption("64");
	menu_vel_splits.AddTextOption("128");

	// menu length split options
	menu_spl_length.AddTextOption("1''");
	menu_spl_length.AddTextOption("2''");
	menu_spl_length.AddTextOption("4''");
	menu_spl_length.AddTextOption("8''");
	menu_spl_length.AddTextOption("16''");

	// menu zone loop
	menu_zone_loop.AddTextOption("Off");
	menu_zone_loop.AddTextOption("Forward");
	menu_zone_loop.AddTextOption("Bi-Directional");
	menu_zone_loop.AddTextOption("Backward");
	menu_zone_loop.AddTextOption("Forward Sustained");

	// menu zone trigger
	menu_zone_trigger.AddTextOption("Attack");
	menu_zone_trigger.AddTextOption("Release");
	
	// menu sample editor edit
	menu_sample_edit.AddTextOption("Cut");
	menu_sample_edit.AddTextOption("Copy");
	menu_sample_edit.AddTextOption("Paste");
	menu_sample_edit.AddTextOption("Trim");
	menu_sample_edit.AddSeparator();
	menu_sample_edit.AddTextOption("Select All");
	
	// menu sample editor loop
	menu_sample_loop.AddTextOption("Set Loop Start");
	menu_sample_loop.AddTextOption("Set Loop End");
	menu_sample_loop.AddTextOption("Set Loop");

	// menu sample editor ampl
	menu_sample_ampl.AddTextOption("Fade In");
	menu_sample_ampl.AddTextOption("Fade Out");
	menu_sample_ampl.AddSeparator();
	menu_sample_ampl.AddTextOption("Normalize");
	menu_sample_ampl.AddTextOption("DC Remove");
	
	// menu sample editor filt
	menu_sample_filt.AddTextOption("Smooth");
	menu_sample_filt.AddTextOption("Enhance");
	
	// menu sample editor spfx
	menu_sample_spfx.AddTextOption("Reverse");
	menu_sample_spfx.AddTextOption("Rectifier");
	menu_sample_spfx.AddSeparator();
	menu_sample_spfx.AddTextOption("Sin Drive");
	menu_sample_spfx.AddTextOption("Tanh Drive");
	menu_sample_spfx.AddSeparator();
	menu_sample_spfx.AddTextOption("Spectral Mirror");
	menu_sample_spfx.AddSeparator();
	menu_sample_spfx.AddTextOption("Pitch Shift 0.5x");
	menu_sample_spfx.AddTextOption("Pitch Shift 2.0x");
	menu_sample_spfx.AddSeparator();
	menu_sample_spfx.AddTextOption("Time Stretch 0.5x");
	menu_sample_spfx.AddTextOption("Time Stretch 2.0x");

	// menu sample editor cuep
	menu_sample_cuep.AddTextOption("Add Cue Marker");
	menu_sample_cuep.AddTextOption("Remove Cue(s)");
	menu_sample_cuep.AddSeparator();
	menu_sample_cuep.AddTextOption("Auto 20%");
	menu_sample_cuep.AddTextOption("Auto 40%");
	menu_sample_cuep.AddTextOption("Auto 60%");
	menu_sample_cuep.AddTextOption("Auto 80%");

    menu_file.addChangeListener (this);
	menu_view.addChangeListener (this);
	menu_options.addChangeListener (this);
	menu_hl_program.addChangeListener (this);
	menu_vst_program.addChangeListener (this);
	menu_vel_splits.addChangeListener (this);
	menu_spl_length.addChangeListener (this);
	menu_zone.addChangeListener (this);
	menu_zone_loop.addChangeListener (this);
	menu_zone_trigger.addChangeListener (this);
	menu_sample_edit.addChangeListener (this);
	menu_sample_loop.addChangeListener (this);
	menu_sample_ampl.addChangeListener (this);
	menu_sample_filt.addChangeListener (this);
	menu_sample_spfx.addChangeListener (this);
	menu_sample_cuep.addChangeListener (this);

    hbitmap_but = ImageFileFormat::loadFrom (Resources::buttons, Resources::buttons_size);
	hbitmap_fnt = ImageFileFormat::loadFrom (Resources::fixed_font, Resources::fixed_font_size);
	hbitmap_gui = ImageFileFormat::loadFrom (Resources::gui, Resources::gui_size);
	hbitmap_ged = ImageFileFormat::loadFrom (Resources::gui_edit, Resources::gui_edit_size);
	hbitmap_kna = ImageFileFormat::loadFrom (Resources::knob_alpha, Resources::knob_alpha_size);
	hbitmap_knb = ImageFileFormat::loadFrom (Resources::knob, Resources::knob_size);
	hbitmap_led = ImageFileFormat::loadFrom (Resources::led, Resources::led_size);
	hbitmap_whl = ImageFileFormat::loadFrom (Resources::pb, Resources::pb_size);
	hbitmap_k12 = ImageFileFormat::loadFrom (Resources::knob_12, Resources::knob_12_size);
	hbitmap_lop = ImageFileFormat::loadFrom (Resources::loop, Resources::loop_size);
	hbitmap_kbd = ImageFileFormat::loadFrom (Resources::keyboard, Resources::keyboard_size);
	hbitmap_dig = ImageFileFormat::loadFrom (Resources::digits, Resources::digits_size);

    addAndMakeVisible (midi_keyboard = new MidiKeyboardComponent (*(effect->getKeyboardstate()),
                                                                  MidiKeyboardComponent::horizontalKeyboard));

    int channelsToDisplay = 0;
    for (int i = 0; i < 16; i++)
        channelsToDisplay |= (1 << i);

    midi_keyboard->setMidiChannelsToDisplay (channelsToDisplay);
    midi_keyboard->setMidiChannel (1);
    midi_keyboard->setKeyPressBaseOctave (3);
    midi_keyboard->setLowestVisibleKey (12);
    midi_keyboard->setKeyWidth (11);
    midi_keyboard->setColour (MidiKeyboardComponent::upDownButtonBackgroundColourId, Colours::black);
    midi_keyboard->setColour (MidiKeyboardComponent::upDownButtonArrowColourId, Colours::white);

    // set size
    setSize (K_EDITOR_WIDTH, K_EDITOR_HEIGHT + K_EDITOR_KEY_HEIGHT);
    
    // start timer
    startTimer (1000 / 25);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHighLifeEditor::~CHighLifeEditor()
{
    PopupMenu::dismissAllActiveMenus();

    stopTimer ();

	deleteAndZero (midi_keyboard);

    menu_file.removeChangeListener (this);
	menu_view.removeChangeListener (this);
	menu_options.removeChangeListener (this);
	menu_hl_program.removeChangeListener (this);
	menu_vst_program.removeChangeListener (this);
	menu_vel_splits.removeChangeListener (this);
	menu_spl_length.removeChangeListener (this);
	menu_zone.removeChangeListener (this);
	menu_zone_loop.removeChangeListener (this);
	menu_zone_trigger.removeChangeListener (this);
	menu_sample_edit.removeChangeListener (this);
	menu_sample_loop.removeChangeListener (this);
	menu_sample_ampl.removeChangeListener (this);
	menu_sample_filt.removeChangeListener (this);
	menu_sample_spfx.removeChangeListener (this);
	menu_sample_cuep.removeChangeListener (this);

	deleteAndZero (hbitmap_k12);
	deleteAndZero (hbitmap_but);
	deleteAndZero (hbitmap_fnt);
	deleteAndZero (hbitmap_gui);
	deleteAndZero (hbitmap_ged);
	deleteAndZero (hbitmap_kna);
	deleteAndZero (hbitmap_knb);
	deleteAndZero (hbitmap_led);
	deleteAndZero (hbitmap_whl);
	deleteAndZero (hbitmap_lop);
	deleteAndZero (hbitmap_kbd);
	deleteAndZero (hbitmap_dig);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::resized ()
{
    DBG ("CHighLifeEditor::resized");

    int keyHeight = K_EDITOR_KEY_HEIGHT;
    if (midi_keyboard)
    {
        midi_keyboard->setBounds (0,
                                  getHeight() - keyHeight,
                                  getWidth(),
                                  keyHeight);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CHighLifeEditor::mouseDown (const MouseEvent &e)
{
    gui_mouse_down (e.x, e.y, e);
}

void CHighLifeEditor::mouseMove (const MouseEvent &e)
{
    gui_mouse_move (e.x, e.y, e);
}

void CHighLifeEditor::mouseDrag (const MouseEvent &e)
{
    gui_mouse_move (e.x, e.y, e);
}

void CHighLifeEditor::mouseUp (const MouseEvent &e)
{
    gui_mouse_up (e.x, e.y, e);
}

void CHighLifeEditor::mouseWheelMove (const MouseEvent &e, float wheelX, float wheelY)
{
    // gui_mouse_wheel (e.x, e.y, wheelY, e);
}

void CHighLifeEditor::paint (Graphics &g)
{
	g.drawImage ((fx->user_gui_page==0) ? hbitmap_gui : hbitmap_ged,
	             0, 0, getWidth(), getHeight(),
	             0, 0, getWidth(), getHeight());

    gui_paint(&g);

    fx->gui_recent_update=true;
}

void CHighLifeEditor::timerCallback ()
{
    gui_timer ();
}

void CHighLifeEditor::changeListenerCallback (void *objectThatHasChanged)
{
    CGuiMenu* menu = (CGuiMenu*) objectThatHasChanged;

    if (menu)
        gui_command (menu->GetResult());
}

