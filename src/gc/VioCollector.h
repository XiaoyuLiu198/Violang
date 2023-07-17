/**
 * Garbage Collector.
 */

#ifndef VioCollector_h
#define VioCollector_h

/**
 * Garbage collector implementing Mark-Sweep algorithm.
 */
struct VioCollector {
  /**
   * Main collection cycle.
   */
  void gc(const std::set<Traceable *> &roots) {
    mark(roots);
    sweep()
  }

  /**
   * Marking phase (trace).
   */
  void mark(const std::set<Traceable *> &roots) {
    std::vector<Traceable*>worklist(roots.begin(), roots.end());

    while (!worklist.empty()) {
      auto object = worklist.back();
      worklist.pop_back();

      if (!object->marked) {
        object->marked = true;
        for (auto &p: getPointers(object)) {
          worklist.push_back(p);
        }
      }
    }
  }

  /**
   * Returns all pointers within this object.
   */
  std::set<Traceable *> getPointers(const Traceable *object) {
    std::set<Traceable*> pointers;

    auto vioValue = OBJECT((Object*)object);
    if (IS_FUNCTION(vioValue)) {
      auto fn = AS_FUNCTION(vioValue);
      for (auto &cell: fn->cells) {
        pointers.insert((Traceable*)cell);
      }
    }
  }

  /**
   * Sweep phase (reclaim).
   */
  void sweep() {
    auto it = Traceable::objects.begin();
    while (it != Traceable::objects.end()) {
      auto object = (Traceable*)*it;
      if (object->marked) {
        object->marked=false; //for future collection cycle
        ++it;
      } else {
        it = Traceable::objects.erase(it);
        delete object;
      }
   }
  }
};

#endif