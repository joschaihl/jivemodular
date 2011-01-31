
#include "MidiBindingsEditor.h"

// Component for editing a single midi binding.
class MidiBindingEditorContent : public Component, public ComboBoxListener
{
public:
   MidiBindingEditorContent()
   {
      addAndMakeVisible(bindToLabel = new Label("BindTo", "Bind to:")); 

      addAndMakeVisible(triggerValue = new Slider("CC/Note")); 
      triggerValue->setSliderStyle(Slider::IncDecButtons);
      triggerValue->setRange(-1, 127, 1); // sickness, allow -1 for disabling binding... lazy ui ..

      addAndMakeVisible(modeCombo = new ComboBox("Binding Mode"));
      modeCombo->addItem("Note off", 1);
      modeCombo->addItem("Note on", 2);
      modeCombo->addItem("Note held", 3);
      modeCombo->addItem("Controller", 4);

      addAndMakeVisible(rangeMaxLabel = new Label("RangeMax", "Range max:")); // checkbox coming here
      addAndMakeVisible(rangeMinLabel = new Label("RangeMin", "Range min:")); // checkbox coming here

      addAndMakeVisible(maxSlider = new Slider("Maximum")); 
      maxSlider->setSliderStyle(Slider::LinearBar);
      maxSlider->setRange(0., 1., 1.0/127);
      addAndMakeVisible(minSlider = new Slider("Minimum")); 
      minSlider->setSliderStyle(Slider::LinearBar);
      minSlider->setRange(0., 1., 1.0/127);

      addAndMakeVisible(incrModeLabel = new Label("StepMode", "Step Mode:")); 
      addAndMakeVisible(incrStepLabel = new Label("Step", "Step:")); 
      addAndMakeVisible(incrMinLabel = new Label("StepMin", "Step min:")); 
      addAndMakeVisible(incrMaxLabel = new Label("StepMax", "Step max:")); 

      addAndMakeVisible(incrDirection = new ComboBox("Increment Mode")); 
      incrDirection->addItem("Decrease", Decrease);
      incrDirection->addItem("Increase", Increase);
      incrDirection->addItem("Bidirectional", Bidirectional);
      incrDirection->addItem("Set to value", SetToValue);

      addAndMakeVisible(incrAmount = new ComboBox("Increment")); 
      incrAmount->addItem("1 (toggle)", 1);
      incrAmount->addItem("1/2", 2);
      incrAmount->addItem("1/3", 3);
      incrAmount->addItem("1/4", 4);
      incrAmount->addItem("1/5", 5);
      incrAmount->addItem("1/8", 8);
      incrAmount->addItem("1/16", 16);
      incrAmount->addItem("1/32", 32);
      incrAmount->addItem("1/127", 127);
      incrAmount->addListener(this);

      addAndMakeVisible(incrMax = new Slider("IncrMax")); 
      incrMax->setSliderStyle(Slider::LinearBar);
      addAndMakeVisible(incrMin = new Slider("IncrMin")); 
      incrMin->setSliderStyle(Slider::LinearBar);

      addAndMakeVisible(velocityScalingLabel = new Label("VelocitySensitive", "Velocity Sensitive:")); 
      addAndMakeVisible(velocityScaling = new Slider("VelocitySensitivity")); 
      velocityScaling->setSliderStyle(Slider::LinearBar);
      velocityScaling->setRange(0., 1., 1.0/127);

      setSize(400, 140);
   }
   
