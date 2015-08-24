struct Utility {
  void setInterface(Emulator::Interface* emulator);

  void loadMedia(string pathname);
  void loadMedia(Emulator::Interface* emulator, Emulator::Interface::Media& media, string pathname);

  void loadRequest(unsigned id, string name, string type, bool required);
  void loadRequest(unsigned id, string path, bool required);
  void saveRequest(unsigned id, string path, bool required);

  void connect(unsigned port, unsigned device);
  void power();
  void reset();
  void load();
  void unload();

  void saveState(unsigned slot);
  void loadState(unsigned slot);

  void synchronizeDSP();
  void synchronizeRuby();
  void updatePalette();
  void updateShader();
  void resize(bool resizeWindow = false);
  void toggleFullScreen();

  void updateStatus();
  void setStatusText(string text);
  void showMessage(string message);

  string libraryPath();

  lstring path;
  lstring pathname;

private:
  string statusText;
  string statusMessage;
  time_t statusTime = 0;
};

extern Utility* utility;
