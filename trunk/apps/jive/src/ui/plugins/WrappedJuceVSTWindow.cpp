/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-9 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

#include "WrappedJuceVSTWindow.h"
#include "WrappedJucePlugin.h"
#include "HostFilterComponent.h"


class JuceVSTContentComponent : public Component
{
public:
   JuceVSTContentComponent(Component* const genericUi)// : Component(TabbedButtonBar::TabsAtTop) 
   :  
      genericUI(genericUi)
   {
      addAndMakeVisible(genericUi);
         
      resized();
   };
   
   ~JuceVSTContentComponent()
   {
      deleteAllChildren();
   }
   
   void resized()
   {
      int h = getHeight();
      int w = getWidth();
      if (genericUI)
         genericUI->setBounds(0, 0, w, h-16);
   }
   
   Component* genericUI;
   AudioPluginInstance* pluginInstance;
};

//==============================================================================
WrappedJuceVSTPluginWindow::WrappedJuceVSTPluginWindow (BasePlugin* _plugin, //Component* const uiComp, Component* const genericUi,
                            HostFilterComponent* _owner)
: 
   DocumentWindow ("", Colours::lightblue, DocumentWindow::minimiseButton | DocumentWindow::closeButton),
   plugin(_plugin),
   owner (_owner)
{
    DBG ("WrappedJuceVSTPluginWindow::WrappedJuceVSTPluginWindow");

   String windowTitle(plugin->getInstanceName() + String(" - ") + plugin->getName());

   WrappedJucePlugin* vstPlug = dynamic_cast<WrappedJucePlugin*>(plugin);
   if (vstPlug)
   {
      pluginInstance = vstPlug->getAudioPluginInstance();

      AudioProcessorEditor* ui = pluginInstance->createEditorIfNeeded();

      if(plugin->getBoolValue(PROP_WINDOWPREFERGENERIC, false) || !ui)
      {
         setContentComponent(new JuceVSTContentComponent(new GenericJuceVSTEditor (plugin, pluginInstance)), true);
         windowTitle += String(" Bindings");
      }
      else
         setContentComponent(ui, true, true);
   }
   setName(windowTitle);
   setResizable(true, true);

   setTopLeftPosition (plugin->getIntValue(PROP_WINDOWXPOS, 50), plugin->getIntValue(PROP_WINDOWYPOS, 50));

   int w = std::max(plugin->getIntValue(PROP_WINDOWWSIZE, 400), 400);
   int h = std::max(plugin->getIntValue(PROP_WINDOWHSIZE, 240), 240);
   setSize(w, h);

   setVisible (true);
}

WrappedJuceVSTPluginWindow* WrappedJuceVSTPluginWindow::CreateWindowFor (BasePlugin* plugin, HostFilterComponent* _owner,
                                          bool useGenericView)
{
//    for (int i = activePluginWindows.size(); --i >= 0;)
//        if (activePluginWindows.getUnchecked(i)->owner == node
//             && activePluginWindows.getUnchecked(i)->isGeneric == useGenericView)
//            return activePluginWindows.getUnchecked(i);

//   AudioProcessorEditor* ui = 0;
//   AudioProcessorEditor* genericui = 0;
   
//useGenericView = true;// see if problems are to do with replacing content component
//   WrappedJucePlugin* vstPlug = dynamic_cast<WrappedJucePlugin*>(plugin);
//   if (vstPlug)
//   {
//      AudioPluginInstance* pnstance = vstPlug->getAudioPluginInstance();
//
//     ui = MakeContentComponentFor(plugin, pnstance, false);
//     genericui = MakeContentComponentFor(plugin, pnstance, true);
   
//      if (ui != 0)
//      {
      //      AudioPluginInstance* const plugin = pluginInstance;//dynamic_cast <AudioPluginInstance*> (node->processor);

//      if (plugin != 0)
//         ui->setName (plugin->getName());
         
      return new WrappedJuceVSTPluginWindow (plugin, /*ui, genericui, */_owner);
//      }
//   }
//
//   return 0;
}
//
//AudioProcessorEditor* WrappedJuceVSTPluginWindow::MakeContentComponentFor(BasePlugin* plugin, AudioPluginInstance* pnstance, bool useGenericView)
//{
//   AudioProcessorEditor* ui = 0;
//
//      if (! useGenericView)
//      {
//         ui = pnstance->createEditorIfNeeded();
//
//         if (ui == 0)
//            useGenericView = true;
//      }
//
//      if (useGenericView)
//         ui = new GenericJuceVSTEditor (plugin, pnstance);
//
//   return ui;
//}

WrappedJuceVSTPluginWindow::~WrappedJuceVSTPluginWindow()
{
   
    plugin->setValue (PROP_WINDOWWSIZE, getWidth());
    plugin->setValue (PROP_WINDOWHSIZE, getHeight());

    DBG ("WrappedJuceVSTPluginWindow::~WrappedJuceVSTPluginWindow");

    setContentComponent (0);
}

void WrappedJuceVSTPluginWindow::moved()
{
   DocumentWindow::moved();

    plugin->setValue (PROP_WINDOWXPOS, getX());
    plugin->setValue (PROP_WINDOWYPOS, getY());
}

void WrappedJuceVSTPluginWindow::resized()
{
   DocumentWindow::resized();
}

void WrappedJuceVSTPluginWindow::closeButtonPressed()
{
    owner->closePluginEditorWindow(plugin);
}
