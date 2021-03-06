struct Settings : Markup::Node {
  Settings();
  ~Settings();
};

Settings::Settings() {
  Markup::Node::operator=(BML::unserialize(string::read(locate("settings.bml"))));

  auto set = [&](const string& name, const string& value) {
    //create node and set to default value only if it does not already exist
    if(!operator[](name)) operator()(name).setValue(value);
  };

  set("Library/Location", {Path::user(), "Emulation/"});

  set("cart-pal/Path", Path::user());
  set("cart-pal/CreateManifests", false);
  set("cart-pal/UseDatabase", true);
  set("cart-pal/UseHeuristics", true);
}

Settings::~Settings() {
  file::write(locate("settings.bml"), BML::serialize(*this));
}
