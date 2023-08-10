/**
 * Global object.
 */

#ifndef Global_h
#define Global_h

/**
 * Global var.
 */
struct GlobalVar {
  std::string name;
  VioValue value;
};

/**
 * Global object.
 */
struct Global {
  /**
   * Global variables and functions.
   */
  std::vector<GlobalVar> globals;
  /**
   * Returns a global.
   */
  GlobalVar& get(size_t index) { return globals[index]; }

  /**
   * Sets a global.
   */
  void set(size_t index, const VioValue& value) {
    if (index >= globals.size()) {
      DIE << "Global " << index << "doesn't exist.";
    }
    globals[index].value = value;
  }

  /**
   * Adds a global constant.
   */
  void addGlobal(const std::string& name, double value) {
    if (exists(name)) {
      return;
    }
    globals.push_back({name, NUMBER(value)});
    // (GlobalVar)
  }

  /**
   * Get global index.
   */
  int getGlobalIndex(const std::string& name) {
    if (globals.size() > 0) {
      for (auto i = (int)globals.size() - 1; i >= 0; i --)
      {
        if (globals[i].name == name) {
          return i;
        }
      }
    }
    return -1;
  }

  /**
   * Whether a global variable exists.
   */
  bool exists(const std::string& name) { return getGlobalIndex(name) != -1; }

    /**
   * Registers a global.
   */
  void define(const std::string& name) {
    auto index = getGlobalIndex(name);

    if (index != -1){
      return;
    }

    // Set to default number
    globals.push_back((GlobalVar){name, NUMBER(0)});
  }

    /**
   * Adds a native function.
   */
  void addNativeFunction(const std::string& name, std::function<void()> fn,
                         size_t arity) {
    if (exists(name)) {
      return;
    }
    // (GlobalVar)
    globals.push_back({name, ALLOC_NATIVE(fn, name, arity)});
  }

};

#endif