   void resized() 
   {
      int pad = 3;
      int h = getHeight() / 4;
      int h2 = getHeight() / 5;
      int w = getWidth() / 2 - pad;
      int labelw = 80 - pad;
      int itemw = w-labelw;
      int itemh = h-pad;
      int itemh2 = h2-pad;
      
      // left column
      bindToLabel->setBounds(pad, pad, labelw, itemh);
      modeCombo->setBounds(labelw, pad, itemw, itemh);
      triggerValue->setBounds(labelw, h*1, itemw, itemh);

      rangeMinLabel->setBounds(pad, 2*h, labelw, itemh);
      rangeMaxLabel->setBounds(pad, 3*h, labelw, itemh);
      minSlider->setBounds(labelw, 2*h, itemw, itemh);
      maxSlider->setBounds(labelw, 3*h, itemw, itemh);

      // right column
      int l = w+pad;
      incrModeLabel->setBounds(l, pad, labelw, itemh2);
      incrStepLabel->setBounds(l, 1*h2, labelw, itemh2);
      incrMinLabel->setBounds(l, 2*h2, labelw, itemh2);
      incrMaxLabel->setBounds(l, 3*h2, labelw, itemh2);
      velocityScalingLabel->setBounds(l, 4*h2, labelw, itemh2);
      l += labelw;
      incrDirection->setBounds(l, pad, itemw, itemh2);
      incrAmount->setBounds(l, h2, itemw, itemh2);
      incrMin->setBounds(l, 2*h2, itemw, itemh2);
      incrMax->setBounds(l, 3*h2, itemw, itemh2);
      velocityScaling->setBounds(l, 4*h2, itemw, itemh2);
   }
   
   ~MidiBindingEditorContent()
   {
      deleteAllChildren();
   }
   
   virtual void comboBoxChanged (ComboBox* slider)
   {
      if (slider == modeCombo)
      {
      // disable stuff based on note/cc mode..   
      }
      else if (slider == incrDirection)
      {
      // disable/relabel stuff based on step mode/set to value mode
      }
      else if (slider == incrAmount)
      {
         int realMax = incrAmount->getSelectedId();
         incrMax->setRange(1, realMax, 1);
         //incrMax->setValue(realMax);
         if (realMax < incrMax->getValue())
            incrMax->setValue(realMax);
         incrMin->setRange(0, realMax-1, 1);
         //incrMin->setValue(0);
      }
      // todo other sliders
   }

   void putBindingOptions(const MidiBinding& binding);
   void getBindingOptions(MidiBinding& binding);

   Label* bindToLabel;
   Slider* triggerValue;
   ComboBox* modeCombo;
   
   // to do invert checkbox
   
   Label* rangeMaxLabel;
   Label* rangeMinLabel;
   Slider* maxSlider;
   Slider* minSlider;

   Label* incrModeLabel;
   Label* incrStepLabel;
   Label* incrMaxLabel;
   Label* incrMinLabel;
   ComboBox* incrDirection;
   ComboBox* incrAmount;
   Slider* incrMax;
   Slider* incrMin;

   Label* velocityScalingLabel;
   Slider* velocityScaling;
};

void MidiBindingEditorContent::putBindingOptions(const MidiBinding& binding)
{
   modeCombo->setSelectedId(binding.getMode() + 1, true);
   triggerValue->setValue(binding.getTriggerValue());

   minSlider->setValue(binding.getMin(), false);
   maxSlider->setValue(binding.getMax(), false);

   double incrAmountd = binding.getIncrAmount();
   int incr = roundToInt(1.0 / fabs(incrAmountd));
   int imax = binding.getIncrMax() / fabs(incrAmountd);
   incrMax->setValue(imax, false);
   int imin = binding.getIncrMin() / fabs(incrAmountd);
   incrMin->setValue(imin, false);

   incrAmount->setSelectedId(incr);
      
   incrDirection->setSelectedId(binding.getStepMode());

   velocityScaling->setValue(binding.getVelocityScaling(), false);
}

void MidiBindingEditorContent::getBindingOptions(MidiBinding& binding)
{
   binding.setMode(modeCombo->getSelectedId() - 1);
   binding.setTriggerValue(triggerValue->getValue());

   binding.setMin(minSlider->getValue());
   binding.setMax(maxSlider->getValue());

   double incrAmountd = 1.0 / incrAmount->getSelectedId();
   binding.setIncrAmount(incrAmountd);
   binding.setStepMin(incrAmountd * incrMin->getValue());
   binding.setStepMax(incrAmountd * incrMax->getValue());
   binding.setStepMode(BindingStepMode(incrDirection->getSelectedId()));
   binding.setVelocityScaling(velocityScaling->getValue());
}

