
#include "JiveParamSlider.h"

// Component for editing note bindings..
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

      setSize(400, 140);
   }
   
   void resized() 
   {
      int pad = 5;
      int h = getHeight() / 4;
      int w = getWidth() / 2 - pad;
      int labelw = 80 - pad;
      int itemw = w-labelw;
      int itemh = h-pad;
      
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
      incrModeLabel->setBounds(l, pad, labelw, itemh);
      incrStepLabel->setBounds(l, 1*h, labelw, itemh);
      incrMinLabel->setBounds(l, 2*h, labelw, itemh);
      incrMaxLabel->setBounds(l, 3*h, labelw, itemh);
      l += labelw;
      incrDirection->setBounds(l, 0, itemw, itemh);
      incrAmount->setBounds(l, h, itemw, itemh);
      incrMin->setBounds(l, 2*h, itemw, itemh);
      incrMax->setBounds(l, 3*h, itemw, itemh);
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
         if (realMax < incrMax->getValue())
            incrMax->setValue(realMax);
         incrMin->setRange(0, realMax-1, 1);
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
};

void MidiBindingEditorContent::putBindingOptions(const MidiBinding& binding)
{
   modeCombo->setSelectedId(binding.getMode() + 1);
   triggerValue->setValue(binding.getTriggerValue());

   minSlider->setValue(binding.getMin());
   maxSlider->setValue(binding.getMax());

   double incrAmountd = binding.getIncrAmount();
   int incr = roundToInt(1.0 / fabs(incrAmountd));
   int imax = binding.getIncrMax() / fabs(incrAmountd);
   incrMax->setValue(imax);
   int imin = binding.getIncrMin() / fabs(incrAmountd);
   incrMin->setValue(imin);

   incrAmount->setSelectedId(incr, false);
      
   incrDirection->setSelectedId(binding.getStepMode());
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
}

void ParamSlider::mouseDown(const MouseEvent& e)
{
   if (managedParam && e.mods.isRightButtonDown ())
   {
      PopupMenu menu = managedParam->generateMidiPopupMenu();

      menu.addItem (3, "Edit", true);

      int result = menu.showAt (e.getScreenX(), e.getScreenY());

      bool handlerd = managedParam->processMidiPopupMenu(result);
      
      if (!handlerd && result == 3)
      {
         MidiBindingEditorContent dialogStuff;
         dialogStuff.putBindingOptions(managedParam->getBinding());
         
         DialogWindow::showModalDialog(String("Edit Note Binding"), &dialogStuff, 0, Colours::brown, true);

         dialogStuff.getBindingOptions(managedParam->getBinding());
      }
   }

   Slider::mouseDown (e);
}
