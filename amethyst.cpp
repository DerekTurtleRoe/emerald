#include <nall/nall.hpp>
using namespace nall;

#include <hiro/hiro.hpp>
using namespace hiro;

#include "amethyst.hpp"
namespace Instances { Instance<Program> program; }
namespace Instances { Instance<SaveDialog> saveDialog; }
Program& program = Instances::program();
SaveDialog& saveDialog = Instances::saveDialog();
Markup::Node settings;
Markup::Node mimetypes;

auto about() -> void {
  AboutDialog()
  .setName("amethyst")
  .setDescription("A lightweight source code editor with tree-view navigation")
  .setVersion("4")
  .setLicense("ISC", "https://opensource.org/licenses/ISC")
  .setAlignment(program)
  .show();
}

auto locate(string location) -> string {
  if(inode::exists({Path::program(), location})) return {Path::program(), location};
  directory::create({Path::userSettings(), "amethyst/"});
  return {Path::userSettings(), "amethyst/", location};
}

auto getFont(string path) -> Font {
  return Font().setFamily(settings[{path, "/family"}].text()).setSize(settings[{path, "/size"}].real());
}

auto getColor(string path) -> Color {
  return Color().setValue(255 << 24 | settings[path].natural());
}

auto getFloat(string path) -> float {
  return settings[path].real();
}

auto getString(string path) -> string {
  return settings[path].text();
}

//color syntax highlighting
auto Document::language() const -> string {
  auto name = Location::base(location);
  for(auto file : mimetypes.find("file")) {
    if(name.match(file["match"].text())) return file["type"].text();
  }
  return {};  //treat unmatched files as binary files
}

//filename
auto Document::name() const -> string {
  string name = Location::base(location);
  if(type == "folder") return name.trimRight("/", 1L);
  if(!name) name = "(untitled)";
  if(modified) name.prepend("*");
  if(desynced) name.prepend("!");
  return name;
}

//filename (pathname)
auto Document::title() const -> string {
  string title = name();
  if(location) title.append(" (", Location::dir(location), ")");
  if(!writable) title.append(" [read-only]");
  return title;
}

auto Document::update() -> void {
  if(type == "folder") {
    if(treeViewItem.expanded()) {
      treeViewItem.setIcon(Icon::Emblem::FolderOpen);
    } else {
      treeViewItem.setIcon(Icon::Emblem::Folder);
    }
  }
  if(type == "binary") treeViewItem.setIcon(Icon::Emblem::Binary);
  if(type == "text") treeViewItem.setIcon(Icon::Emblem::Text);
  if(!writable) {
    treeViewItem.setForegroundColor(getColor("browser/color/readonly"));
  } else if(modified) {
    treeViewItem.setForegroundColor(getColor("browser/color/modified"));
  } else if(desynced) {
    treeViewItem.setForegroundColor(getColor("browser/color/desynced"));
  } else {
    treeViewItem.setForegroundColor(getColor("browser/color/standard"));
  }
  treeViewItem.setText(name());
}

//update the location of a document and all of its children
auto Document::rename(string name) -> void {
  auto oldLocation = location;
  auto newLocation = Location::dir(location).append(name);
  location = newLocation;
  update();
  for(auto document : program.documents) {
    if(document.data() == this) continue;  //location already updated
    if(!document->location.beginsWith(oldLocation)) continue;  //not a descendent of the renamed document
    document->location.trimLeft(oldLocation, 1L);
    document->location.prepend(newLocation);
    document->update();
  }
  program.setTitle();
}

