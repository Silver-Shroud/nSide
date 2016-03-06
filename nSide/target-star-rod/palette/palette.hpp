struct PaletteViewer : Window {
  VerticalLayout layout{this};
    HorizontalLayout controlLayout{&layout, Size{~0, 0}, 5};
      Widget spacer{&controlLayout, Size{~0, 0}};
      CheckLabel autoUpdate{&controlLayout, Size{0, 0}, 5};
      Button update{&controlLayout, Size{80, 0}};
    Canvas canvas{&layout, Size{256, 256}};

  StatusBar statusBar{this};

  auto updateColors() -> void;
  PaletteViewer();
};

extern unique_pointer<PaletteViewer> paletteViewer;
