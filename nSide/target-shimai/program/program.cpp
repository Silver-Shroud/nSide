#include "../shimai.hpp"
#include <a2600/interface/interface.hpp>
#include <fc/interface/interface.hpp>
#if defined(PROFILE_BALANCED)
#include <sfc-balanced/interface/interface.hpp>
#else
#include <sfc/interface/interface.hpp>
#endif
#include <ms/interface/interface.hpp>
#include <md/interface/interface.hpp>
#include <pce/interface/interface.hpp>
#include <gb/interface/interface.hpp>
#include <gba/interface/interface.hpp>
#include <ws/interface/interface.hpp>
#include "interface.cpp"
#include "medium.cpp"
#include "state.cpp"
#include "utility.cpp"
unique_pointer<Program> program;

Program::Program(string_vector args) {
  program = this;

  Emulator::platform = this;
  emulators.append(new Atari2600::Interface);
  emulators.append(new Famicom::FamicomInterface);
  emulators.append(new SuperFamicom::Interface);
  emulators.append(new MasterSystem::SG1000Interface);
  emulators.append(new MasterSystem::MasterSystemInterface);
  emulators.append(new MegaDrive::Interface);
  emulators.append(new PCEngine::PCEngineInterface);
  emulators.append(new PCEngine::SuperGrafxInterface);
  emulators.append(new GameBoy::GameBoyInterface);
  emulators.append(new GameBoy::GameBoyColorInterface);
  emulators.append(new GameBoyAdvance::Interface);
  emulators.append(new MasterSystem::GameGearInterface);
  emulators.append(new WonderSwan::WonderSwanInterface);
  emulators.append(new WonderSwan::WonderSwanColorInterface);
  emulators.append(new Famicom::VSSystemInterface);
  emulators.append(new Famicom::PlayChoice10Interface);
  emulators.append(new Famicom::FamicomBoxInterface);

  new Presentation;
  presentation->setVisible();

  video = Video::create(settings["Video/Driver"].text());
  video->setContext(presentation->viewport.handle());
  video->setBlocking(settings["Video/Synchronize"].boolean());
  if(!video->ready()) MessageDialog().setText("Failed to initialize video driver").warning();
  presentation->clearViewport();

  audio = Audio::create(settings["Audio/Driver"].text());
  audio->setExclusive(settings["Audio/Exclusive"].boolean());
  audio->setContext(presentation->viewport.handle());
  audio->setDevice(settings["Audio/Device"].text());
  audio->setBlocking(settings["Audio/Synchronize"].boolean());
  audio->setChannels(2);
  if(!audio->ready()) MessageDialog().setText("Failed to initialize audio driver").warning();

  input = Input::create(settings["Input/Driver"].text());
  input->setContext(presentation->viewport.handle());
  input->onChange({&InputManager::onChange, &inputManager()});
  if(!input->ready()) MessageDialog().setText("Failed to initialize input driver").warning();

  new InputManager;
  new Theme;
  new Graphics;
  new Sound;
  new Controls;

  new Home;

  updateVideoShader();
  updateAudioDriver();
  updateAudioEffects();

  args.takeLeft();  //ignore program location in argument parsing
  for(auto& argument : args) {
    if(argument == "--fullscreen") {
      presentation->toggleFullScreen();
    } else if(directory::exists(argument.split("|", 1L).right())) {
      if(!argument.transform("\\", "/").endsWith("/")) argument.append("/");
      mediumQueue.append(argument);
    } else if(file::exists(argument)) {
      if(auto result = execute("cart-pal", "--import", argument)) {
        mediumQueue.append(result.output.strip());
      }
    }
  }
  if(mediumQueue) loadMedium();
  else            home->reset();

  Application::onMain({&Program::main, this});
}

auto Program::main() -> void {
  inputManager->poll();
  inputManager->pollHotkeys();

  if(!emulator) return home->run();

  if(!emulator->loaded() || pause || (!focused() && settings["Input/FocusLoss/Pause"].boolean())) {
    audio->clear();
    usleep(20 * 1000);
    return;
  }

  emulator->run();
}

auto Program::quit() -> void {
  hasQuit = true;
  unloadMedium();
  settings.quit();
  inputManager->quit();
  video.reset();
  audio.reset();
  input.reset();
  Application::quit();
}
