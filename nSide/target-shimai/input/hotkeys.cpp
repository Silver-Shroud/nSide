auto InputManager::appendHotkeys() -> void {
  static int quickStateSlot = 0;

  { auto hotkey = new InputHotkey;
    hotkey->name = "Toggle Fullscreen";
    hotkey->press = [] {
      presentation->toggleFullScreen();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Toggle Mouse Capture";
    hotkey->press = [] {
      input->acquired() ? input->release() : input->acquire();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Save Quick State";
    hotkey->press = [] {
      program->saveState(quickStateSlot);
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Load Quick State";
    hotkey->press = [] {
      program->loadState(quickStateSlot);
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Decrement Quick State";
    hotkey->press = [&] {
      if(--quickStateSlot < 0) quickStateSlot = 9;
      program->showMessage({"Selected quick state slot ", quickStateSlot});
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Increment Quick State";
    hotkey->press = [&] {
      if(++quickStateSlot > 9) quickStateSlot = 0;
      program->showMessage({"Selected quick state slot ", quickStateSlot});
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Pause Emulation";
    hotkey->press = [] {
      program->pause = !program->pause;
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Fast Forward";
    hotkey->press = [] {
      video->setBlocking(false);
      audio->setBlocking(false);
    };
    hotkey->release = [] {
      video->setBlocking(settings["Video/Synchronize"].boolean());
      audio->setBlocking(settings["Audio/Synchronize"].boolean());
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Power Cycle";
    hotkey->press = [] {
      program->powerCycle();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Rotate Display";
    hotkey->press = [] {
      program->rotateDisplay();
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Save Screenshot";
    hotkey->press = [] {
      if(!::emulator) return;

      auto time = chrono::timestamp();
      string filename = {
        "screenshot-",
        chrono::local::datetime(time).replace(":", ";").replace(" ", "-"),
        ".png"
      };
      Emulator::video.screenshot({program->mediumPaths.right(), filename});
      program->showMessage("Screenshot saved");
    };
    hotkeys.append(hotkey);
  }

  { auto hotkey = new InputHotkey;
    hotkey->name = "Export Memory";
    hotkey->press = [] {
      if(!::emulator) return;
      ::emulator->exportMemory();
      program->showMessage("Memory exported");
    };
    hotkeys.append(hotkey);
  }

  for(auto& hotkey : hotkeys) {
    hotkey->path = string{"Hotkey/", hotkey->name}.replace(" ", "");
    hotkey->assignment = settings(hotkey->path).text();
    hotkey->bind();
  }
}

auto InputManager::pollHotkeys() -> void {
  if(!program->focused()) return;

  for(auto& hotkey : hotkeys) {
    int16 state = hotkey->poll();
    if(hotkey->state == 0 && state == 1 && hotkey->press) hotkey->press();
    if(hotkey->state == 1 && state == 0 && hotkey->release) hotkey->release();
    hotkey->state = state;
  }

  static shared_pointer<HID::Device> keyboard = nullptr;
  if(!keyboard) {
    for(auto& device : inputManager->devices) {
      if(device->isKeyboard()) keyboard = device;
    }
  }

  //Unload/Quit hotkey
  //Because stock higan and nSide don't have this hotkey and yield to the OS's
  //global exit hotkey (Alt+F4 on Windows, etc.), which doesn't work in
  //fullscreen. Key code 0 represents Escape in all of ruby's input drivers.
  static bool previousEsc = 0;
  auto esc = keyboard->group(0).input(0).value();
  if(previousEsc == 0 && esc == 1) {
    if(::emulator) program->unloadMedium();
    else           program->quit();
  }
  previousEsc = esc;

  if(settings["Input/Driver"].text() == "Windows") {
    static shared_pointer<HID::Device> xinput1 = nullptr;
    if(!xinput1) {
      for(auto& device : inputManager->devices) {
        if(device->isJoypad() && device->id() == 0x1'045e'028e) xinput1 = device;
      }
    }

    if(xinput1) {
      auto guide = xinput1->group(HID::Joypad::GroupID::Button).input(10).value();
      if(guide == 1 && ::emulator) program->unloadMedium();
    }
  }
}
