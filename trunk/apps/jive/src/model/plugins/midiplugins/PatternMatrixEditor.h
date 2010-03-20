

#ifndef JOST_PATTERNMATRIXEDITOR_H
#define JOST_PATTERNMATRIXEDITOR_H

#include "PluginEditorComponent.h"
#include "ui/utility/JiveParamSlider.h"

class PatternMatrixPlugin;

class PatternMatrixEditor  : public PluginEditorComponent,
                          public ChangeListener,
                          public SliderListener,
                        public ComboBoxListener
{
public:
   //==============================================================================
   PatternMatrixEditor (PatternMatrixPlugin* owner_);
   ~PatternMatrixEditor();

   int getPreferredWidth ()                        { return 420; }
   int getPreferredHeight ()                       { return 400; }
   bool isResizable ()                             { return true; }

   void paint (Graphics& g);
   void resized();

   void changeListenerCallback (void* source);
   void sliderValueChanged (Slider* sliderThatWasMoved);
   void comboBoxChanged (ComboBox* comboBoxThatHasChanged);

   void updateParameters();

    //==============================================================================
    juce_UseDebuggingNewOperator

private:
   double comboIdToQuantize(int i);
   int quantizeToComboId(double q);

   PatternMatrixPlugin* owner;

   ComboBox* quantizeBox;
   ParamSlider* partStatus[MAXPARTS];
   Slider* partCC[MAXPARTS];

private:
    //==============================================================================
    // (prevent copy constructor and operator= being generated..)
    PatternMatrixEditor (const PatternMatrixEditor&);
    const PatternMatrixEditor& operator= (const PatternMatrixEditor&);
};


#endif   // JOST_PATTERNMATRIXEDITOR_H
