
#include "JiveClipListComponent.h"
//#include "MidiBindingsEditor.h"

#define DEFAULT_MAXIMUM_CLIPS 16
#define EDIT_LIST_ID 1000

class ClipListEditor : public Component, public TableListBoxModel, public ButtonListener, public DragAndDropContainer, public DragAndDropTarget
{
public:
   ClipListEditor(StringArray cliplist_);
   ~ClipListEditor();

   std::vector<int> getChangeInfo() {return newClipOrder;};
   StringArray getClipList();

   void resized();

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
   virtual const String getDragSourceDescription (const SparseSet<int>& currentlySelectedRows) { return String("ClipItemRow"); };
   
   //DragAndDropTarget implementation
   bool isInterestedInDragSource (const String& sourceDescription,
                                           Component* sourceComponent) {return true;}; // should hone that a bit
    virtual void itemDragEnter (const String& sourceDescription,
                                Component* sourceComponent,
                                int x,
                                int y);
    virtual void itemDragMove (const String& sourceDescription,
                               Component* sourceComponent,
                               int x,
                               int y);
    virtual void itemDragExit (const String& sourceDescription,
                               Component* sourceComponent);
    virtual void itemDropped (const String& sourceDescription,
                              Component* sourceComponent,
                              int x,
                              int y);
                                                                  
   // ButtonListener implementation
   virtual void buttonClicked (Button* button);
   
   void calcDragRow(int x, int y);
   
private:
   enum ColumnIds {
      Clip,
   };

   Button* removeButton;
   TableListBox* clipsListTable;

   StringArray clipList;
   std::vector<int> newClipOrder;
   bool isDragging;
   int rowDragIsAfter; // -1 for start, 0 for after first row, etc
};

ClipListEditor::ClipListEditor(StringArray clipList_)
:
   clipsListTable(0),
   clipList(clipList_),
   isDragging(false),
   rowDragIsAfter(-1)
{
   addAndMakeVisible(clipsListTable = new TableListBox("ClipsList", this));
   TableHeaderComponent* header = clipsListTable->getHeader();
   header->addColumn("Clip", Clip, 100, 20, -1);

   addAndMakeVisible(removeButton = new TextButton("Delete"));
   removeButton->addButtonListener(this);
   
   for (int i=0; i<clipList.size(); i++)
      newClipOrder.push_back(i);

   setSize(400, 350);
}

ClipListEditor::~ClipListEditor()
{
   deleteAllChildren();
}

StringArray ClipListEditor::getClipList()
{
   StringArray newClipsInOrder;
   for (int i=0; i<newClipOrder.size(); i++)
      if (newClipOrder[i] >= 0 && newClipOrder[i] < clipList.size())
         newClipsInOrder.insert(i, clipList[newClipOrder[i]]);
   return newClipsInOrder;
}

void ClipListEditor::resized()
{
   int buttonH = 40;
   int width = getWidth()-10;
   int height = getHeight();
   int lh = height - buttonH;

   if (clipsListTable)
   {
      clipsListTable->setBounds(5, 5, width, lh - 5);
      clipsListTable->getHeader()->setColumnWidth(Clip, width);
   }
   if (removeButton)
   {
      int addw = width / 3;
      removeButton->setBounds(addw*2, lh, addw-5, buttonH - 5);
   }
}


int ClipListEditor::getNumRows()
{
   return newClipOrder.size();
}

void ClipListEditor::paintRowBackground (Graphics& g,
                               int rowNumber,
                               int width, int height,
                               bool rowIsSelected)
{
   if (rowIsSelected)
      g.setColour(Colours::yellow);
   else
      g.setColour(Colours::orange);
   g.fillRect(0, 0, width, height);
   
   if (isDragging)
   {
      g.setColour(Colours::blue);
      if (rowDragIsAfter == rowNumber)
         g.fillRect(0, height-3, width, 3);
      if (rowDragIsAfter == rowNumber-1)
         g.fillRect(0, 0, width, 3);
   }
}

void ClipListEditor::calcDragRow(int x, int y)
{
   int row = 0;
   rowDragIsAfter = getNumRows() - 1;

   Rectangle pos = clipsListTable->getCellPosition(Clip, row, true);
   if (y <= (pos.getY() + pos.getHeight() / 2))
      rowDragIsAfter = -1;
   else 
   {
      for (int row=1; row<getNumRows(); row++)
      {
         pos = clipsListTable->getCellPosition(Clip, row, true);
         if (y < (pos.getY() + pos.getHeight() / 2))
         {
            rowDragIsAfter = row-1;
            break;
         }
      }
   }
}

