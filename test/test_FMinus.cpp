
#include <gtest/gtest.h>

#include "formula.h"
#include "parse_util.h"
#include "formula_util.h"
#include "trace.h"

#include "formula_util.h"

using namespace HyperPLTL;
using namespace std;

PHyperProp propertyOnceMinusOperator() {
  PVarMap varmap = std::make_shared<VarMap>();
  varmap->addIntVar("x");
  varmap->addIntVar("y");
  std::string formula = "(F- (AND (EQ x) (EQ y)))";
  return parse_formula(formula, varmap);
}

TEST(PropertyLibTest, ValidTraceOnceMinusOperator_Simple) {

  PVarMap varmap = std::make_shared<VarMap>();
  unsigned xid = varmap->addIntVar("x");
  PHyperProp property = parse_formula("(F- (EQ x))", varmap);

  PTrace trace1(new Trace(0, 1));
  PTrace trace2(new Trace(0, 1));
  TraceList tracelist({trace1, trace2});

  bool result = false;
  unsigned traceLength = rand() % 20 + 20;
  size_t cycle = 0;

  for (cycle = 0; cycle < traceLength; cycle++) {
    if(cycle == 10){
      trace1->updateTermValue(xid, cycle, 10);
      trace2->updateTermValue(xid, cycle, 10);
      continue;
    }
    trace1->updateTermValue(xid, cycle, rand() % std::numeric_limits<unsigned>::max());
    trace2->updateTermValue(xid, cycle, rand() % std::numeric_limits<unsigned>::max());
  }

  result = evaluateTraces(property, tracelist);

  EXPECT_TRUE(result);
}

TEST(PropertyLibTest, InValidTraceOnceMinusOperator_Simple) {

  PVarMap varmap = std::make_shared<VarMap>();
  unsigned xid = varmap->addIntVar("x");
  PHyperProp property = parse_formula("(F- (EQ x))", varmap);

  PTrace trace1(new Trace(0, 1));
  PTrace trace2(new Trace(0, 1));
  TraceList tracelist({trace1, trace2});

  bool result = false;
  unsigned traceLength = rand() % 20 + 20;
  size_t cycle = 0;

  for (cycle = 0; cycle < traceLength; cycle++) {
    unsigned xvalue = rand() % std::numeric_limits<unsigned>::max();
    trace1->updateTermValue(xid, cycle, xvalue);
    trace2->updateTermValue(xid, cycle, !xvalue);
  }

  result = evaluateTraces(property, tracelist);

  EXPECT_FALSE(result);
}


TEST(PropertyLibTest, ValidTraceOnceMinusOperator) {
  PHyperProp property = propertyOnceMinusOperator();

  PTrace trace1(new Trace(0, 2));
  PTrace trace2(new Trace(0, 2));
  TraceList tracelist({trace1, trace2});
  unsigned xid = property->getVarId("x");
  unsigned yid = property->getVarId("y");

  bool result = false;
  unsigned traceLength = rand() % 20 + 20;

  size_t cycle = 0;
  for (; cycle < traceLength; ++cycle) {
    unsigned xvalue = rand() % 100;
    // setting 'x' var value
    trace1->updateTermValue(xid, cycle, xvalue);
    trace2->updateTermValue(xid, cycle, !xvalue);
    // setting 'y' var value

    unsigned yvalue = rand() % 100;
    trace1->updateTermValue(yid, cycle, yvalue);
    trace2->updateTermValue(yid, cycle, !yvalue);
  }

  trace1->updateTermValue(xid, cycle, 10);
  trace2->updateTermValue(xid, cycle, 10);
  trace1->updateTermValue(yid, cycle, 11);
  trace2->updateTermValue(yid, cycle, 11);
  cycle = cycle + 1;

  traceLength = rand() % 20 + 20 + cycle;
  for (; cycle < traceLength; ++cycle) {
    unsigned xvalue = rand() % 100;
    // setting 'x' var value
    trace1->updateTermValue(xid, cycle, xvalue);
    trace2->updateTermValue(xid, cycle, !xvalue);
    // setting 'y' var value

    unsigned yvalue = rand() % 100;
    trace1->updateTermValue(yid, cycle, yvalue);
    trace2->updateTermValue(yid, cycle, !yvalue);
  }

  result = evaluateTraces(property, tracelist);

  EXPECT_TRUE(result);
}