Program::Program() {
  fileMenu.setText("File");
  saveAction.setText("Save").setIcon(Icon::Action::Save).onActivate([&] { documentSave(program, documentActive()); });
  saveAllAction.setText("Save All").setIcon(Icon::Action::Save).onActivate([&] {
    for(auto document : documents) documentSave(program, document);
  });
  quitAction.setText("Quit").setIcon(Icon::Action::Quit).onActivate([&] { close(); });

  searchMenu.setText("Search");
  findAction.setText("Find").setIcon(Icon::Edit::Find).onActivate([&] {
    gotoLayout.setVisible(false);
    findEdit.setText("");
    findLayout.setVisible(true);
    findEdit.setFocused();
    layout.resize();
  });
  gotoAction.setText("Goto").setIcon(Icon::Go::Right).onActivate([&] {
    findLayout.setVisible(false);
    gotoEdit.setText("");
    gotoLayout.setVisible(true);
    gotoEdit.setFocused();
    layout.resize();
  });

  helpMenu.setText("Help");
  aboutAction.setText("About ...").setIcon(Icon::Prompt::Question).onActivate([&] { about(); });

  menuBar.setFont(getFont("menu/font"));

  layout.cell(treeView).setSize({getFloat("browser/width"), ~0});
  treeView.setActivation(Mouse::Click::Single);
  treeView.setCollapsible();
  treeView.setFont(getFont("browser/font"));
  treeView.setBackgroundColor(getColor("browser/color/background"));
  treeView.setForegroundColor(getColor("browser/color/standard"));
  treeView.onActivate([&] { documentActivate(); });
  treeView.onChange([&] { documentChange(); });
  treeView.onContext([&] {
    if(auto document = documentActive()) {
      newFolderAction.setEnabled(document->writable).setVisible(document->type == "folder");
      newFileAction.setEnabled(document->writable).setVisible(document->type == "folder");
      renameDocumentAction.setEnabled(document->location && document->writable).setVisible(true);
      removeDocumentAction.setEnabled(document->location && document->writable).setVisible(true);
      renameDocumentAction.setText(document->type == "folder" ? "Rename Folder ..." : "Rename File ...");
      removeDocumentAction.setText(document->type == "folder" ? "Remove Folder ..." : "Remove File ...");
    } else {
      newFolderAction.setEnabled(directory::writable(rootLocation)).setVisible(true);
      newFileAction.setEnabled(directory::writable(rootLocation)).setVisible(true);
      renameDocumentAction.setVisible(false);
      removeDocumentAction.setVisible(false);
    }
    treeViewFolderMenu.setVisible();
  });

  resizeGrip.setCollapsible();
  resizeGrip.onActivate([&] {
    resizeWidth = layout.cell(treeView).size().width();
  });
  resizeGrip.onResize([&](auto offset) {
    float min = 128, max = layout.geometry().width() - 128;
    float width = resizeWidth + offset;
    width = width < min ? min : width > max ? max : width;
    if(layout.cell(treeView).size().width() != width) {
      layout.cell(treeView).setSize({width, ~0});
      layout.resize();
    }
  });

  noDocument.setCollapsible();
  noDocument.setBackgroundColor(getColor("editor/color/background"));
  noDocument.setEditable(false);

  findLayout.setCollapsible();
  findLayout.setVisible(false);
  findLabel.setText(" Find:").setFont(getFont("window/font"));
  findEdit.setFont(getFont("find/font"));
  findEdit.setBackgroundColor(getColor("find/color/background"));
  findEdit.setForegroundColor(getColor("find/color/standard"));
  findEdit.onActivate([&] { findNext(); });
  findNextButton.setBordered(false).setIcon(Icon::Go::Down).onActivate([&] { findNext(); });
  findPreviousButton.setBordered(false).setIcon(Icon::Go::Up).onActivate([&] { findPrevious(); });
  findCloseButton.setBordered(false).setIcon(Icon::Action::Close).onActivate([&] {
    findLayout.setVisible(false);
    layout.resize();
  });

  gotoLayout.setCollapsible();
  gotoLayout.setVisible(false);
  gotoLabel.setText(" Goto:").setFont(getFont("window/font"));
  gotoEdit.setFont(getFont("goto/font"));
  gotoEdit.setBackgroundColor(getColor("goto/color/background"));
  gotoEdit.setForegroundColor(getColor("goto/color/standard"));
  gotoEdit.onActivate([&] { gotoLine(); });
  gotoLineButton.setBordered(false).setIcon(Icon::Go::Right).onActivate([&] { gotoLine(); });
  gotoCloseButton.setBordered(false).setIcon(Icon::Action::Close).onActivate([&] {
    gotoLayout.setVisible(false);
    layout.resize();
  });

  newFolderAction.setText("New Folder ...").setIcon(Icon::Emblem::Folder).onActivate([&] {
    auto location = rootLocation;
    if(auto document = documentActive()) {
      if(!document->treeViewItem.expanded()) treeView.doActivate();  //let the user know which names are already taken
      location = document->location;
    }
    if(auto name = NameDialog()
    .setIcon(Icon::Emblem::Folder)
    .setAlignment(*this)
    .create()
    ) {
      if(inode::exists({location, name})) {
        return (void)MessageDialog().setTitle("amethyst").setAlignment(program).setText("Chosen name already exists.").error();
      }
      if(!directory::create({location, name})) {
        return (void)MessageDialog().setTitle("amethyst").setAlignment(program).setText("Failed to create folder.").error();
      }
      if(auto parent = treeView.selected()) {
        append(parent, {location, name, "/"}).setSelected();
      } else {
        append(treeView, {location, name, "/"}).setSelected();
      }
      treeView.doChange();
    }
  });

  newFileAction.setText("New File ...").setIcon(Icon::Emblem::File).onActivate([&] {
    auto location = rootLocation;
    if(auto document = documentActive()) {
      if(!document->treeViewItem.expanded()) treeView.doActivate();  //let the user know which names are already taken
      location = document->location;
    }
    if(auto name = NameDialog()
    .setIcon(Icon::Emblem::File)
    .setAlignment(*this)
    .create()
    ) {
      if(inode::exists({location, string{name}.trimRight("/", 1L)})) {
        return (void)MessageDialog().setTitle("amethyst").setAlignment(program).setText("Chosen name already exists.").error();
      }
      if(!file::create({location, name})) {
        return (void)MessageDialog().setTitle("amethyst").setAlignment(program).setText("Failed to create file.").error();
      }
      if(auto parent = treeView.selected()) {
        append(parent, {location, name}).setSelected();
      } else {
        append(treeView, {location, name}).setSelected();
      }
      treeView.doChange();
    }
  });

  renameDocumentAction.setIcon(Icon::Application::TextEditor).onActivate([&] {
    if(auto document = documentActive()) {
      image icon{Icon::Emblem::File};
      if(document->type == "folder") icon = {Icon::Emblem::Folder};
      if(auto newName = NameDialog()
      .setIcon(icon)
      .setAlignment(*this)
      .rename(document->name())
      ) {
        auto oldName = Location::base(document->location);
        if(newName == oldName) return;
        auto location = Location::dir(document->location);
        if(inode::exists({location, newName})) {
          return (void)MessageDialog().setTitle("amethyst").setAlignment(program).setText("Chosen name already exists.").error();
        }
        if(!inode::rename({location, oldName}, {location, newName})) {
          string type = document->type == "folder" ? "folder." : "file.";
          return (void)MessageDialog().setTitle("amethyst").setAlignment(program).setText({"Failed to rename ", type}).error();
        }
        if(document->type == "folder") newName.append("/");
        document->rename(newName);
      }
    }
  });

  removeDocumentAction.setIcon(Icon::Edit::Delete).onActivate([&] {
    if(auto document = documentActive()) {
      if(MessageDialog().setTitle("amethyst").setAlignment(program).setText({
        "Are you sure you want to permanently delete this ",
        document->type == "folder" ? "folder, and all of its contents?\n\n" : "file?\n\n",
        document->title()
      }).question() == "No") return;
      if(document->type == "folder") {
        if(!document->treeViewItem.expanded()) treeView.doActivate();  //show the user what will be deleted
        if(!directory::remove(document->location)) {
          return (void)MessageDialog().setTitle("amethyst").setAlignment(program).setText("Failed to remove folder.").error();
        }
      } else {
        if(!file::remove(document->location)) {
          return (void)MessageDialog().setTitle("amethyst").setAlignment(program).setText("Failed to remove file.").error();
        }
      }
      document->treeViewItem.remove();
      if(auto index = documents.find(document)) documents.remove(index());
    }
  });

  onClose([&] { close(); });

  auto x = getFloat("window/x"), y = getFloat("window/y"), width = getFloat("window/width"), height = getFloat("window/height");
  auto workspace = Desktop::workspace();
  setFrameGeometry({
    x ? x : workspace.x(),
    y ? y : workspace.y(),
    width ? width : workspace.width() - x,
    height ? height : workspace.height() - y
  });

  Keyboard::append(Hotkey().setSequence("Escape").onPress([&] { if(program.focused()) {
    if(findLayout.visible()) {
      findLayout.setVisible(false);
      layout.resize();
      if(auto document = documentActive()) document->sourceEdit.setFocused();
    }
    if(gotoLayout.visible()) {
      gotoLayout.setVisible(false);
      layout.resize();
      if(auto document = documentActive()) document->sourceEdit.setFocused();
    }
  }}));

  Keyboard::append(Hotkey().setSequence("Control+S").onPress([&] { if(program.focused()) {
    saveAction.doActivate();
  }}));

  Keyboard::append(Hotkey().setSequence("Control+F").onPress([&] { if(program.focused()) {
    findAction.doActivate();
  }}));

  Keyboard::append(Hotkey().setSequence("Control+G").onPress([&] { if(program.focused()) {
    gotoAction.doActivate();
  }}));

  Keyboard::append(Hotkey().setSequence("Control+Q").onPress([&] { if(program.focused()) {
    quitAction.doActivate();
  }}));

  Keyboard::append(Hotkey().setSequence("Control+Grave").onPress([&] { if(program.focused()) {
    bool visible = !treeView.visible();
    treeView.setVisible(visible);
    resizeGrip.setVisible(visible);
    if(!visible) {
      if(auto document = documentActive()) document->sourceEdit.setFocused();
    }
    layout.resize();
  }}));

  keyboardPollTimer.setInterval(50).onActivate([&] {
    Keyboard::poll();
  });
}

