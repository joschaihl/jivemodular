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

#ifndef __JUCE_WRAPPEDJUCEVSTWINDOW_H
#define __JUCE_WRAPPEDJUCEVSTWINDOW_H

#include "model/BasePlugin.h"
#include "GenericJuceVSTEditor.h"

//==============================================================================
/** A desktop window containing a plugin's UI.
*/
class WrappedJuceVSTPluginWindow  : public DocumentWindow
{
    WrappedJuceVSTPluginWindow (BasePlugin* plugi, //Component* const uiComp, Component* const genericUi,
                  HostFilterComponent* _owner);

public:
    static WrappedJuceVSTPluginWindow* CreateWindowFor (BasePlugin* plugin, HostFilterComponent* owner, bool useGenericView = false);
   //static AudioProcessorEditor* MakeContentComponentFor(BasePlugin* plugin, AudioPluginInstance* pluginInstance, bool useGenericView);

    ~WrappedJuceVSTPluginWindow();

    void moved();
    void resized();
    void closeButtonPressed();
   BasePlugin* getPlugin() { return plugin; };
   
private:
   BasePlugin* plugin; 
   AudioPluginInstance* pluginInstance;
    HostFilterComponent* owner;
};

#endif // __JUCE_WRAPPEDJUCEVSTWINDOW_H