void ClipListEditor::itemDragEnter (const String& sourceDescription,
                          Component* sourceComponent,
                          int x,
                          int y)
{
   isDragging = true;
   calcDragRow(x, y);
    repaint();
}
void ClipListEditor::itemDragMove (const String& sourceDescription,
                         Component* sourceComponent,
                         int x,
                         int y)
{
   calcDragRow(x, y);
    repaint();
}
void ClipListEditor::itemDragExit (const String& sourceDescription,
                         Component* sourceComponent)
{
   isDragging = false;
}
void ClipListEditor::itemDropped (const String& sourceDescription,
                              Component* sourceComponent,
                              int x,
                              int y)
                              
{
   isDragging = false;
   calcDragRow(x, y);
   // get selected row
   int movedRow = clipsListTable->getSelectedRow(); // only one thing at a time for now..
   // insert it into order after rowDragIsAfter
   int movedClipId = newClipOrder[movedRow];
   std::vector<int> newNewClipOrder;
   if (rowDragIsAfter < 0)
      newNewClipOrder.push_back(movedClipId);
   for (int i=0; i<newClipOrder.size(); i++)
   {
      if (movedRow != i)
         newNewClipOrder.push_back(newClipOrder[i]);
      if (rowDragIsAfter == i)
         newNewClipOrder.push_back(movedClipId);
   }
   newClipOrder = newNewClipOrder;
   // redraw table
   repaint();
}

void ClipListEditor::paintCell (Graphics& g,
                            int rowNumber,
                            int columnId,
                            int width, int height,
                            bool rowIsSelected)
{
   String string;
   if (columnId == Clip && rowNumber >= 0 && rowNumber < newClipOrder.size())
   {
      if (newClipOrder[rowNumber] >= 0 && newClipOrder[rowNumber] < clipList.size())
         string = File(clipList[newClipOrder[rowNumber]]).getFileNameWithoutExtension();
   }
   g.setColour(Colours::black);
   g.drawText(string, 0, 0, width, height, Justification(Justification::horizontallyCentred | Justification::verticallyCentred), true);
}

void ClipListEditor::selectedRowsChanged (int lastRowSelected)
{

}
   
void ClipListEditor::buttonClicked (Button* button)
{
   if (button == removeButton)
   {
      int movedRow = clipsListTable->getSelectedRow(); // only one thing at a time for now..
      newClipOrder.erase(newClipOrder.begin()+movedRow);
      clipsListTable->updateContent();
   }
}

ClipListComponent::ClipListComponent(String componentName, String _fileBrowserWildcard)
: 
   Component(componentName),
   maxClips(DEFAULT_MAXIMUM_CLIPS),
   currentClip(-1),
   fileBrowserWildcard(_fileBrowserWildcard),
   clipsCombo(0),
   isFileDragOver(false)
{
   addAndMakeVisible(clipsCombo = new ComboBox("clipFilesList"));
   clipsCombo->addListener(this);
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

      clipsCombo->addItem("Edit list...", EDIT_LIST_ID);

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
      for (int i=0; i<clipFiles.size(); i++)
      {
         File clipFile(clipFiles[i]);
         clipsCombo->addItem(String(String(i) + String(" ") + clipFile.getFileNameWithoutExtension()), i+1); // dummy combo id of clip id + 1
      }
      clipsCombo->addItem("Edit list...", EDIT_LIST_ID);
   }

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

   if (cid == EDIT_LIST_ID)
   {
        ClipListEditor ed(clipFiles);
//         if (DialogWindow::showModalDialog(String("Edit Clip List"), &ed, 0, Colours::green, true))
         DialogWindow::showModalDialog(String("Edit Clip List"), &ed, 0, Colours::green, true, true, true);
         sendClipListChangedMessage(ed.getClipList(), ed.getChangeInfo());

      setCurrentClipIndex(0, true); // just pick first for now; in future, pick the one that was picked, or pick the first if the prev was deleted...?
   }
   if (cid > 0 && cid < DEFAULT_MAXIMUM_CLIPS)
   {
      // convert it to a clip index
      // set the current clip index & notify listeners
      setCurrentClipIndex(cid - 1, true);
   }
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

void ClipListComponent::sendClipListChangedMessage(StringArray newClipList, std::vector<int> changeInfo)
{
    ComponentDeletionWatcher deletionWatcher (this);

    jassert (! deletionWatcher.hasBeenDeleted());

    for (int i = listeners.size(); --i >= 0;)
    {
        ((ClipListListener*) listeners.getUnchecked (i))->clipListChanged(this, newClipList, changeInfo);

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

