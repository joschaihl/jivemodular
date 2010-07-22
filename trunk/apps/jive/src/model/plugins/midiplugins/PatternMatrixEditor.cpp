
#include "PatternMatrixPlugin.h"

#include "PatternMatrixEditor.h"


//==============================================================================
PatternMatrixEditor::PatternMatrixEditor (PatternMatrixPlugin* owner_)
:
   PluginEditorComponent (owner_),
   owner(owner_),
   quantizeBox(0),
   gridWidth(0)
//   partStatus(0),
//   partCC(0)
{
   for (int i = 0; i < MAXPARTS; i++)
   {
      addAndMakeVisible(partStatus[i] = new ParamSlider(owner, owner->getParameterObject(i), i));
      partStatus[i]->setTextBoxIsEditable(false);
//      partStatus[i]->setTextBoxStyle(Slider::TextBoxLeft, true, partStatus[i]->getTextBoxWidth(), partStatus[i]->getTextBoxHeight());

      addAndMakeVisible(partCC[i] = new Slider(String("PartCC") + String(i)));
      partCC[i]->setSliderStyle(Slider::IncDecButtons);
      partCC[i]->setRange(0, 127, 1);
      partCC[i]->addListener(this);
   }
   
   addAndMakeVisible (quantizeBox = new ComboBox (String::empty));
   quantizeBox->setEditableText (false);
   quantizeBox->setJustificationType (Justification::centredLeft);
   quantizeBox->addItem (T("off"), 1);
   quantizeBox->addItem (T("1 bar"), 2);
   quantizeBox->addItem (T("1/2 bar"), 3);
   quantizeBox->addItem (T("beat"), 5);
   quantizeBox->addItem (T("8th"), 9);
   quantizeBox->addItem (T("16th"), 17);
   quantizeBox->addItem (T("32th"), 33);
   quantizeBox->addItem (T("64th"), 65);
   quantizeBox->setTooltip (T("Snap"));
   quantizeBox->addListener (this);

   addAndMakeVisible(midiChannel = new Slider(String("MidiChannel")));
   midiChannel->setSliderStyle(Slider::IncDecButtons);
   midiChannel->setRange(1, 16, 1);
   midiChannel->addListener(this);

   owner->addChangeListener(this);
}

PatternMatrixEditor::~PatternMatrixEditor()
{
   owner->removeChangeListener(this);
   deleteAllChildren();
}

double PatternMatrixEditor::comboIdToQuantize(int i)
{
   double q = 0;
   if (i > 1)
      q = (i - 1) / 1.0;
   return q;
}

int PatternMatrixEditor::quantizeToComboId(double q)
{
   int cid = 1;
   if (q > 0)
      cid = roundToInt((q / 1.0) + 1);
   //else cid = 0;
   return cid;
}

void PatternMatrixEditor::updateParameters()
{
   for (int i = 0; i < MAXPARTS; i++)
   {
      if (partStatus[i])
      {
         partStatus[i]->setValue(owner->getPartPattern(i) / static_cast<float>(MAXPATTERNSPERPART));
         partStatus[i]->updateText();      
      }
      if (partCC[i])
         partCC[i]->setValue(owner->getPartCC(i));
   }
   if (quantizeBox)
      quantizeBox->setSelectedId(quantizeToComboId(owner->getQuantize()));
   if (midiChannel)
      midiChannel->setValue(owner->getMidiChannel(), false);
}

//==============================================================================
void PatternMatrixEditor::paint (Graphics& g)
{
   // draw some grid lines so user has easy click targets for switching parts..
   // note that these are underneath the sliders, so are sometimes obscured by slider value
   // in future will move this line drawing to a component & then can add it last so it draws on top
   int height = getHeight();
   int hstep =  gridWidth / MAXPARTS;
   for (int i=1; i<MAXPARTS; i++)
   {
      g.setColour(Colours::black.brighter(0.25));
      g.drawLine(i*hstep, hstep+5, i*hstep, height-5, 1);
   }
}

void PatternMatrixEditor::resized()
{
   int w = getWidth();
   int h = getHeight();
   int ccWidth = 100;
   int partH = h / (MAXPARTS + 1);
   
   gridWidth = w-ccWidth;
   
   midiChannel->setBounds(10, 10, 100, partH-20);
   quantizeBox->setBounds(w-110, 10, 100, partH-20);
   for (int i = 0; i < MAXPARTS; i++)
   {
      if (partStatus[i])
         partStatus[i]->setBounds(0, partH*(i+1), gridWidth, partH);
      if (partCC[i])
         partCC[i]->setBounds(w-ccWidth, partH*(i+1), ccWidth, partH);
   }
}

void PatternMatrixEditor::sliderValueChanged (Slider* sliderThatWasMoved)
{
   for (int i = 0; i < MAXPARTS; i++)
   {
      if (partCC[i] == sliderThatWasMoved)
         owner->setPartCC(i, sliderThatWasMoved->getValue());
   }
   if (midiChannel == sliderThatWasMoved)
      owner->setMidiChannel(sliderThatWasMoved->getValue());
}

void PatternMatrixEditor::comboBoxChanged (ComboBox* comboBoxThatHasChanged)
{
   if (comboBoxThatHasChanged == quantizeBox)
      owner->setQuantize(comboIdToQuantize(quantizeBox->getSelectedId()));
}

void PatternMatrixEditor::changeListenerCallback (void* source)
{
   updateParameters();
}



