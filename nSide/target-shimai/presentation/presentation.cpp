#include "../shimai.hpp"
unique_pointer<Presentation> presentation;

Presentation::Presentation() {
  presentation = this;

  viewport.setDroppable().onDrop([&](auto locations) {
    if(!directory::exists(locations(0))) return;
    program->mediumQueue.append(locations(0));
    program->loadMedium();
  });

  onClose([&] { program->quit(); });

  setTitle("shimai");
  setResizable(false);
  setBackgroundColor({0, 0, 0});
  resizeViewport();
  setCentered();

  #if defined(PLATFORM_WINDOWS)
  Application::Windows::onModalChange([](bool modal) { if(modal && audio) audio->clear(); });
  #endif

  #if defined(PLATFORM_MACOSX)
  showConfigurationSeparator.setVisible(false);
  showConfiguration.setVisible(false);
  about.setVisible(false);
  Application::Cocoa::onAbout([&] { about.doActivate(); });
  Application::Cocoa::onActivate([&] { setFocused(); });
  Application::Cocoa::onPreferences([&] { showConfiguration.doActivate(); });
  Application::Cocoa::onQuit([&] { doClose(); });
  #endif
}

auto Presentation::updateEmulator() -> void {
  if(!emulator) return;

  emulator->set("Blur Emulation", settings["Video/BlurEmulation"].boolean());
  emulator->set("Color Emulation", settings["Video/ColorEmulation"].boolean());
  emulator->set("Scanline Emulation", settings["Video/ScanlineEmulation"].boolean());
}

auto Presentation::clearViewport() -> void {
  if(!video) return;

  uint32_t* output;
  uint length = 0;
  uint width = viewport.geometry().width();
  uint height = viewport.geometry().height();
  if(video->lock(output, length, width, height)) {
    for(uint y : range(height)) {
      auto dp = output + y * (length >> 2);
      for(uint x : range(width)) *dp++ = 0xff000000;
    }

    video->unlock();
    video->refresh();
  }
}

auto Presentation::resizeViewport() -> void {
  //clear video area before resizing to avoid seeing distorted video momentarily
  clearViewport();

  double emulatorWidth = 320;
  double emulatorHeight = 240;
  double aspectCorrection = 1.0;
  if(emulator) {
    auto resolution = emulator->videoResolution();
    emulatorWidth = resolution.width;
    emulatorHeight = resolution.height;
    aspectCorrection = resolution.aspectCorrection;
    if(emulator->information.overscan && settings["Video/Overscan/Mask"].boolean()) {
      uint overscanHorizontal = settings["Video/Overscan/Horizontal"].natural();
      uint overscanVertical = settings["Video/Overscan/Vertical"].natural();
      emulatorWidth -= overscanHorizontal * 2;
      emulatorHeight -= overscanVertical * 2;
    }
  }
  if(!fullScreen()) {
    if(settings["Video/Windowed/AspectCorrection"].boolean()) emulatorWidth *= aspectCorrection;
    uint viewportMultiplier = 2;
    if(settings["Video/Windowed/Multiplier"].text() == "Small") viewportMultiplier = settings["Video/Windowed/Multiplier/Small"].natural();
    if(settings["Video/Windowed/Multiplier"].text() == "Medium") viewportMultiplier = settings["Video/Windowed/Multiplier/Medium"].natural();
    if(settings["Video/Windowed/Multiplier"].text() == "Large") viewportMultiplier = settings["Video/Windowed/Multiplier/Large"].natural();
    uint viewportWidth = ceil(1280.0 * viewportMultiplier / 3.0);
    uint viewportHeight = ceil(720.0 * viewportMultiplier / 3.0);
    uint multiplier = min(viewportWidth / emulatorWidth, viewportHeight / emulatorHeight);
    if(!settings["Video/Windowed/Adaptive"].boolean()) {
      emulatorWidth *= multiplier;
      emulatorHeight *= multiplier;
      setSize({viewportWidth, viewportHeight});
      viewport.setGeometry({
        (viewportWidth - emulatorWidth) / 2, (viewportHeight - emulatorHeight) / 2,
        emulatorWidth, emulatorHeight
      });
    } else {
      setSize({emulatorWidth * multiplier, emulatorHeight * multiplier});
      viewport.setGeometry({0, 0, emulatorWidth * multiplier, emulatorHeight * multiplier});
    }
  } else {
    if(settings["Video/Fullscreen/AspectCorrection"].boolean()) emulatorWidth *= aspectCorrection;
    uint viewportWidth = geometry().width();
    uint viewportHeight = geometry().height();
    if(!settings["Video/Fullscreen/Adaptive"].boolean()) {
      uint multiplier = min(viewportWidth / emulatorWidth, viewportHeight / emulatorHeight);
      emulatorWidth *= multiplier;
      emulatorHeight *= multiplier;
    } else {
      double multiplier = min(viewportWidth / emulatorWidth, viewportHeight / emulatorHeight);
      emulatorWidth *= multiplier;
      emulatorHeight *= multiplier;
    }
    viewport.setGeometry({
      (viewportWidth - emulatorWidth) / 2, (viewportHeight - emulatorHeight) / 2,
      emulatorWidth, emulatorHeight
    });
  }

  //clear video area again to ensure entire viewport area has been painted in
  clearViewport();
}

auto Presentation::toggleFullScreen() -> void {
  if(!fullScreen()) {
    setResizable(true);
    setFullScreen(true);
    if(!input->acquired()) input->acquire();
  } else {
    if(input->acquired()) input->release();
    setFullScreen(false);
    setResizable(false);
  }
  resizeViewport();
}
