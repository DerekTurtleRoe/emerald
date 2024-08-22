//Document may not be the most descriptive name, since folders are included ...
struct Document {
  auto language() const -> string;
  auto name() const -> string;
  auto title() const -> string;
  auto update() -> void;
  auto rename(string) -> void;

  string location;         //file system path
  string type;             //"folder", "binary", "text"
  bool loaded = false;     //used to delay loading contents
  uint64_t timestamp = 0;  //used to detect when a file was modified externally after loading
  bool writable = false;   //indicates whether location is writable (true) or read-only (false)
  bool modified = false;   //only "text" files will set modified = true
  bool desynced = false;   //set to true when file has been modified externally
  SourceEdit sourceEdit;
  TreeViewItem treeViewItem;
};

struct Program : Window {
  Program();
  auto main(Arguments) -> void;
  auto close() -> void;
  auto setTitle() -> void;
  auto scan(string pathname) -> void;
  template<typename T> auto scan(T parent, string pathname) -> void;
  template<typename T> auto append(T parent, string location = "") -> TreeViewItem;

  template<typename T> auto documentFind(T item) -> shared_pointer<Document>;
  auto documentActive() -> shared_pointer<Document>;
  auto documentActivate() -> void;
  auto documentChange() -> void;
  auto documentModify() -> void;
  auto documentSave(Window parent, shared_pointer<Document>) -> void;
  auto documentBinary(string location) -> string;

  auto findNext() -> void;
  auto findPrevious() -> void;

  auto gotoLine() -> void;

  string rootLocation;
  vector<shared_pointer<Document>> documents;

  MenuBar menuBar{this};
    Menu fileMenu{&menuBar};
      MenuItem saveAction{&fileMenu};
      MenuItem saveAllAction{&fileMenu};
      MenuSeparator quitSeparator{&fileMenu};
      MenuItem quitAction{&fileMenu};
    Menu searchMenu{&menuBar};
      MenuItem findAction{&searchMenu};
      MenuItem gotoAction{&searchMenu};
    Menu helpMenu{&menuBar};
      MenuItem aboutAction{&helpMenu};

  HorizontalLayout layout{this};
    TreeView treeView{&layout, Size{200, ~0}, 0};
    HorizontalResizeGrip resizeGrip{&layout, Size{5, ~0}, 0};
    VerticalLayout editorLayout{&layout, Size{~0, ~0}};
      HorizontalLayout documentLayout{&editorLayout, Size{~0, ~0}, 3};
        TextEdit noDocument{&documentLayout, Size{~0, ~0}};
      HorizontalLayout findLayout{&editorLayout, Size{~0, 0}, 3};
        Label findLabel{&findLayout, Size{0, 0}};
        LineEdit findEdit{&findLayout, Size{~0, 0}};
        Button findNextButton{&findLayout, Size{0, 0}, 0};
        Button findPreviousButton{&findLayout, Size{0, 0}, 0};
        Button findCloseButton{&findLayout, Size{0, 0}, 0};
      HorizontalLayout gotoLayout{&editorLayout, Size{~0, 0}, 3};
        Label gotoLabel{&gotoLayout, Size{0, 0}};
        LineEdit gotoEdit{&gotoLayout, Size{~0, 0}};
        Button gotoLineButton{&gotoLayout, Size{0, 0}, 0};
        Button gotoCloseButton{&gotoLayout, Size{0, 0}, 0};

  PopupMenu treeViewFolderMenu;
    MenuItem newFolderAction{&treeViewFolderMenu};
    MenuItem newFileAction{&treeViewFolderMenu};
    MenuItem renameDocumentAction{&treeViewFolderMenu};
    MenuItem removeDocumentAction{&treeViewFolderMenu};

  Timer keyboardPollTimer;
  float resizeWidth = 0;
};

struct SaveDialog : Window {
  SaveDialog();
  auto run() -> bool;
  auto saveSelected() -> void;
  auto close(bool quit) -> void;

  bool quit = true;

  VerticalLayout layout{this};
    Label promptLabel{&layout, Size{~0, 0}};
    TableView tableView{&layout, Size{~0, ~0}};
    HorizontalLayout controlLayout{&layout, Size{~0, 0}};
      Button selectAllButton{&controlLayout, Size{100, 0}};
      Button deselectAllButton{&controlLayout, Size{100, 0}};
      Widget controlSpacer{&controlLayout, Size{~0, 0}};
      Button quitButton{&controlLayout, Size{100, 0}};
      Button saveQuitButton{&controlLayout, Size{100, 0}};
      Button cancelButton{&controlLayout, Size{100, 0}};
};