//if no files or a single file is loaded, hide the TreeView and give focus to the SourceEdit control
//if a directory is loaded, show the TreeView and do not select any items (for delayed file loading)
auto Program::main(Arguments arguments) -> void {
  if(arguments.size() == 1 && directory::exists(arguments[0])) {
    scan(treeView, rootLocation = arguments[0]);
  } else {
    if(arguments.size() == 1 && file::exists(arguments[0])) {
      rootLocation = Location::dir(arguments[0]);
      append(treeView, arguments[0]).setSelected();
    } else {
      rootLocation = Path::user();
      append(treeView).setSelected();
    }
    treeView.setVisible(false).doChange();
    resizeGrip.setVisible(false);
  }
  setTitle();
  setVisible();
  keyboardPollTimer.setEnabled();
  Application::run();
}

auto Program::close() -> void {
  if(saveDialog.run()) Application::quit();
}

auto Program::setTitle() -> void {
  if(auto document = documentActive()) {
    Window::setTitle(document->title());
  } else {
    Window::setTitle({"(", rootLocation, ")"});
  }
}

template<typename T> auto Program::scan(T parent, string location) -> void {
  for(auto& name : directory::contents(location)) {
    append(parent, {location, name});
  }
}

template<typename T> auto Program::append(T parent, string location) -> TreeViewItem {
  shared_pointer<Document> document{new Document};
  document->location = location;
  TreeViewItem item{&parent};
  document->treeViewItem = item;
  //note: this will leak a small amount of memory per document
  //it is a workaround for hiro only supporting strings as Object properties
  auto pointer = new shared_pointer_weak<Document>{document};
  item.setAttribute("document", (uintptr)pointer);
  if(location.endsWith("/")) {
    document->type = "folder";
  } else if(location) {
    document->type = document->language() ? "text" : "binary";
  } else {
    document->type = "text";
  }
  document->writable = !location || inode::writable(location);
  document->update();
  documents.append(document);
  return item;
}

