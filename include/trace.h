#ifndef __TRACE_H_DEFINED__
#define __TRACE_H_DEFINED__

#include <algorithm>
#include <cassert>
#include <memory>
#include <variant>
#include <vector>

class Trace;
typedef std::shared_ptr<Trace> PTrace;
typedef std::vector<PTrace> TraceList;

template <class T>
struct VarTrace {
  /// DataPoint stores the value of a signal at a particular time index.
  using DataPoint = std::pair<uint32_t, T>;
  std::vector<DataPoint> datapoints;

  /// time when the last addition was performed.
  uint32_t lastCycle;

 public:
  VarTrace() : lastCycle(0) {}

  /**
   * updateValue(t, value) must be called every cycle.
   * It will insert into the vector if needed.
   */
  void updateValue(uint32_t time, const T& v) {
    if (datapoints.size() == 0) {
      assert(time == 0);
      datapoints.push_back(DataPoint(time, v));
    } else {
      // must update a time index only once.
      assert(time > lastCycle);

      // find the last index into the array.
      size_t last = datapoints.size() - 1;
      assert(last <= time);

      // check if we need to add.g
      if (datapoints[last].second != v) {
        std::pair<uint32_t, T> tempdata(time, v);
        datapoints.emplace_back(tempdata);
      }  // else nothing to do.
    }

    lastCycle = time;
  }

  /** Extends the trace to the specified number of cycles. */
  void extendToCycle(uint32_t cycle) {
    assert(cycle >= lastCycle);
    lastCycle = cycle;
  }

  /// Return the element at a particular index.
  const T operator[](uint32_t cycle) {
    assert(datapoints.size() > 0);

    auto lowerCmp = [](const std::pair<unsigned int, T>& cv, const unsigned int v) {
      return cv.first < v;
    };
    auto upperCmp = [](const unsigned int v, const std::pair<unsigned int, T>& cv) {
      return v < cv.first;
    };

    auto lower = std::lower_bound(datapoints.begin(), datapoints.end(), cycle, lowerCmp);
    assert((lower - datapoints.begin()) >= 0);
    auto upper = std::upper_bound(datapoints.begin(), datapoints.end(), cycle, upperCmp);

    if (lower == upper)
      return (lower - 1)->second;
    else
      return lower->second;
  }

  size_t dimension() const { return datapoints.size(); }

  bool operator!=(VarTrace<T> const& other) const {
    return !(datapoints == other.datapoints);
  }
  bool operator==(VarTrace<T> const& other) const {
    return datapoints == other.datapoints;
  }
};

using ValueType = std::variant<uint32_t, std::vector<uint32_t>>;
using TraceType = std::variant<VarTrace<uint32_t>, VarTrace<std::vector<uint32_t>>>;

class Trace {
  /** A vector of traces for each propositional variable. */
  std::vector<VarTrace<bool>> propositions;

  //
  // TODO : https://github.com/skmuduli92/libprop/issues/2#issue-612886338
  //  + instead of making ValueType a std::variant
  //    make VarTrace a variant, this will fix non-homogeneity issue
  //
  //  + write visitor methods to process serialization nicely
  //

  /** A vector of traces for each term variable. */
  // std::vector<VarTrace<ValueType>> variables;
  std::vector<TraceType> variables;

  /** The last valid time cycle in this trace. */
  uint32_t lastCycle;

 public:
  /** Create a trace capable of storing numVars variables and
      numProps propositions. */
  Trace(unsigned numProps, unsigned numVars)
      : propositions(numProps), variables(numVars), lastCycle(0) {}

  /** Return the number of propositional variables in the trace. */
  unsigned numProps() const { return propositions.size(); }

  /** Return the number of term (numeric) variables in the trace. */
  unsigned numVars() const { return variables.size(); }

  /** Update the value of variable i at time cycle. */
  void updateTermValue(unsigned i, uint32_t cycle, ValueType value) {
    assert(i < variables.size());
    if (lastCycle < cycle) {
      lastCycle = cycle;
    }

    std::visit(VariantTraceWriter{cycle, variables[i]}, value);
  }

  /** Update the value of proposition i at time cycle. */
  void updatePropValue(unsigned i, uint32_t cycle, bool value) {
    assert(i < propositions.size());
    if (lastCycle < cycle) {
      lastCycle = cycle;
    }
    propositions[i].updateValue(cycle, value);
  }

  /** Return the value of variable i at time cycle. */
  ValueType termValueAt(unsigned i, uint32_t cycle) {
    assert(i < variables.size());
    return std::visit(VariantTraceReader{cycle}, variables[i]);
  }

  /** Return the value of a proposition i at time cycle. */
  bool propValueAt(unsigned i, uint32_t cycle) {
    assert(i < propositions.size());
    return propositions[i][cycle];
  }

  void extendToCycle(uint32_t cycle) {
    assert(cycle >= lastCycle);
    lastCycle = cycle;
    for (auto p : propositions) {
      p.extendToCycle(cycle);
    }
    for (auto v : variables) {
      // v.extendToCycle(cycle);
      std::visit(VariantExtendCycle{cycle}, v);
    }
  }

  /// get trace length (un-compressed)
  size_t length(void) const { return 1 + lastCycle; }

  // utility function to store a trace object in binary format for
  bool operator==(Trace const& other) const {

    if (numProps() != other.numProps()) return false;
    if (numVars() != other.numVars()) return false;
    if (length() != other.length()) return false;

    for (size_t pi = 0; pi < numProps(); ++pi) {
      if (propositions[pi] != other.propositions[pi]) return false;
    }

    for (size_t vi = 0; vi < numVars(); ++vi) {
      if (variables[vi] != other.variables[vi]) return false;
    }

    return true;
  }

  bool operator!=(Trace const& other) const { return !(*this == other); }

 public:
  // visitors for std::variant, executes appropriate logic based on the active type

  struct VariantTraceWriter {
    uint32_t time;
    TraceType& tr;

    void operator()(uint32_t& val) {
      if (time == 0) {
        tr = VarTrace<uint32_t>();
      }

      VarTrace<uint32_t>& varref = std::get<VarTrace<uint32_t>>(tr);
      varref.updateValue(time, val);
    }

    void operator()(std::vector<uint32_t>& val) {
      if (time == 0) {
        tr = VarTrace<std::vector<uint32_t>>();
      }

      VarTrace<std::vector<uint32_t>>& varref =
          std::get<VarTrace<std::vector<uint32_t>>>(tr);

      varref.updateValue(time, val);
    }
  };

  struct VariantTraceReader {
    uint32_t time;

    ValueType operator()(VarTrace<uint32_t>& vt) const { return vt[time]; }
    ValueType operator()(VarTrace<std::vector<uint32_t>>& vt) const { return vt[time]; }
  };

  struct VariantExtendCycle {
    uint32_t cycle;

    void operator()(VarTrace<uint32_t>& vt) const { vt.extendToCycle(cycle); }
    void operator()(VarTrace<std::vector<uint32_t>>& vt) const {
      vt.extendToCycle(cycle);
    }
  };

 private:
  friend class TraceSerialize;
};

class TraceSerialize {

  // storing data in compressed format

  struct ElemCounter {

    // FIXME : fix the logic of vec length
    size_t operator()([[maybe_unused]] VarTrace<uint32_t>& tv) { return 1; }
    size_t operator()(VarTrace<std::vector<uint32_t>>& tv) { return tv.dimension(); }
  };

 public:
  static size_t getByteSize(PTrace trace);
  static size_t store(uint8_t* dest, PTrace trace);
  static PTrace load(uint8_t* source);
};

#endif
