
#ifndef JIVE_MIDIBINDINGSEDITOR_H
#define JIVE_MIDIBINDINGSEDITOR_H

#include "Config.h"
#include "model/BasePlugin.h"

class MidiBindingEditorContent;

// Component for editing a list of bindings for a parameter - 
// lists all the bindings, showing the selected binding settings 
// in a MidiBindingEditorContent at the bottom.
// Can also support add/remove individual bindings.
class MidiBindingsEditor : public Component, public TableListBoxModel, public ButtonListener
{
public:
   MidiBindingsEditor(MidiAutomatable* parameter_);
   ~MidiBindingsEditor();

   void resized();

   void showCurBindingOpts();
   void saveCurBindingOpts();

   //- TableListBoxModel implementation
   virtual int getNumRows();
   virtual void paintRowBackground (Graphics& g,
                               int rowNumber,
                               int width, int height,
                               bool rowIsSelected);
   virtual void paintCell (Graphics& g,
                            int rowNumber,
                            int columnId,
                            int width, int height,
                            bool rowIsSelected);
   virtual void selectedRowsChanged (int lastRowSelected);
   
   // ButtonListener implementation
   virtual void buttonClicked (Button* button);
 
private:
   enum ColumnIds {
      Mode,
      TriggerVal
   };

   Button* addButton;
   Button* removeButton;
   TableListBox* bindingsList;
   MidiBindingEditorContent* bindingEditor;

   MidiAutomatable* parameter;
   int currentBinding;
};



#endif   // JIVE_MIDIBINDINGSEDITOR_H
