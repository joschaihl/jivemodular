/*
 ==============================================================================

 This file is part of the JUCETICE project - Copyright 2007 by Lucio Asnaghi.

 JUCETICE is based around the JUCE library - "Jules' Utility Class Extensions"
 Copyright 2007 by Julian Storer.

 ------------------------------------------------------------------------------

 JUCE and JUCETICE can be redistributed and/or modified under the terms of
 the GNU Lesser General Public License, as published by the Free Software
 Foundation; either version 2 of the License, or (at your option) any later
 version.

 JUCE and JUCETICE are distributed in the hope that they will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU Lesser General Public License
 along with JUCE and JUCETICE; if not, visit www.gnu.org/licenses or write to
 Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 Boston, MA 02111-1307 USA

 ==============================================================================
*/

#include "HostFilterComponent.h"
#include "resources/Resources.h"

#ifndef JOST_VST_PLUGIN
  #include "extras/audio plugins/wrapper/Standalone/juce_AudioFilterStreamer.h"
  #include "extras/audio plugins/wrapper/Standalone/juce_StandaloneFilterWindow.h"
#endif

#include "ui/plugins/WrappedJuceVSTWindow.h"
#include "model/plugins/WrappedJucePlugin.h"


//==============================================================================
HostFilterComponent::HostFilterComponent (HostFilterBase* const ownerFilter_)
    : AudioProcessorEditor (ownerFilter_),
      tooltipWindow (0),
      toolbar (0),
      main (0),
      navigator(0),
      browser (0),
      verticalDividerBar (0),
      horizontalDividerBar (0)
{
    DBG ("HostFilterComponent::HostFilterComponent");

    Config* config = Config::getInstance();

    // global look and feel
    lookAndFeel.readColourFromConfig ();
    LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);

    // register tooltip window
    tooltipWindow = 0;
    if (config->showTooltips)
        tooltipWindow = new TooltipWindow (0, 1000);

    // register ourselves with the plugin - it will use its ChangeBroadcaster base
    // class to tell us when something has changed.
    getFilter()->addChangeListener (this);
    // getFilter()->addListenerToParameters (this);

    // add toolbar / main tabbed component / tabbed browser / divider
    factory = new ToolbarMainItemFactory (this);
    
    addAndMakeVisible (toolbar = new Toolbar ());
    addAndMakeVisible (main = new MainTabbedComponent (this));
//    addAndMakeVisible (browser = new BrowserTabbedComponent (this));
//    addAndMakeVisible (navigator = new ViewportNavigator (0));

    addAndMakeVisible (resizer = new ResizableCornerComponent (this, &resizeLimits));
    resizeLimits.setSizeLimits (150, 150, 1280, 1024);

    // And use our item factory to add a set of default icons to it...
    toolbar->setVertical (false);
    if (config->toolbarSet != String::empty)
        toolbar->restoreFromString (*factory, config->toolbarSet);
    else
        toolbar->addDefaultItems (*factory);

    // build layout
    setBrowserVisible (config->showBrowser,
                       config->browserLeft,
                       false);

    // set its size
    int initialWidth = 800, initialHeight = 600;
    setSize (initialWidth, initialHeight);

    // get the command manager
    CommandManager* commandManager = CommandManager::getInstance();
    commandManager->registerAllCommandsForTarget (this);
    commandManager->setFirstCommandTarget (this);

//    commandManager->getKeyMappings()->resetToDefaultMappings();
//    addKeyListener (commandManager->getKeyMappings());

#ifndef JOST_VST_PLUGIN
    addKeyListener (commandManager->getKeyMappings());
#endif

   // beautiful annoying splash icon in your face
   commandManager->invokeDirectly (CommandIDs::appAbout, false);

    // register as listener to transport
    getFilter()->getTransport()->addChangeListener (this);
    
        XmlElement* const savedPluginList = ApplicationProperties::getInstance()
                                          ->getUserSettings()
                                          ->getXmlValue (T("pluginList"));

    if (savedPluginList != 0)
    {
        knownPluginList.recreateFromXml (*savedPluginList);
        delete savedPluginList;
    }

    // add internal VST plugins
    const File deadMansPedalFile (ApplicationProperties::getInstance()->getUserSettings()->getFile().getSiblingFile ("RecentlyCrashedPluginsList"));
#if JUCE_MAC
    File internalPluginFolder = File::getSpecialLocation(File::currentApplicationFile).getChildFile("./Contents/PlugIns"); // nicely hidden inside bundle on mac os x
#else
    File internalPluginFolder = File::getSpecialLocation(File::currentApplicationFile).getChildFile("../Plugins"); // plugin folder alongside app on other platforms.. for now
