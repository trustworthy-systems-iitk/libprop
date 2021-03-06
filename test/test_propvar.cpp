#include <gtest/gtest.h>

#include "formula.h"
#include "parse_util.h"
#include "trace.h"

using namespace HyperPLTL;

TEST(PropertyLibTest, ValidTracePropVar) {
  std::string propstr = "(G+ (IMPLIES x.0 x.1))";
  PVarMap varmap = std::make_shared<VarMap>();
  varmap->addPropVar("x");

  PHyperProp property = parse_formula(propstr, varmap);

  PTrace trace1(new Trace(1, 0));
  PTrace trace2(new Trace(1, 0));

  TraceList tracelist({trace1, trace2});
  bool result = false;

  unsigned xid = property->getPropId("x");

  for (size_t cycle = 0; cycle < 10; ++cycle) {
    bool bval1 = rand() % 2;
    bool bval2 = bval1 ? 1 : (rand() % 2);
    trace1->updatePropValue(xid, cycle, bval1);
    trace2->updatePropValue(xid, cycle, bval2);
    result = property->eval(cycle, tracelist);
  }

  EXPECT_TRUE(result);
}

TEST(PropertyLibTest, InvalidTracePropVar) {
  std::string propstr = "(G+ (IMPLIES x.0 x.1))";
  PVarMap varmap = std::make_shared<VarMap>();
  varmap->addPropVar("x");

  PHyperProp property = parse_formula(propstr, varmap);

  PTrace trace1(new Trace(1, 0));
  PTrace trace2(new Trace(1, 0));

  TraceList tracelist({trace1, trace2});
  bool result = true;

  size_t cycle = 0;
  bool bval1, bval2;

  for (; cycle < 10; ++cycle) {
    bval1 = rand() % 2;
    bval2 = bval1 ? 1 : (rand() % 2);
    trace1->updatePropValue(0, cycle, bval1);
    trace2->updatePropValue(0, cycle, bval2);
    result = property->eval(cycle, tracelist);
  }

  // property violation
  trace1->updatePropValue(0, cycle, true);
  trace2->updatePropValue(0, cycle, false);
  result = property->eval(cycle, tracelist);
  cycle += 1;

  for (; cycle < 20; ++cycle) {
    bval1 = rand() % 2;
    bval2 = bval1 ? 1 : (rand() % 2);
    trace1->updatePropValue(0, cycle, bval1);
    trace2->updatePropValue(0, cycle, bval2);
    result = property->eval(cycle, tracelist);
  }

  EXPECT_FALSE(result);
}
