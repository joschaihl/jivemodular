
#include "JiveClipListComponent.h"
//#include "MidiBindingsEditor.h"

#define DEFAULT_MAXIMUM_CLIPS 16

//ClipListComponent::ClipListComponent(String componentName, String _fileBrowserWildcard, AudioProcessor* ownerPlugin_, AudioParameter* managedParam_, const int paramIndex_)
ClipListComponent::ClipListComponent(String componentName, String _fileBrowserWildcard)
: 
   Component(componentName),
   maxClips(DEFAULT_MAXIMUM_CLIPS),
   currentClip(-1),
   fileBrowserWildcard(_fileBrowserWildcard),
   clipsCombo(0),
   isFileDragOver(false)//,
//   ownerPlugin(ownerPlugin_),
//   managedParam(managedParam_),
//   paramIndex(paramIndex_)
{
   addAndMakeVisible(clipsCombo = new ComboBox("clipFilesList"));
   clipsCombo->addListener(this);

//   if (managedParam)
//      managedParam->addListener(this);

   setMaxClips(maxClips);
}

ClipListComponent::~ClipListComponent()
{
   clipsCombo->removeListener(this);
    deleteAllChildren();
}

int ClipListComponent::getMaxClips() const
{
   return maxClips;
}

void ClipListComponent::setMaxClips(int _maxClips)
{
   maxClips = _maxClips;

   for (int i=clipFiles.size()-1; i>=maxClips; i--)
      clipFiles.remove(i);
}

int ClipListComponent::getNumClips() const
{
   return clipFiles.size();
}

void ClipListComponent::addClipFile(String filename)
{
   clipsCombo->removeListener(this);

   // add the new string
   clipFiles.insert(0, filename);

   // trim any off the end via setMaxClips
   setMaxClips(maxClips);

   // update the combobox list
   // TODO: move this to a method so can be refreshed whenever we feel like it
   if (clipsCombo)
   {
      clipsCombo->clear();
//      for (int i=clipFiles.size()-1; i>=0; i--)
      for (int i=0; i<clipFiles.size(); i++)
      {
         File clipFile(clipFiles[i]);
         clipsCombo->addItem(String(String(i) + String(" ") + clipFile.getFileNameWithoutExtension()), i+1); // dummy combo id of clip id + 1
      }
      // TODO: set current clip to 1 and THEN update combo

      clipsCombo->setSelectedId(1);
   }
   
   // TODO: parameter to make this optional so dragged list notifies only once at end
   sendCurrentClipChangedMessage();
   clipsCombo->addListener(this);
}

void ClipListComponent::setClipFile(int clipIndex, String filename)
{
   clipsCombo->removeListener(this);

   // update the new string
   clipFiles.set(clipIndex, filename);

   // trim any off the end via setMaxClips
   setMaxClips(maxClips);

   // update the combobox list
   // TODO: move this to a method so can be refreshed whenever we feel like it
   if (clipsCombo)
   {
      clipsCombo->clear(true); // DON't NOTIFY
//      for (int i=clipFiles.size()-1; i>=0; i--)
      for (int i=0; i<clipFiles.size(); i++)
      {
         File clipFile(clipFiles[i]);
         clipsCombo->addItem(String(String(i) + String(" ") + clipFile.getFileNameWithoutExtension()), i+1); // dummy combo id of clip id + 1
      }
      // TODO: set current clip to 1 and THEN update combo

//      clipsCombo->setSelectedId(1);
   }
   
   // TODO: parameter to make this optional so dragged list notifies only once at end
//   sendCurrentClipChangedMessage();
   clipsCombo->addListener(this);
}

String ClipListComponent::getCurrentClipFile() const
{
   String currentFile = String::empty;
   
   if (currentClip >= 0 && currentClip < clipFiles.size())
      currentFile = clipFiles[currentClip];
      
   return currentFile;
}

int ClipListComponent::getCurrentClipIndex()
{
   return currentClip;
}

void ClipListComponent::setCurrentClipIndex(int index, bool notifyObservers)
{
   clipsCombo->removeListener(this);
   if (index >= 0 && index < clipFiles.size())
   {
      currentClip = index;
    
      if (clipsCombo && (clipsCombo->getSelectedId() != currentClip + 1))
         clipsCombo->setSelectedId(currentClip + 1, true);
   }
   if (notifyObservers)
      sendCurrentClipChangedMessage();   
   clipsCombo->addListener(this);
}

void ClipListComponent::parameterChanged(AudioParameter* newParameter, const int index)
{
//   setValue(newParameter->getValueMapped (), false);
}

void ClipListComponent::resized()
{
   // all combo for now!
   // (in future will have "edit list" buttons, etc)
   int w = getWidth();
   int h = getHeight();
   clipsCombo->setBounds(0, 0, w, h);
}