#endif
    VSTPluginFormat vst;
    PluginDirectoryScanner internalPluginScanner(internalPluginList, vst, FileSearchPath(internalPluginFolder.getFullPathName()), false, deadMansPedalFile);
    while (internalPluginScanner.scanNextFile(true)) {
       // keep looking
    }    

    knownPluginList.addChangeListener (this);
    pluginSortMethod = (KnownPluginList::SortMethod) ApplicationProperties::getInstance()->getUserSettings()
                            ->getIntValue (T("pluginSortMethod"), KnownPluginList::sortByManufacturer);
}

//==============================================================================
HostFilterComponent::~HostFilterComponent()
{
     DBG ("HostFilterComponent::~HostFilterComponent");

    knownPluginList.removeChangeListener (this);

    // register as listener to transport
    getFilter()->getTransport()->removeChangeListener (this);

    // deregister ouselves from the plugin (in this case the host)
    getFilter()->removeChangeListener (this);
    // getFilter()->removeListenerToParameters (this);

    // save toolbar layout
    Config::getInstance()->toolbarSet = toolbar->toString ();

    // close and free editor window
    closePluginEditorWindows ();

    // clear all childrens and objects
    deleteAndZero (navigator);
    deleteAndZero (toolbar);
    deleteAndZero (factory);
    deleteAndZero (main);
    deleteAndZero (browser);
    deleteAndZero (verticalDividerBar);
    deleteAndZero (horizontalDividerBar);
    deleteAndZero (tooltipWindow);
    deleteAndZero (resizer);

    // clear command manager commands... save keymappings before this point !
    CommandManager* commandManager = CommandManager::getInstance();
    commandManager->setFirstCommandTarget (0);
    commandManager->clearCommands ();
}

//==============================================================================
void HostFilterComponent::resized()
{
    if (! main->areComponentsCreated ())
        rebuildComponents ();

    int toolbarHeight = JOST_DEFAULT_TOOLBAR_HEIGHT;
    if (toolbar)
        toolbar->setBounds (0, 0, getWidth(), toolbarHeight);
    else
        toolbarHeight = 0;

    Config* config = Config::getInstance();

    if (config->showBrowser)
    {
        if (config->browserLeft)
        {
            Component* vcomps[] = { 0, verticalDividerBar, main };
            verticalLayout.layOutComponents (vcomps, 3,
                                             0,
                                             toolbarHeight,
                                             getWidth(),
                                             getHeight() - toolbarHeight,
                                             false,     // lay out side-by-side
                                             true);     // resize the components' heights as well as widths

            Component* hcomps[] = { browser, horizontalDividerBar, navigator };
            horizontalLayout.layOutComponents (hcomps, 3,
                                               verticalLayout.getItemCurrentPosition (0),
                                               toolbarHeight,
                                               verticalLayout.getItemCurrentPosition (1),
                                               getHeight() - toolbarHeight,
                                               true,     // lay out side-by-side
                                               true);     // resize the components' heights as well as widths
        }
        else
        {
            Component* vcomps[] = { main, verticalDividerBar, 0 };
            verticalLayout.layOutComponents (vcomps, 3,
                                             0,
                                             toolbarHeight,
                                             getWidth(),
                                             getHeight() - toolbarHeight,
                                             false,     // lay out side-by-side
                                             true);     // resize the components' heights as well as widths

            Component* hcomps[] = { browser, horizontalDividerBar, navigator };
            horizontalLayout.layOutComponents (hcomps, 3,
                                               verticalLayout.getItemCurrentPosition (2),
                                               toolbarHeight,
                                               getWidth() - verticalLayout.getItemCurrentPosition (2),
                                               getHeight() - toolbarHeight,
                                               true,     // lay out side-by-side
                                               true);     // resize the components' heights as well as widths
        }
    }
    else
    {
        main->setBounds (0, toolbarHeight, getWidth(), getHeight() - toolbarHeight);
    }
    
    resizer->setBounds (getWidth() - 16, getHeight() - 16, 16, 16);
}

//==============================================================================
void HostFilterComponent::paint (Graphics& g)
{
#ifdef JOST_VST_PLUGIN
    g.fillAll (Config::getInstance ()->getColour (T("mainBackground")));
#endif
}

//==============================================================================
void HostFilterComponent::loadPluginFromFile (const File& file)
{
    DBG ("HostFilterComponent::loadPluginFromFile");

    GraphComponent* graph = main->getGraph ();

    if (graph) graph->loadAndAppendPlugin (file, 100, 100);
}

//==============================================================================
bool HostFilterComponent::isPluginEditorWindowOpen (BasePlugin* plugin) const
{
    for (int i = pluginWindows.size(); --i >= 0;)
    {
        VstPluginWindow* window = pluginWindows.getUnchecked (i);
        if (window && window->getPlugin() == plugin)
            return window->isVisible ();
    }
    for (int i = jucePluginWindows.size(); --i >= 0;)
    {
        WrappedJuceVSTPluginWindow* window = jucePluginWindows.getUnchecked (i);
        if (window && window->getPlugin() == plugin)
            return window->isVisible ();
    }
    return false;
}