template<typename T> auto Program::documentFind(T item) -> shared_pointer<Document> {
  if(item) {
    if(auto pointer = (shared_pointer_weak<Document>*)item.attribute("document").natural()) {
      if(auto document = pointer->acquire()) return document;
    }
  }
  return {};
}

auto Program::documentActive() -> shared_pointer<Document> {
  return documentFind(treeView.selected());
}

auto Program::documentActivate() -> void {
  if(auto item = treeView.selected()) {
    if(auto document = documentFind(item)) {
      if(document->type == "folder") {
        if(!document->loaded) {
          document->loaded = true;
          scan(document->treeViewItem, document->location);
        }
        item.setExpanded(!item.expanded());
        document->update();
      }
    }
  }
}

auto Program::documentChange() -> void {
  noDocument->setVisible(true);
  //at this point, the previously selected TreeView item (document) has lost focus ...
  //since it is unknown since document was active, ensure that every document is hidden instead
  for(auto document : documents) {
    if(document->sourceEdit.visible()) document->sourceEdit.setVisible(false);
  }

  if(auto item = treeView.selected()) {
    auto document = documentFind(item);
    if(document && document->type != "folder") {
      if(!document->loaded) {
        //do not load files that are so large they would hang the editor for a long time ...
        if(file::size(document->location) >= 64 * 1024 * 1024) return;
        document->loaded = true;
        document->timestamp = file::timestamp(document->location, file::time::modify);
        document->writable = !document->location || file::writable(document->location);
        document->sourceEdit.onChange([&] { documentModify(); });
        document->sourceEdit.setCollapsible();
        document->sourceEdit.setFont(getFont("editor/font"));
        document->sourceEdit.setWordWrap(false);
        document->sourceEdit.setLanguage(document->language());
        document->sourceEdit.setScheme(getString("editor/scheme"));
        if(document->type == "text") {
          document->sourceEdit.setNumbered(true);
          document->sourceEdit.setText(file::read(document->location));
          document->sourceEdit.setEditable(document->writable);
        } else {
          document->sourceEdit.setNumbered(false);
          document->sourceEdit.setText(documentBinary(document->location));
          document->sourceEdit.setEditable(false);
        }
        documentLayout.append(document->sourceEdit, Size{~0, ~0});
        document->sourceEdit.setTextCursor();
        document->update();  //writable property may have changed since parent directory was scanned
      } else if(file::timestamp(document->location, file::time::modify) > document->timestamp) {
        //file has been modified externally since it was last selected ...
        document->desynced = true;
        document->update();
        setTitle();
      }
      document->sourceEdit.setVisible(true);
      noDocument->setVisible(false);
    }
  }

  setTitle();
  layout.resize();
  //todo: somehow, this prevents a brief flickering when changing documents for the first time,
  //where the window color (usually bright gray) paints in place of the SourceEdit control
  Application::processEvents();
}

