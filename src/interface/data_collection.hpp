//========================================================================================
// (C) (or copyright) 2020-2023. Triad National Security, LLC. All rights reserved.
//
// This program was produced under U.S. Government contract 89233218CNA000001 for Los
// Alamos National Laboratory (LANL), which is operated by Triad National Security, LLC
// for the U.S. Department of Energy/National Nuclear Security Administration. All rights
// in the program are reserved by Triad National Security, LLC, and the U.S. Department
// of Energy/National Nuclear Security Administration. The Government is granted for
// itself and others acting on its behalf a nonexclusive, paid-up, irrevocable worldwide
// license in this material to reproduce, prepare derivative works, distribute copies to
// the public, perform publicly and display publicly, and to permit others to do so.
//========================================================================================
#ifndef INTERFACE_DATA_COLLECTION_HPP_
#define INTERFACE_DATA_COLLECTION_HPP_

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace parthenon {
class Mesh;

/// The DataCollection class is an abstract container that contains at least a
/// "base" container of some type (e.g., of MeshData or MeshBlockData) plus
/// additional containers identified by string labels.
/// Current usage includes (but is not limited to) storing MeshBlockData for different
/// stages in multi-stage drivers or the corresponding MeshBlockPacks in a
/// DataCollection of MeshData.
template <typename T>
class DataCollection {
 public:
  DataCollection() {
    containers_["base"] = std::make_shared<T>(); // always add "base" container
    pmy_mesh_ = nullptr;
  }

  void SetMeshPointer(Mesh *pmesh) { pmy_mesh_ = pmesh; }

  // Full versions: add a MeshData w/given name, fields from 'src' selected by field_names
  std::shared_ptr<T> &Add(const std::string &name, const std::shared_ptr<T> &src,
                          const std::vector<std::string> &field_names = {},
                          const bool shallow = false);
  std::shared_ptr<T> &AddShallow(const std::string &name, const std::shared_ptr<T> &src,
                                 const std::vector<std::string> &field_names = {});
  // Allow naming src rather than passing pointer
  std::shared_ptr<T> &Add(const std::string &name, const std::string src_name,
                          const std::vector<std::string> &field_names = {});
  std::shared_ptr<T> &AddShallow(const std::string &name, const std::string src_name,
                                 const std::vector<std::string> &field_names = {});
  // Allow omitting the source name entirely, defaulting to "base"
  std::shared_ptr<T> &Add(const std::string &name,
                          const std::vector<std::string> &field_names = {});
  std::shared_ptr<T> &AddShallow(const std::string &name,
                                 const std::vector<std::string> &field_names = {});

  auto &Stages() { return containers_; }
  const auto &Stages() const { return containers_; }

  std::shared_ptr<T> &Get() { return containers_.at("base"); }
  const std::shared_ptr<T> &Get() const { return containers_.at("base"); }
  std::shared_ptr<T> &Get(const std::string &label) {
    auto it = containers_.find(label);
    if (it == containers_.end()) {
      throw std::runtime_error("Container " + label + " does not exist in collection.");
    }
    return it->second;
  }

  void Set(const std::string &name, std::shared_ptr<T> &d) { containers_[name] = d; }

  std::shared_ptr<T> &GetOrAdd(const std::string &mbd_label, const int &partition_id);

  void PurgeNonBase() {
    auto c = containers_.begin();
    while (c != containers_.end()) {
      if (c->first != "base") {
        c = containers_.erase(c);
      } else {
        ++c;
      }
    }
  }

 private:
  Mesh *pmy_mesh_;
  std::map<std::string, std::shared_ptr<T>> containers_;
};

} // namespace parthenon

#endif // INTERFACE_DATA_COLLECTION_HPP_
