auto Program::path(uint id) -> string {
  return mediumPaths(id);
}

auto Program::open(uint id, string name, vfs::file::mode mode, bool required) -> vfs::shared::file {
  if(name == "manifest.bml" && !path(id).endsWith(".sys/")) {
    if(!file::exists({path(id), name})) {
      if(auto manifest = execute("cart-pal", "--manifest", path(id))) {
        return vfs::memory::file::open(manifest.output.data<uint8_t>(), manifest.output.size());
      }
    }
  }

  if(auto result = vfs::fs::file::open({path(id), name}, mode)) return result;

  if(required) {
    debugger->print("Error: missing required file:\n", path(id), name, "\n\n");
  }

  return {};
}

auto Program::load(uint id, string name, string type) -> maybe<uint> {
  string location;
  if(mediumQueue) {
    location = mediumQueue.takeLeft().transform("\\", "/");
    if(!location.endsWith("/")) location.append("/");
  } else {
    location = BrowserDialog()
    .setTitle({"Load ", name})
    .setPath({higan_settings["Library/Location"].text(), name})
    .setFilters({string{name, "|*.", type}, "All|*.*"})
    .openFolder();
  }
  if(!directory::exists(location)) return mediumQueue.reset(), nothing;

  directory::create({location, "debug/"});

  uint pathID = mediumPaths.size();
  mediumPaths.append(location);
  return pathID;
}

auto Program::videoRefresh(const uint32* data, uint pitch, uint width, uint height) -> void {
  uint32_t* output;
  uint length;

  if(video->lock(output, length, width, height)) {
    pitch >>= 2, length >>= 2;

    for(auto y : range(height)) {
      memory::copy(output + y * length, data + y * pitch, width * sizeof(uint32));
    }

    video->unlock();
    video->refresh();
  }

  /*
  uint32* output = presentation->canvas.data();
  pitch >>= 2;

  bool interlace = pitch == 512;
  if(interlace == false) {
    for(uint y = 0; y < height; y++) {
      const uint32 *sp = data + y * pitch;
      uint32* dp0 = output + y * pitch, *dp1 = dp0 + (pitch >> 1);
      for(uint x = 0; x < width; x++) {
        *dp0++ = palette[*sp];
        *dp1++ = palette[*sp++];
      }
    }
  } else {
    for(uint y = 0; y < height; y++) {
      const uint32* sp = data + y * pitch;
      uint32* dp = output + y * 512; // outputPitch
      for(uint x = 0; x < width; x++) {
        *dp++ = palette[*sp++];
      }
    }
  }

  presentation->canvas.setData();
  */
}

auto Program::audioSample(const double* samples, uint channels) -> void {
  if(settings["Audio/Mute"].boolean()) return audio->sample(0, 0);
  int16 left  = sclamp<16>(samples[0] * 32768.0);
  int16 right = sclamp<16>(samples[1] * 32768.0);
  audio->sample(left, right);
}

auto Program::inputPoll(uint port, uint device, uint input) -> int16 {
  if(presentation->focused() == false) return 0;

  if(port == SFC::ID::Port::Controller1) {
    if(device == SFC::ID::Device::Gamepad) {
      switch(input) {
      case SFC::Gamepad::Up:     return hiro::Keyboard::pressed("Up");
      case SFC::Gamepad::Down:   return hiro::Keyboard::pressed("Down");
      case SFC::Gamepad::Left:   return hiro::Keyboard::pressed("Left");
      case SFC::Gamepad::Right:  return hiro::Keyboard::pressed("Right");
      case SFC::Gamepad::B:      return hiro::Keyboard::pressed("Z");
      case SFC::Gamepad::A:      return hiro::Keyboard::pressed("X");
      case SFC::Gamepad::Y:      return hiro::Keyboard::pressed("A");
      case SFC::Gamepad::X:      return hiro::Keyboard::pressed("S");
      case SFC::Gamepad::L:      return hiro::Keyboard::pressed("D");
      case SFC::Gamepad::R:      return hiro::Keyboard::pressed("C");
      case SFC::Gamepad::Select: return hiro::Keyboard::pressed("Apostrophe");
      case SFC::Gamepad::Start:  return hiro::Keyboard::pressed("Enter");
      }
    }
  }

  return 0;
}