auto Program::documentModify() -> void {
  if(auto document = documentActive()) {
    if(!document->modified) {
      document->modified = true;
      document->update();
      setTitle();
    }
  }
}

auto Program::documentSave(Window parent, shared_pointer<Document> document) -> void {
  if(!document || !document->modified) return;
  if(!document->location) {
    document->location = BrowserDialog().setAlignment(parent).saveFile();
    if(!document->location) return;
  } else {
    auto timestamp = file::timestamp(document->location, file::time::modify);
    if(timestamp > document->timestamp) {
      if(MessageDialog().setAlignment(parent).setTitle("amethyst").setText({
        "File modified externally since opening. Save anyway?\n\n",
        document->title()
      }).warning({"Yes", "No"}) == "No") return;
    }
  }
  if(!file::write(document->location, document->sourceEdit.text())) {
    MessageDialog().setAlignment(parent).setTitle("amethyst").setText({
      "Failed to save file. Perhaps file permissions changed after loading.\n\n",
      document->title()
    }).error();
    return;
  }
  document->timestamp = file::timestamp(document->location, file::time::modify);
  document->modified = false;
  document->desynced = false;
  document->update();
  if(document == documentActive()) setTitle();
}

auto Program::documentBinary(string location) -> string {
  string output;
  auto data = file::read(location);
  uint offset = 0;
  do {
    output.append(hex(offset, 8L), "  ");
    for(uint index : range(16)) {
      if(offset + index < data.size()) {
        output.append(hex(data[offset + index], 2L), " ");
      } else {
        output.append("   ");
      }
    }
    output.append(" ");
    for(uint index : range(16)) {
      if(offset + index < data.size()) {
        char byte = data[offset + index];
        output.append(byte >= 0x20 && byte <= 0x7e ? byte : '.');
      } else {
        output.append(" ");
      }
    }
    output.append("\n");
    offset += 16;
  } while(offset < data.size());
  return output;
}

//note: GTK SourceEdit::Cursor takes UTF-8 character indexes;
//nall/string functions use uint8_t indexes.
//use string::characters() to convert utf8_t to UTF-8 indexes below

auto Program::findNext() -> void {
  if(!findEdit.text()) return;
  if(auto document = documentActive()) {
    auto search = findEdit.text();
    auto text = document->sourceEdit.text();
    auto cursor = document->sourceEdit.textCursor();
    if(auto match = text.findNext(cursor.offset(), search)) {
      int offset = text.characters(0, match());
      int length = search.characters();
      document->sourceEdit.setTextCursor({offset, length});
    } else if(match = text.find(search)) {
      int offset = text.characters(0, match());
      int length = search.characters();
      document->sourceEdit.setTextCursor({offset, length});
    }
  }
}

