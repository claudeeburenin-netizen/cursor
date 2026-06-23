#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>

struct StructA {
  explicit StructA(std::string name) : name(std::move(name)) {}

  std::string name;
};

using WeakStructMap = std::map<std::string, std::weak_ptr<StructA>>;
using StructOwners = std::map<std::string, std::shared_ptr<StructA>>;

int main() {
  {
    WeakStructMap weak_map;

    weak_map["temporary"] = std::make_shared<StructA>("temporary");

    // The temporary shared_ptr is destroyed at the end of the assignment
    // expression, so weak_ptr has no owner to lock.
    assert(weak_map.at("temporary").expired());
  }

  {
    WeakStructMap weak_map;
    StructOwners owners;

    owners["persistent"] = std::make_shared<StructA>("persistent");
    weak_map["persistent"] = owners.at("persistent");

    auto value = weak_map.at("persistent").lock();
    assert(value);
    assert(value->name == "persistent");

    owners.clear();
    assert(weak_map.at("persistent").expired());
  }

  return 0;
}