void HostFilterComponent::openPluginEditorWindow (BasePlugin* plugin)
{
   // Because the mouse event that invokes this routine is asynchronous, if user double clicks twice in quick succession, we get called again while we are bringing the editor up, and crash in the vst gui show code.
   // This boolean is will prevent entering that code while it is still in progress.
   // (it's possible that we may miss a double-click if user tries to open 2 plugins in quick succession - in which case they can double-click again..)
    static bool creatingWindowNow = false;
    
    DBG ("HostFilterComponent::openPluginEditorWindow");

    if (plugin)
    {
      if (plugin->getType() == JOST_PLUGINTYPE_WRAPPEDJUCEVST)
      {
         WrappedJuceVSTPluginWindow* jucePlugWindow = 0;
         for (int i = jucePluginWindows.size(); --i >= 0;)
         {
            WrappedJuceVSTPluginWindow* window = jucePluginWindows.getUnchecked (i);
            if (window && window->getPlugin() == plugin)
            {
                jucePlugWindow = window;
                break;
            }
         }
         if (!jucePlugWindow && !creatingWindowNow)
         {
            creatingWindowNow = true;
            jucePlugWindow = WrappedJuceVSTPluginWindow::CreateWindowFor(plugin, this); 
            jucePluginWindows.add(jucePlugWindow);
            creatingWindowNow = false;
         }
         
         if (jucePlugWindow)
         {
            if (! jucePlugWindow->isVisible ())
                jucePlugWindow->setVisible (true);
            jucePlugWindow->toFront (false);
         }
      }
      else 
      {
      // normal Jost plugin behaviour
        VstPluginWindow* pluginWindow = 0;
        for (int i = pluginWindows.size(); --i >= 0;)
        {
            VstPluginWindow* window = pluginWindows.getUnchecked (i);
            if (window && window->getPlugin() == plugin)
            {
                pluginWindow = window;
                break;
            }
        }

        if (! pluginWindow)
        {
            pluginWindow = new VstPluginWindow (this, plugin);
            pluginWindows.add (pluginWindow);
        }
        else
        {
            if (pluginWindow->getPlugin () != plugin)
                pluginWindow->setPlugin (plugin);

            if (! pluginWindow->isVisible ())
                pluginWindow->setVisible (true);
            pluginWindow->toFront (false);
        }

      }
      
        // save property with plugin
        plugin->setValue (PROP_WINDOWOPEN, 1);
    }
}

void HostFilterComponent::closePluginEditorWindow (BasePlugin* plugin)
{
    DBG ("HostFilterComponent::closePluginEditorWindow");

   if (plugin)
   {
      if (plugin->getType() == JOST_PLUGINTYPE_WRAPPEDJUCEVST)
      {
         // special behaviour for wrapped Juce plugin
         for (int i = jucePluginWindows.size(); --i >= 0;)
         {
            WrappedJuceVSTPluginWindow* window = jucePluginWindows.getUnchecked (i);
            if (window && window->getPlugin() == plugin)
            {
               // save property with plugin
               plugin->setValue (PROP_WINDOWOPEN, 0);

               jucePluginWindows.removeObject (window, true);
               break;
            }
         }

      }
      else 
      {
         // normal Jost plugin behaviour
         for (int i = pluginWindows.size(); --i >= 0;)
         {
            VstPluginWindow* window = pluginWindows.getUnchecked (i);
            if (window && window->getPlugin() == plugin)
            {
               // save property with plugin
               plugin->setValue (PROP_WINDOWOPEN, 0);

               pluginWindows.removeObject (window, true);
               break;
            }
         }
      }
   }
}

void HostFilterComponent::closePluginEditorWindows()
{
    DBG ("HostFilterComponent::closePluginEditorWindows");

    pluginWindows.clear (true);
    
  jucePluginWindows.clear(true);

}

void HostFilterComponent::resizePluginEditorWindow (BasePlugin* plugin,
                                                    const int width,
                                                    const int height)
{
    DBG ("HostFilterComponent::resizePluginEditorWindow");

    for (int i = pluginWindows.size(); --i >= 0;)
    {
        VstPluginWindow* window = pluginWindows.getUnchecked (i);
        if (window && window->getPlugin() == plugin)
        {
            window->resizeContentComponent (width, height);
            break;
        }
    }
    
    // TODO support hostcomponent resizing jucevst plugin editor windows, i think this is here so size can be saved/loaded
}

void HostFilterComponent::updatePluginEditorWindowDisplay ()
{
    for (int i = pluginWindows.size(); --i >= 0;)
        pluginWindows.getUnchecked (i)->repaint ();

    // TODO might need to tell jucevsts to repaint here?
}