void ClipListComponent::paintOverChildren (Graphics& g)
{
    if (isFileDragOver)
    {
        g.setColour (Colours::red.withAlpha (0.2f));
        g.drawRect (0, 0, getWidth(), getHeight(), 3);
    }
}

//void ClipListComponent::mouseDown(const MouseEvent& e)
//{
//   // TODO: share this code so my param-bindable controls use same code for popup menu etc
//   if (managedParam && e.mods.isRightButtonDown())
//   {
//      PopupMenu menu = managedParam->generateMidiPopupMenu();
//
//      menu.addItem (3, "Edit Bindings...", true);
//
//      int result = menu.showAt (e.getScreenX(), e.getScreenY());
//
//      bool handlerd = managedParam->processMidiPopupMenu(result);
//      
//      if (!handlerd && result == 3)
//      {
//         MidiBindingsEditor ed(managedParam);
//         DialogWindow::showModalDialog(String("Edit Parameter Bindings"), &ed, 0, Colours::green, true);
//      }
//   }
//   else
//      Component::mouseDown (e);
//}

// FileDragAndDropTarget
bool ClipListComponent::isInterestedInFileDrag (const StringArray& files)
{
   bool something = false;
   WildcardFileFilter wildcard(fileBrowserWildcard, String::empty, String::empty);
   for (int i=0; i<files.size() && !something; i++)
   {
      something = wildcard.isFileSuitable(File(files[i]));
   }
   return something;
}

void ClipListComponent::filesDropped (const StringArray& files, int, int)
{
   isFileDragOver = false;
   repaint();

// we don't manage the slots - the plugin does
// so we notify our client (plugin GUI) and it can get the plugin to process the new files
// (the code for enforcing max, shunting existing down the list will of course look the same as (it was..) here!)
StringArray goodClipFiles;
   WildcardFileFilter wildcard(fileBrowserWildcard, String::empty, String::empty);
   for (int i=0; i<files.size(); i++)
   {
      if (wildcard.isFileSuitable(File(files[i])))
         goodClipFiles.add(files[i]);
//         addClipFile(files[i]);
   }
   if (goodClipFiles.size() > 0)
      sendFilesDroppedMessage(goodClipFiles);
}

void ClipListComponent::fileDragEnter (const StringArray& files, int, int)
{
// TODO: check extensions and we're only interested in fileBrowserWildcard
   isFileDragOver = isInterestedInFileDrag(files);
   if (isFileDragOver)
      repaint();
}

void ClipListComponent::fileDragExit (const StringArray& files)
{
   isFileDragOver = false;
   repaint();
}

void ClipListComponent::comboBoxChanged(ComboBox* comboBoxThatHasChanged)
{
   // get the selected item id 
   int cid = clipsCombo->getSelectedId();
   // convert it to a clip index
   // set the current clip index & notify listeners
   setCurrentClipIndex(cid - 1, true);
}


void ClipListComponent::addListener (ClipListListener* const listener) throw()
{
    jassert (listener != 0);
    if (listener != 0)
        listeners.add (listener);
}

void ClipListComponent::removeListener (ClipListListener* const listener) throw()
{
    listeners.removeValue (listener);
}

void ClipListComponent::sendClipListChangedMessage()
{
    ComponentDeletionWatcher deletionWatcher (this);

    jassert (! deletionWatcher.hasBeenDeleted());

    for (int i = listeners.size(); --i >= 0;)
    {
        ((ClipListListener*) listeners.getUnchecked (i))->clipListChanged(this);

        if (deletionWatcher.hasBeenDeleted())
            return;

        i = jmin (i, listeners.size() - 1);
    }
}

void ClipListComponent::sendCurrentClipChangedMessage()
{
    ComponentDeletionWatcher deletionWatcher (this);

    jassert (! deletionWatcher.hasBeenDeleted());

    for (int i = listeners.size(); --i >= 0;)
    {
        ((ClipListListener*) listeners.getUnchecked (i))->currentClipChanged(this);

        if (deletionWatcher.hasBeenDeleted())
            return;

        i = jmin (i, listeners.size() - 1);
    }
}

void ClipListComponent::sendFilesDroppedMessage(const StringArray& files)
{
    ComponentDeletionWatcher deletionWatcher (this);

    jassert (! deletionWatcher.hasBeenDeleted());

    for (int i = listeners.size(); --i >= 0;)
    {
        ((ClipListListener*) listeners.getUnchecked (i))->clipFilesDropped(this, files);

        if (deletionWatcher.hasBeenDeleted())
            return;

        i = jmin (i, listeners.size() - 1);
    }
}

