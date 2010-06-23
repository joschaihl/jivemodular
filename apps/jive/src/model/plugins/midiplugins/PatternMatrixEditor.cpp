
#include "PatternMatrixPlugin.h"

#include "PatternMatrixEditor.h"


//==============================================================================
PatternMatrixEditor::PatternMatrixEditor (PatternMatrixPlugin* owner_)
:
   PluginEditorComponent (owner_),
   owner(owner_),
   quantizeBox(0)
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
   }
   
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
}

//==============================================================================
void PatternMatrixEditor::paint (Graphics& g)
{

}

void PatternMatrixEditor::resized()
{
   int w = getWidth();
   int h = getHeight();
   int ccWidth = 100;
   int partH = h / (MAXPARTS + 1);
   
   quantizeBox->setBounds(40, 10, w-80, partH-20);
   for (int i = 0; i < MAXPARTS; i++)
   {
      if (partStatus[i])
         partStatus[i]->setBounds(0, partH*(i+1), w-ccWidth, partH);
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