//==============================================================================
void HostFilterComponent::clearComponents ()
{
    DBG ("HostFilterComponent::clearComponents");

    closePluginEditorWindows ();

    main->clearComponents ();
}

void HostFilterComponent::rebuildComponents ()
{
    DBG ("HostFilterComponent::rebuildComponents");

    // update navigator with an empty view !
    if (navigator)
        navigator->setViewedViewport (0);

    // recreate main components !
    main->rebuildComponents ();
    
    // update navigator with the main area viewport !
    if (navigator) 
    {
        navigator->setViewedViewport (main->getGraphViewport ());
        navigator->updateVisibleArea (false);
    }

    setCurrentSessionFile(Config::getInstance()->lastSessionFile);
}

//==============================================================================
void HostFilterComponent::setBrowserVisible (const bool isVisible,
                                             const bool positionLeft,
                                             const bool issueResize)
{
       DBG ("HostFilterComponent::setBrowserVisible");
    Config* config = Config::getInstance();
   if (!browser)
   {
      config->showBrowser = false;
      return;
   }

    config->showBrowser = isVisible;
    config->browserLeft = positionLeft;

    deleteAndZero (verticalDividerBar);
    deleteAndZero (horizontalDividerBar);
    verticalLayout.clearAllItems ();
    horizontalLayout.clearAllItems ();

    // show or not, checking for config
   if (browser)
      browser->setVisible (config->showBrowser);
   if (navigator)
      navigator->setVisible (config->showBrowser);

    if (config->showBrowser)
    {
        // check for vertical position
        if (config->browserLeft)
        {
            browser->setOrientation (TabbedButtonBar::TabsAtLeft);

            verticalLayout.setItemLayout (0, 150, -0.8, 350);     // browser
            verticalLayout.setItemLayout (1, 2, 2, 2);            // divider bar
            verticalLayout.setItemLayout (2, -0.2, -1.0, -0.8);   // main area
        }
        else
        {
            browser->setOrientation (TabbedButtonBar::TabsAtRight);

            verticalLayout.setItemLayout (0, -0.2, -1.0, -0.8); // main area
            verticalLayout.setItemLayout (1, 2, 2, 2);          // divider bar
            verticalLayout.setItemLayout (2, 150, -0.8, 350);   // browser
        }

        verticalDividerBar = new StretchableLayoutResizerBar (&verticalLayout, 1, true);
        addAndMakeVisible (verticalDividerBar);

        // check for horizontal position
        horizontalLayout.setItemLayout (0, -0.1, -1.0, -0.8); // browser area
        horizontalLayout.setItemLayout (1, 2, 2, 2);          // divider bar
        horizontalLayout.setItemLayout (2, 150, 150, 150);    // navigator

        horizontalDividerBar = new StretchableLayoutResizerBar (&horizontalLayout, 1, false);
        addAndMakeVisible (horizontalDividerBar);
    }

    if (issueResize)
        resized ();
}

//==============================================================================
void HostFilterComponent::changeListenerCallback (void* source)
{
    if (source == this)
    {
        closePluginEditorWindows ();
    }
    else if (source == getFilter())
    {
        clearComponents ();
        rebuildComponents ();

        // reopen windows saved with session
        Host* host = getFilter()->host;
        for (int j = 0; j < host->getPluginsCount(); j++)
        {
            BasePlugin* plugin = host->getPluginByIndex (j);
            if (plugin && plugin->getIntValue (PROP_WINDOWOPEN, 0))
                openPluginEditorWindow (plugin);
        }

        resized ();
    }
    else if (source == getFilter()->getTransport())
    {
        // update transport !
        CommandManager::getInstance()->commandStatusChanged ();
    }
    else if (source == &knownPluginList)
    {
       // save the plugin list every time it gets changed, so that if we're scanning
       // and it crashes, we've still saved the previous ones
       XmlElement* const savedPluginList = knownPluginList.createXml();

       if (savedPluginList != 0)
       {
           ApplicationProperties::getInstance()->getUserSettings()
                 ->setValue (T("pluginList"), savedPluginList);

           delete savedPluginList;

           ApplicationProperties::getInstance()->saveIfNeeded();
       }    
    }
    else
    {
        for (int i = pluginWindows.size(); --i >= 0;)
        {
            VstPluginWindow* window = pluginWindows.getUnchecked (i);
            if (window)
                window->updateParameters ();
        }
    }
}

void HostFilterComponent::parameterChanged (AudioParameter* parameter, const int index)
{
    // DBG (T("PARAMETER ") + String (index) + T(" changed"));
}


//==============================================================================
// the window main menu
const StringArray HostFilterComponent::getMenuBarNames ()
{
    const tchar* const names[] = {
                                    CommandCategories::file,
                                    CommandCategories::audio,
                                    CommandCategories::about,
                                    0
    };

    return StringArray ((const tchar**) names);
}