MidiBindingsEditor::MidiBindingsEditor(MidiAutomatable* parameter_)
:
   parameter(parameter_),
   currentBinding(-1)
{
   addAndMakeVisible(addButton = new TextButton("Add"));
   addButton->addButtonListener(this);
   addAndMakeVisible(removeButton = new TextButton("Delete"));
   removeButton->addButtonListener(this);

   addAndMakeVisible(bindingsList = new TableListBox("BindingsList", this));
   TableHeaderComponent* header = bindingsList->getHeader();
   header->addColumn("Mode", Mode, 100, 20, -1);
   header->addColumn("Note/CC", TriggerVal, 100, 20, -1);
   
   addChildComponent(bindingEditor = new MidiBindingEditorContent());

   bindingsList->updateContent();
   bindingsList->selectRow(0);
   
   setSize(400, 350);
}

MidiBindingsEditor::~MidiBindingsEditor()
{
   saveCurBindingOpts();
   deleteAllChildren();
}

void MidiBindingsEditor::resized()
{
   int width = getWidth()-10;
   int height = getHeight();
   int edH = 130;
   int buttonH = 40;
   int lh = height - edH - buttonH;

   if (bindingsList)
      bindingsList->setBounds(5, 5, width, lh - 5);
   
   if (addButton && removeButton)
   {
      int addw = width / 3;
      addButton->setBounds(5, lh, addw*2-5, buttonH - 5);
      removeButton->setBounds(addw*2, lh, addw-5, buttonH - 5);
   }
   
   if (bindingEditor)
      bindingEditor->setBounds(5, lh + buttonH, width, edH);
}

int MidiBindingsEditor::getNumRows()
{
   return parameter ? parameter->getNumBindings() : 0;
}

void MidiBindingsEditor::paintRowBackground (Graphics& g,
                            int rowNumber,
                            int width, int height,
                            bool rowIsSelected)
{
   if (rowIsSelected)
      g.setColour(Colours::yellow);
   else
      g.setColour(Colours::orange);
   g.fillRect(0, 0, width, height);
}

void MidiBindingsEditor::paintCell (Graphics& g,
                         int rowNumber,
                         int columnId,
                         int width, int height,
                         bool rowIsSelected)
{
   String string("-");
   MidiBinding* bp = parameter->getBinding(rowNumber);
   if (bp)
   {
      if (columnId == Mode)
      {
      // move to a method on MidiBinding!!
      // and use in the dropdown in MidiBindingEditorContent!
         switch (bp->getMode()) {
            case NoteOff:
               string = "NoteOff";
               break;
            case NoteOn:
               string = "NoteOn";
               break;
            case NoteHeld:
               string = "NoteHeld";
               break;
            case Controller:
               string = "CC";
               break;
         }
      }
      else if (columnId == TriggerVal)
         string = String(bp->getTriggerValue());
   }
   g.setColour(Colours::black);
   g.drawText(string, 0, 0, width, height, Justification(Justification::horizontallyCentred | Justification::verticallyCentred), true);
}

void MidiBindingsEditor::showCurBindingOpts()
{
   int sel = bindingsList->getSelectedRow();
   MidiBinding* bp = parameter->getBinding(sel);   
   if (bp)
   {
      bindingEditor->putBindingOptions(*bp);      
      bindingEditor->setVisible(true);      
      currentBinding = sel;
   }
}

void MidiBindingsEditor::saveCurBindingOpts()
{
   MidiBinding* bp = parameter->getBinding(currentBinding);   
   if (bp)
   {
      bindingEditor->getBindingOptions(*bp);
      parameter->RegisterBinding(currentBinding);   
   }
}

void MidiBindingsEditor::selectedRowsChanged (int lastRowSelected)
{
   saveCurBindingOpts();
   showCurBindingOpts();
}

void MidiBindingsEditor::buttonClicked (Button* button)
{
   if (button == addButton)
   {
      // set up a new binding similar to the current one.. could do more here
      int curTrig = 0;
      NoteBindingMode curMode = Controller;
      MidiBinding* bp = parameter->getBinding(currentBinding);
      if (bp)
      {
         curTrig = bp->getTriggerValue();
         curMode = bp->getMode();
      }
      int curBinding = 0;
      if (curMode == Controller)
         curBinding = parameter->addControllerNumber(curTrig);
      else
         curBinding = parameter->addNoteNumber(curTrig);
      bindingsList->selectRow(curBinding);
   }
   else if (button == removeButton)
   {
      parameter->removeBinding(currentBinding);
   }
   bindingsList->updateContent();   
}