TEST(PropertyLibTest, InvalidTraceOnceMinusOperator) {
  PHyperProp property = propertyOnceMinusOperator();

  PTrace trace1(new Trace(0, 2));
  PTrace trace2(new Trace(0, 2));
  TraceList tracelist({trace1, trace2});
  unsigned xid = property->getVarId("x");
  unsigned yid = property->getVarId("y");

  bool result = false;
  unsigned traceLength = rand() % 20 + 20;

  size_t cycle = 0;
  for (; cycle < traceLength; ++cycle) {
    unsigned xvalue = rand() % 100;
    // setting 'x' var value
    trace1->updateTermValue(xid, cycle, xvalue);
    trace2->updateTermValue(xid, cycle, !xvalue);
    // setting 'y' var value

    unsigned yvalue = rand() % 100;
    trace1->updateTermValue(yid, cycle, yvalue);
    trace2->updateTermValue(yid, cycle, yvalue);
  }

  traceLength = rand() % 20 + 20 + cycle;
  for (; cycle < traceLength; ++cycle) {
    unsigned xvalue = rand() % 100;
    // setting 'x' var value
    trace1->updateTermValue(xid, cycle, xvalue);
    trace2->updateTermValue(xid, cycle, xvalue);
    // setting 'y' var value

    unsigned yvalue = rand() % 100;
    trace1->updateTermValue(yid, cycle, yvalue);
    trace2->updateTermValue(yid, cycle, !yvalue);
  }

  result = evaluateTraces(property, tracelist);

  EXPECT_FALSE(result);
}

TEST(PropertyLibTest, ValidTraceOperator_XMinus_FMinus) {
  PVarMap varmap = std::make_shared<VarMap>();
  unsigned xid = varmap->addIntVar("x");
  unsigned yid = varmap->addIntVar("y");
  std::string formula = "(F- (IMPLIES (EQ x) (X+ (EQ y))))";
  auto property = parse_formula(formula, varmap);

  PTrace trace1(new Trace(0, 2));
  PTrace trace2(new Trace(0, 2));
  TraceList tracelist({trace1, trace2});

  unsigned xvalue = 0, yvalue = 0;
  long cycle = 0;

  for (; cycle < 10; ++cycle) {
    xvalue = rand() % std::numeric_limits<unsigned>::max();
    trace1->updateTermValue(xid, cycle, xvalue);
    trace2->updateTermValue(xid, cycle, xvalue);

    yvalue = rand() % std::numeric_limits<unsigned>::max();
    trace1->updateTermValue(yid, cycle, yvalue);
    yvalue = rand() % std::numeric_limits<unsigned>::max();
    trace2->updateTermValue(yid, cycle, yvalue);
  }

  xvalue = rand() % std::numeric_limits<unsigned>::max();
  trace1->updateTermValue(xid, cycle, xvalue);
  trace2->updateTermValue(xid, cycle, xvalue);
  yvalue = rand() % std::numeric_limits<unsigned>::max();
  trace1->updateTermValue(yid, cycle, yvalue);
  yvalue = rand() % std::numeric_limits<unsigned>::max();
  trace2->updateTermValue(yid, cycle, yvalue);

  ++cycle;

  xvalue = rand() % std::numeric_limits<unsigned>::max();
  trace1->updateTermValue(xid, cycle, xvalue);
  xvalue = rand() % std::numeric_limits<unsigned>::max();
  trace2->updateTermValue(xid, cycle, xvalue);
  yvalue = rand() % std::numeric_limits<unsigned>::max();
  trace1->updateTermValue(yid, cycle, yvalue);
  trace2->updateTermValue(yid, cycle, yvalue);

  ++cycle;

  for (; cycle < 20; ++cycle) {
    xvalue = rand() % std::numeric_limits<unsigned>::max();
    trace1->updateTermValue(xid, cycle, xvalue);
    xvalue = rand() % std::numeric_limits<unsigned>::max();
    trace2->updateTermValue(xid, cycle, xvalue);

    yvalue = rand() % std::numeric_limits<unsigned>::max();
    trace1->updateTermValue(yid, cycle, yvalue);
    yvalue = rand() % std::numeric_limits<unsigned>::max();
    trace2->updateTermValue(yid, cycle, yvalue);
  }

  bool result = false;
  result = evaluateTraces(property, tracelist);

  ASSERT_TRUE(result);
}

TEST(PropertyLibTest, InvalidTraceOperator_XMinus_FMinus) {
   PVarMap varmap = std::make_shared<VarMap>();
  unsigned xid = varmap->addIntVar("x");
  unsigned yid = varmap->addIntVar("y");
  std::string formula = "(F- (IMPLIES (EQ x) (X- (EQ y))))";
  auto property = parse_formula(formula, varmap);

  PTrace trace1(new Trace(0, 2));
  PTrace trace2(new Trace(0, 2));
  TraceList tracelist({trace1, trace2});

  unsigned xvalue = 0, yvalue = 0;
  long cycle = 0;

  for (; cycle < 10; ++cycle) {
    xvalue = rand() % std::numeric_limits<unsigned>::max();
    trace1->updateTermValue(xid, cycle, xvalue);
    trace2->updateTermValue(xid, cycle, xvalue);

    yvalue = rand() % std::numeric_limits<unsigned>::max();
    trace1->updateTermValue(yid, cycle, yvalue);
    trace2->updateTermValue(yid, cycle, !yvalue);
  }

  bool result = false;
  result = evaluateTraces(property, tracelist);

  EXPECT_FALSE(result);
}