// where the menu is constructed
const PopupMenu HostFilterComponent::getMenuForIndex (int menuIndex,
                                                      const String& menuName)
{
    PopupMenu menu;

    Config* config = Config::getInstance();
    CommandManager* commandManager = CommandManager::getInstance();

    switch (menuIndex)
    {
    case 0: // CommandCategories::file
        {
            PopupMenu recentSessionsSubMenu;
            config->recentSessions.createPopupMenuItems (recentSessionsSubMenu,
                                                         CommandIDs::recentSessions,
                                                         false,
                                                         true);

            PopupMenu recentPluginsSubMenu;
            config->recentPlugins.createPopupMenuItems (recentPluginsSubMenu,
                                                        CommandIDs::recentPlugins,
                                                        false,
                                                        true);

            menu.addCommandItem (commandManager, CommandIDs::sessionNew);
            menu.addCommandItem (commandManager, CommandIDs::sessionLoad);
            menu.addCommandItem (commandManager, CommandIDs::sessionSaveNoPrompt);
            menu.addCommandItem (commandManager, CommandIDs::sessionSave);
            menu.addSubMenu (T("Recent sessions"), recentSessionsSubMenu);
            menu.addSeparator();
            menu.addCommandItem (commandManager, CommandIDs::pluginOpen);
            menu.addCommandItem (commandManager, CommandIDs::pluginClose);
            menu.addCommandItem (commandManager, CommandIDs::pluginClear);
            menu.addCommandItem (commandManager, CommandIDs::showPluginListEditor);
            menu.addSubMenu (T("Recent plugins"), recentPluginsSubMenu);
            menu.addSeparator();
            menu.addCommandItem (commandManager, CommandIDs::appExit);
            break;
        }

    case 1: // CommandCategories::audio
        {
#ifndef JOST_VST_PLUGIN
            menu.addCommandItem (commandManager, CommandIDs::audioOptions);
            menu.addSeparator ();
#endif
            menu.addCommandItem (commandManager, CommandIDs::audioPlayPause);
            menu.addCommandItem (commandManager, CommandIDs::audioRecord);
            menu.addCommandItem (commandManager, CommandIDs::audioStop);
            menu.addCommandItem (commandManager, CommandIDs::audioRewind);
            menu.addSeparator ();
            menu.addCommandItem (commandManager, CommandIDs::audioStemsStartStop);
            menu.addCommandItem (commandManager, CommandIDs::audioStemsSetup);
            break;
        }
    case 2: // CommandCategories::about
        {
            menu.addCommandItem (commandManager, CommandIDs::appAbout);
            break;
        }
    }

    return menu;
}

void HostFilterComponent::menuItemSelected (int menuItemID,
                                            int topLevelMenuIndex)
{
    Config* config = Config::getInstance();
    GraphComponent* graph = main->getGraph ();

    switch (topLevelMenuIndex)
    {
    case 0: // CommandCategories::file
        {
            // handle recent plugins selection
            int fileID = menuItemID - CommandIDs::recentPlugins;
            if (fileID >= 0 && fileID < config->recentPlugins.getNumFiles())
            {
                File fileToLoad = config->recentPlugins.getFile (fileID);

                if (graph)
                    graph->loadAndAppendPlugin (config->recentPlugins.getFile (fileID), 100, 100);

                break;
            }

            // handle recent session selection
            fileID = menuItemID - CommandIDs::recentSessions;
            if (fileID >= 0 && fileID < config->recentSessions.getNumFiles())
            {
                MemoryBlock fileData;
                File fileToLoad = config->recentSessions.getFile (fileID);

                if (fileToLoad.existsAsFile()
                    && fileToLoad.loadFileAsData (fileData))
                {
                    getFilter ()->setStateInformation (fileData.getData (), fileData.getSize());

                    Config::getInstance()->addRecentSession (fileToLoad);
                    Config::getInstance()->lastSessionFile = fileToLoad;
                }
            }
            break;
        }
    }

    toFront (true);
}

//==============================================================================
// The following methods implement the ApplicationCommandTarget interface, allowing
// this window to publish a set of actions it can perform, and which can be mapped
// onto menus, keypresses, etc.
ApplicationCommandTarget* HostFilterComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

// this returns the set of all commands that this target can perform..
void HostFilterComponent::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] = {
                                CommandIDs::pluginOpen,
                                CommandIDs::pluginClose,
                                CommandIDs::pluginClear,
                                CommandIDs::showPluginListEditor,
#ifndef JOST_VST_PLUGIN
                                CommandIDs::audioOptions,
#endif
                                CommandIDs::audioPlay,
                                CommandIDs::audioStop,
                                CommandIDs::audioRecord,
                                CommandIDs::audioRewind,
                                CommandIDs::audioLoop,
                                CommandIDs::audioPlayPause,
                                CommandIDs::audioStemsSetup,
                                CommandIDs::audioStemsStartStop,

                                CommandIDs::sessionNew,
                                CommandIDs::sessionLoad,
                                CommandIDs::sessionSave,
                                CommandIDs::sessionSaveNoPrompt,

                                CommandIDs::appToolbar,
                                CommandIDs::appBrowser,
                                CommandIDs::appFullScreen,
                                CommandIDs::appAbout,
                                CommandIDs::appExit
    };

    commands.addArray (ids, sizeof (ids) / sizeof (ids [0]));
}