auto Program::findPrevious() -> void {
  if(!findEdit.text()) return;
  if(auto document = documentActive()) {
    auto search = findEdit.text();
    auto text = document->sourceEdit.text();
    auto cursor = document->sourceEdit.textCursor();
    if(auto match = text.findPrevious(cursor.offset(), search)) {
      int offset = text.characters(0, match());
      int length = search.characters();
      document->sourceEdit.setTextCursor({offset, length});
    } else if(match = text.findPrevious(text.size(), search)) {
      int offset = text.characters(0, match());
      int length = search.characters();
      document->sourceEdit.setTextCursor({offset, length});
    }
  }
}

auto Program::gotoLine() -> void {
  if(!gotoEdit.text()) return;
  if(auto document = documentActive()) {
    auto line = max(1, gotoEdit.text().natural());
    auto text = document->sourceEdit.text();
    uint currentLine = 1;
    for(uint offset : range(text.size())) {
      if(line == currentLine) {
        offset = text.characters(0, offset);
        document->sourceEdit.setTextCursor(offset);
        return;
      }
      if(text[offset] == '\n') currentLine++;
    }
    document->sourceEdit.setTextCursor(text.characters());
  }
}

SaveDialog::SaveDialog() {
  layout.setPadding(5);
  promptLabel.setFont(getFont("window/font"));
  tableView.setFont(getFont("save/font"));
  tableView.setBackgroundColor(getColor("save/color/background"));
  tableView.setForegroundColor(getColor("save/color/standard"));
  selectAllButton.setText("Select All").setFont(getFont("window/font")).onActivate([&] {
    for(auto item : tableView.items()) item.cell(0).setChecked(true);
  });
  deselectAllButton.setText("Deselect All").setFont(getFont("window/font")).onActivate([&] {
    for(auto item : tableView.items()) item.cell(0).setChecked(false);
  });
  quitButton.setText("Don't Save").setFont(getFont("window/font")).onActivate([&] { close(true); });
  saveQuitButton.setText("Save & Quit").setFont(getFont("window/font")).onActivate([&] { saveSelected(); close(true); });
  cancelButton.setText("Cancel").setFont(getFont("window/font")).onActivate([&] { close(false); });
  onClose([&] { close(false); });
  setTitle("amethyst");
  setDismissable();
  setSize({640, 400});
}

auto SaveDialog::run() -> bool {
  tableView.reset();
  tableView.append(TableViewColumn().setWidth(~0));
  for(auto document : program.documents) {
    if(!document->modified) continue;
    TableViewItem item{&tableView};
    item.setAttribute("document", document->treeViewItem.attribute("document"));
    TableViewCell cell{&item};
    cell.setText(document->title().trimLeft("*", 1L)).setCheckable().setChecked();
  }
  auto modified = tableView.itemCount();
  if(modified == 0) return true;
  promptLabel.setText({"The following document", modified == 1 ? " has" : "s have", " been modified. Save changes?"});

  setAlignment(program);
  setVisible();
  cancelButton.setFocused();
  quit = true;
  setModal();
  return quit;
}

auto SaveDialog::saveSelected() -> void {
  for(auto item : tableView.items()) {
    if(!item.cell(0).checked()) continue;
    if(auto document = program.documentFind(item)) {
      program.documentSave(saveDialog, document);
    }
  }
}

auto SaveDialog::close(bool quit) -> void {
  this->quit = quit;
  setModal(false);
  setVisible(false);
}

#include <nall/main.hpp>
auto nall::main(Arguments arguments) -> void {
  settings = BML::unserialize(file::read(locate("settings.bml")));
  Application::setName("amethyst");

  Instances::program.construct();
  Instances::saveDialog.construct();

  if(!file::exists(locate("settings.bml")) || !file::exists(locate("mimetypes.bml"))) {
    return (void)MessageDialog().setTitle("amethyst").setText({
      "Missing configuration files.\n"
      "Please run 'make install' before using this software."
    });
  }

  //todo: somehow, settings is reset between hiro::initialize() and nall::main()
  //this is a *serious* problem, but for now, reload settings to work around it
  settings = BML::unserialize(file::read(locate("settings.bml")));
  mimetypes = BML::unserialize(file::read(locate("mimetypes.bml")));
  program.main(arguments);

  Instances::program.destruct();
  Instances::saveDialog.destruct();
}