// This method is used when something needs to find out the details about one of the commands
// that this object can perform..
void HostFilterComponent::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    const int none = 0;
    const int cmd = ModifierKeys::commandModifier;
    // const int shift = ModifierKeys::shiftModifier;

    GraphComponent* graph = main->getGraph ();
    Transport* transport = getFilter()->getTransport();

    switch (commandID)
    {
    //----------------------------------------------------------------------------------------------
    case CommandIDs::pluginOpen:
        result.setInfo (T("Open Plugin..."), T("Open a plugin"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('l'), cmd);
        result.setActive (true);
        break;
    case CommandIDs::pluginClose:
        {
        result.setInfo (T("Close Plugins"), T("Close selected plugins"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('k'), cmd);
        // TODO - have to update this !
//        GraphComponent* track = tracks.getUnchecked (0);
//        result.setActive ((track ? (track->getSelectedPlugin () != -1) : false));
        result.setActive (false);
        break;
        }
    case CommandIDs::pluginClear:
        {
        result.setInfo (T("Clear Plugins"), T("Close all plugins"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('j'), cmd);
        result.setActive ((graph ? (graph->getPluginsCount () > 2) : false));
        break;
        }
    case CommandIDs::showPluginListEditor:
        {
        result.setInfo (T("Show Plugin List"), T("Show plugin list window"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('p'), cmd);
        result.setActive (true);
        break;
        }
    //----------------------------------------------------------------------------------------------
#ifndef JOST_VST_PLUGIN
    case CommandIDs::audioOptions:
        {
        result.setInfo (T("Audio & MIDI Settings..."), T("Show device manager"), CommandCategories::audio, 0);
        // result.addDefaultKeypress (KeyPress::backspaceKey, none);
        result.setActive (true);
        break;
        }
#endif
    case CommandIDs::audioPlay:
        {
        result.setInfo (T("Play"), T("Play sequencers"), CommandCategories::audio, 0);
        if (! transport->isPlaying())
            result.addDefaultKeypress (KeyPress::spaceKey, none);
        result.setActive (! transport->isPlaying());
        break;
        }
    case CommandIDs::audioPlayPause:
        {
        if (transport->isPlaying())
            result.setInfo (T("Pause"), T("Pause sequencers"), CommandCategories::audio, 0);
        else
            result.setInfo (T("Play"), T("Play sequencers"), CommandCategories::audio, 0);
         result.addDefaultKeypress (KeyPress::spaceKey, none);
        break;
        }
    case CommandIDs::audioStop:
        {
        result.setInfo (T("Stop"), T("Stop sequencers"), CommandCategories::audio, 0);
        if (transport->isPlaying())
            result.addDefaultKeypress (KeyPress::spaceKey, none);
        result.setActive (transport->isPlaying());
        break;
        }
    case CommandIDs::audioRecord:
        {
        result.setInfo (T("Record"), T("Activate recording"), CommandCategories::audio, 0);
        result.addDefaultKeypress (T('r'), cmd);
        result.setTicked (transport->isRecording());
        result.setActive (true);
        break;
        }
    case CommandIDs::audioRewind:
        {
        result.setInfo (T("Rewind"), T("Rewind sequencers"), CommandCategories::audio, 0);
        result.addDefaultKeypress (KeyPress::backspaceKey, none);
        result.setActive (transport->getPositionInFrames() != 0);
        break;
        }
    case CommandIDs::audioLoop:
        {
        result.setInfo (T("Looping"), T("Loop sequencers"), CommandCategories::audio, 0);
        result.addDefaultKeypress (T('l'), cmd);
        result.setTicked (transport->isLooping());
        result.setActive (true);
        break;
        }
    //----------------------------------------------------------------------------------------------
    case CommandIDs::audioStemsStartStop:
        {
        int renderNumber = 0;
        if (getHost()->isStemRenderingActive(renderNumber))
         result.setInfo (T("Stop rendering stems (" + String(renderNumber) + String(")")), T("Stop rendering stems"), CommandCategories::audio, 0);
        else
         result.setInfo (T("Start rendering stems (" + String(renderNumber) + String(")")), T("Start rendering stems"), CommandCategories::audio, 0);
   
        result.setActive (true);
        break;
        }
    case CommandIDs::audioStemsSetup:
        {
        result.setInfo (T("Setup stem render..."), T("Stem Setup"), CommandCategories::audio, 0);
        result.setActive (true);
        break;
        }
    //----------------------------------------------------------------------------------------------
    case CommandIDs::sessionNew:
        {
        result.setInfo (T("New Session"), T("New session"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('n'), cmd);
        result.setActive (true);
        break;
        }
    case CommandIDs::sessionLoad:
        result.setInfo (T("Open Session..."), T("Open a session"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('a'), cmd);
        result.setActive (true);
        break;
    case CommandIDs::sessionSave:
        {
        result.setInfo (T("Save Session As..."), T("Save a session to a specified file"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('s'), ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
        result.setActive ((graph ? (graph->getPluginsCount () > 0) : false));
        break;
        }
    case CommandIDs::sessionSaveNoPrompt:
        {
        result.setInfo (T("Save Session"), T("Save session to existing file"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('s'), cmd);
        result.setActive ((graph ? (graph->getPluginsCount () > 0) : false));
        break;
        }
    //----------------------------------------------------------------------------------------------
    case CommandIDs::appToolbar:
        result.setInfo (T("Edit toolbar"), T("Edit toolbar items"), CommandCategories::about, 0);
        result.setActive (toolbar != 0);
        break;
    case CommandIDs::appBrowser:
        result.setInfo (T("Show/Hide browser"), T("Show or hide the file browser"), CommandCategories::about, 0);
        result.setActive (true);
        break;
    case CommandIDs::appFullScreen:
        result.setInfo (T("Full Screen"), T("Set main window full screen"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('t'), cmd);
        result.setActive (true);
        break;
    case CommandIDs::appExit:
        result.setInfo (T("Quit"), T("Quit Jive"), CommandCategories::file, 0);
        result.addDefaultKeypress (T('q'), cmd);
        result.setActive (true);
        break;
    case CommandIDs::appAbout:
        result.setInfo (T("About..."), T("About Jive"), CommandCategories::about, 0);
        result.setActive (true);
        break;
    //----------------------------------------------------------------------------------------------
    default:
        break;
    }
}

void HostFilterComponent::setCurrentSessionFile(const File& newFile)
{
   currentSessionFile = newFile;
   StandaloneFilterWindow* window = findParentComponentOfClass ((StandaloneFilterWindow*) 0);
   if (window)
   {
     String jostMainWindowName;
     String filename = currentSessionFile.getFileNameWithoutExtension();
     jostMainWindowName << JucePlugin_Name;
     if (!filename.isEmpty())
        jostMainWindowName << " - " << filename;
     window->setName(jostMainWindowName);
   }
}

void HostFilterComponent::handleSaveCommand(bool saveToExistingFileAndDontPrompt)
{
   File tmp = currentSessionFile;
   
   bool userConfirmed = true;
   if (!saveToExistingFileAndDontPrompt || !tmp.exists())
   {
      FileChooser myChooser (T("Save Session..."),
                              currentSessionFile.exists() ? tmp : Config::getInstance ()->lastSessionDirectory,
                              JOST_SESSION_WILDCARD,
                              JOST_USE_NATIVE_FILE_CHOOSER);

      if (myChooser.browseForFileToSave (true))
         tmp = myChooser.getResult().withFileExtension (JOST_SESSION_EXTENSION);
      else
         userConfirmed = false;      
   }
   
   if (userConfirmed && (tmp != File::nonexistent))
   {
      MemoryBlock fileData;
      getFilter ()->getStateInformation (fileData);

      if (tmp.replaceWithData (fileData.getData (), fileData.getSize()))
      {
         Config::getInstance()->addRecentSession (tmp);
         setCurrentSessionFile(tmp);
         Config::getInstance()->lastSessionFile = tmp;
      }
   }
}

bool HostFilterComponent::perform (const InvocationInfo& info)
{
    Config* config = Config::getInstance();

    GraphComponent* graph = main->getGraph ();
    Transport* transport = getFilter()->getTransport();

    switch (info.commandID)
    {
    //----------------------------------------------------------------------------------------------
    case CommandIDs::pluginOpen:
        {
            graph->loadAndAppendPlugin ();
            break;
        }
    case CommandIDs::pluginClose:
        {
            graph->closeSelectedPlugins ();
            break;
        }
    case CommandIDs::pluginClear:
        {
            graph->closeAllPlugins ();
            break;
        }
    case CommandIDs::showPluginListEditor:
        {
           if (PluginListWindow::currentPluginListWindow == 0)
               PluginListWindow::currentPluginListWindow = new PluginListWindow (knownPluginList);

           PluginListWindow::currentPluginListWindow->toFront (true);
            
            break;
        }

    //----------------------------------------------------------------------------------------------
#ifndef JOST_VST_PLUGIN
    case CommandIDs::audioOptions:
        {
            StandaloneFilterWindow* window = findParentComponentOfClass ((StandaloneFilterWindow*) 0);
            if (window)
                window->showAudioSettingsDialog ();

            break;
        }
#endif

    case CommandIDs::audioPlay:
        {
            transport->play ();
            break;
        }
    case CommandIDs::audioPlayPause:
        {
            transport->togglePlay ();
            break;
        }
    case CommandIDs::audioStop:
        {
            transport->stop ();
            break;
        }
    case CommandIDs::audioRecord:
        {
            transport->record ();
            break;
        }
    case CommandIDs::audioRewind:
        {
            transport->rewind ();
            break;
        }
    case CommandIDs::audioLoop:
        {
            transport->setLooping (! transport->isLooping());
            break;
        }

    //----------------------------------------------------------------------------------------------
    case CommandIDs::sessionNew:
        {
           bool retValue = 
               AlertWindow::showYesNoCancelBox (AlertWindow::WarningIcon,
                                             T("Unsaved Changes"),
                                             T("Are you sure you want to close the current session? You may lose any unsaved changes."));
            if (retValue)
            {
               closePluginEditorWindows ();
               getFilter()->getHost ()->closeAllPlugins (true);

               clearComponents ();
               Config::getInstance()->lastSessionFile = File::nonexistent;
               rebuildComponents ();
            }
            break;
        }
    
    case CommandIDs::sessionLoad:
        {
            FileChooser myChooser (T("Load a session file..."),
                                    Config::getInstance ()->lastSessionDirectory,
                                    JOST_SESSION_WILDCARD, JOST_USE_NATIVE_FILE_CHOOSER);

            if (myChooser.browseForFileToOpen())
            {
              bool retValue = 
               AlertWindow::showYesNoCancelBox (AlertWindow::WarningIcon,
                                             T("Unsaved Changes"),
                                             T("Are you sure you want to close the current session? You may lose any unsaved changes."));
               if (retValue)
               {

                MemoryBlock fileData;
                File fileToLoad = myChooser.getResult();

                if (fileToLoad.existsAsFile()
                    && fileToLoad.loadFileAsData (fileData))
                {
                    getFilter ()->setStateInformation (fileData.getData (), fileData.getSize());

                    Config::getInstance()->addRecentSession (fileToLoad);
                    Config::getInstance()->lastSessionFile = fileToLoad;
                }
               }
            }
            break;
        }
    case CommandIDs::sessionSave:
        {
            handleSaveCommand();
            break;
        }
    case CommandIDs::sessionSaveNoPrompt:
        {
            handleSaveCommand(true);
            break;
        }

    case CommandIDs::audioStemsStartStop:
        {
            getHost()->toggleStemRendering();
            break;
        }
    case CommandIDs::audioStemsSetup:
        {
            FileChooser myChooser (T("Save Rendered Stem Files To..."),
                                    Config::getInstance ()->lastStemsDirectory);
            if (myChooser.browseForDirectory ())
            {
                Config::getInstance ()->lastStemsDirectory = myChooser.getResult();
            }
            break; 
        }

    //----------------------------------------------------------------------------------------------
    case CommandIDs::appToolbar:
        {
            toolbar->showCustomisationDialog (*factory,
                                              Toolbar::allCustomisationOptionsEnabled);
                                              // (Toolbar::allowIconsOnlyChoice | Toolbar::showResetToDefaultsButton));
            break;
        }
    case CommandIDs::appBrowser:
        {
            setBrowserVisible (! config->showBrowser, config->browserLeft);
            break;
        }
    case CommandIDs::appFullScreen:
        {
            DocumentWindow* window = findParentComponentOfClass <DocumentWindow> ();
            if (window) {
                window->setFullScreen (! window->isFullScreen ());
                window->setMenuBar (window->isFullScreen () ? 0 : this);
            }
            break;
        }
    case CommandIDs::appExit:
        {
            deleteAndZero(PluginListWindow::currentPluginListWindow);
            JUCEApplication::getInstance()->systemRequestedQuit();
            break;
        }
    case CommandIDs::appAbout:
        {
//            Image* splashImage = ImageCache::getFromMemory (Resource::jost_about,
//                                                            Resource::jost_about_size);
         // todo: move appResourcesFolder() to somewhere everyone can use it
#if JUCE_MAC
			File appResourcesFolder(File::getSpecialLocation(File::currentApplicationFile).getChildFile("./Contents/Resources"));
#else
			File appResourcesFolder(File::getSpecialLocation(File::currentApplicationFile).getParentDirectory());
#endif
			File splashImageFile(appResourcesFolder.getChildFile("JiveAbout.png"));
            Image* splashImage = ImageFileFormat::loadFrom(splashImageFile);
            SplashScreen* splash = new SplashScreen();
            splash->show (T(JucePlugin_Name), splashImage, 3500, false);
            break;
        }

    //----------------------------------------------------------------------------------------------
    default:
        return false;
    }

    return true;
}

