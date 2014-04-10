// abtest.py --test=test_fcmp.pnacl.ll --driver=test_fcmp_main.cpp --prefix=Subzero_ --output=test_fcmp

#include <stdio.h>
#include <float.h>
#include <math.h>
#include <assert.h>

extern "C" {

bool fcmpFalseFloat(float a, float b);
bool fcmpOeqFloat(float a, float b);
bool fcmpOgtFloat(float a, float b);
bool fcmpOgeFloat(float a, float b);
bool fcmpOltFloat(float a, float b);
bool fcmpOleFloat(float a, float b);
bool fcmpOneFloat(float a, float b);
bool fcmpOrdFloat(float a, float b);
bool fcmpUeqFloat(float a, float b);
bool fcmpUgtFloat(float a, float b);
bool fcmpUgeFloat(float a, float b);
bool fcmpUltFloat(float a, float b);
bool fcmpUleFloat(float a, float b);
bool fcmpUneFloat(float a, float b);
bool fcmpUnoFloat(float a, float b);
bool fcmpTrueFloat(float a, float b);

bool fcmpFalseDouble(double a, double b);
bool fcmpOeqDouble(double a, double b);
bool fcmpOgtDouble(double a, double b);
bool fcmpOgeDouble(double a, double b);
bool fcmpOltDouble(double a, double b);
bool fcmpOleDouble(double a, double b);
bool fcmpOneDouble(double a, double b);
bool fcmpOrdDouble(double a, double b);
bool fcmpUeqDouble(double a, double b);
bool fcmpUgtDouble(double a, double b);
bool fcmpUgeDouble(double a, double b);
bool fcmpUltDouble(double a, double b);
bool fcmpUleDouble(double a, double b);
bool fcmpUneDouble(double a, double b);
bool fcmpUnoDouble(double a, double b);
bool fcmpTrueDouble(double a, double b);

bool Subzero_fcmpFalseFloat(float a, float b);
bool Subzero_fcmpOeqFloat(float a, float b);
bool Subzero_fcmpOgtFloat(float a, float b);
bool Subzero_fcmpOgeFloat(float a, float b);
bool Subzero_fcmpOltFloat(float a, float b);
bool Subzero_fcmpOleFloat(float a, float b);
bool Subzero_fcmpOneFloat(float a, float b);
bool Subzero_fcmpOrdFloat(float a, float b);
bool Subzero_fcmpUeqFloat(float a, float b);
bool Subzero_fcmpUgtFloat(float a, float b);
bool Subzero_fcmpUgeFloat(float a, float b);
bool Subzero_fcmpUltFloat(float a, float b);
bool Subzero_fcmpUleFloat(float a, float b);
bool Subzero_fcmpUneFloat(float a, float b);
bool Subzero_fcmpUnoFloat(float a, float b);
bool Subzero_fcmpTrueFloat(float a, float b);

bool Subzero_fcmpFalseDouble(double a, double b);
bool Subzero_fcmpOeqDouble(double a, double b);
bool Subzero_fcmpOgtDouble(double a, double b);
bool Subzero_fcmpOgeDouble(double a, double b);
bool Subzero_fcmpOltDouble(double a, double b);
bool Subzero_fcmpOleDouble(double a, double b);
bool Subzero_fcmpOneDouble(double a, double b);
bool Subzero_fcmpOrdDouble(double a, double b);
bool Subzero_fcmpUeqDouble(double a, double b);
bool Subzero_fcmpUgtDouble(double a, double b);
bool Subzero_fcmpUgeDouble(double a, double b);
bool Subzero_fcmpUltDouble(double a, double b);
bool Subzero_fcmpUleDouble(double a, double b);
bool Subzero_fcmpUneDouble(double a, double b);
bool Subzero_fcmpUnoDouble(double a, double b);
bool Subzero_fcmpTrueDouble(double a, double b);
}

int main(int argc, char **argv) {
  static const float FloatNegInf = -1.0 / 0.0;
  static const float FloatZero = 0.0;
  static const float FloatTen = 10.0;
  static const float FloatPosInf = 1.0 / 0.0;
  static const float FloatNan = 0.0 / 0.0;
  assert(fpclassify(FloatNegInf) == FP_INFINITE);
  assert(fpclassify(FloatPosInf) == FP_INFINITE);
  assert(fpclassify(FloatNan) == FP_NAN);
  assert(FloatNegInf < FloatZero);
  assert(FloatNegInf < FloatPosInf);
  assert(FloatZero < FloatPosInf);

  static const double DoubleNegInf = -1.0 / 0.0;
  static const double DoubleZero = 0.0;
  static const double DoubleTen = 10.0;
  static const double DoublePosInf = 1.0 / 0.0;
  static const double DoubleNan = 0.0 / 0.0;
  assert(fpclassify(DoubleNegInf) == FP_INFINITE);
  assert(fpclassify(DoublePosInf) == FP_INFINITE);
  assert(fpclassify(DoubleNan) == FP_NAN);
  assert(DoubleNegInf < DoubleZero);
  assert(DoubleNegInf < DoublePosInf);
  assert(DoubleZero < DoublePosInf);

  static float FloatValues[] = { FloatNegInf, FloatZero,   FLT_MIN,  FloatTen,
                                 FLT_MAX,     FloatPosInf, FloatNan, };
  const static unsigned NumFloatValues =
      sizeof(FloatValues) / sizeof(*FloatValues);

  static double DoubleValues[] = { DoubleNegInf, DoubleZero, DBL_MIN,
                                   DoubleTen,    DBL_MAX,    DoublePosInf,
                                   DoubleNan, };
  const static unsigned NumDoubleValues =
      sizeof(DoubleValues) / sizeof(*DoubleValues);

  typedef bool (*FuncTypeFloat)(float, float);
  static struct {
    const char *Name;
    FuncTypeFloat FuncSz;
    FuncTypeFloat FuncLlc;
  } FuncFloat[] = {
      { "fcmpFalseFloat", Subzero_fcmpFalseFloat, fcmpFalseFloat },
      { "fcmpOeqFloat", Subzero_fcmpOeqFloat, fcmpOeqFloat },
      { "fcmpOgtFloat", Subzero_fcmpOgtFloat, fcmpOgtFloat },
      { "fcmpOgeFloat", Subzero_fcmpOgeFloat, fcmpOgeFloat },
      { "fcmpOltFloat", Subzero_fcmpOltFloat, fcmpOltFloat },
      { "fcmpOleFloat", Subzero_fcmpOleFloat, fcmpOleFloat },
      { "fcmpOneFloat", Subzero_fcmpOneFloat, fcmpOneFloat },
      { "fcmpOrdFloat", Subzero_fcmpOrdFloat, fcmpOrdFloat },
      { "fcmpUeqFloat", Subzero_fcmpUeqFloat, fcmpUeqFloat },
      { "fcmpUgtFloat", Subzero_fcmpUgtFloat, fcmpUgtFloat },
      { "fcmpUgeFloat", Subzero_fcmpUgeFloat, fcmpUgeFloat },
      { "fcmpUltFloat", Subzero_fcmpUltFloat, fcmpUltFloat },
      { "fcmpUleFloat", Subzero_fcmpUleFloat, fcmpUleFloat },
      { "fcmpUneFloat", Subzero_fcmpUneFloat, fcmpUneFloat },
      { "fcmpUnoFloat", Subzero_fcmpUnoFloat, fcmpUnoFloat },
      { "fcmpTrueFloat", Subzero_fcmpTrueFloat, fcmpTrueFloat },
    };
  const static unsigned NumFuncFloat = sizeof(FuncFloat) / sizeof(*FuncFloat);

  typedef bool (*FuncTypeDouble)(double, double);
  static struct {
    const char *Name;
    FuncTypeDouble FuncSz;
    FuncTypeDouble FuncLlc;
  } FuncDouble[] = {
      { "fcmpFalseDouble", Subzero_fcmpFalseDouble, fcmpFalseDouble },
      { "fcmpOeqDouble", Subzero_fcmpOeqDouble, fcmpOeqDouble },
      { "fcmpOgtDouble", Subzero_fcmpOgtDouble, fcmpOgtDouble },
      { "fcmpOgeDouble", Subzero_fcmpOgeDouble, fcmpOgeDouble },
      { "fcmpOltDouble", Subzero_fcmpOltDouble, fcmpOltDouble },
      { "fcmpOleDouble", Subzero_fcmpOleDouble, fcmpOleDouble },
      { "fcmpOneDouble", Subzero_fcmpOneDouble, fcmpOneDouble },
      { "fcmpOrdDouble", Subzero_fcmpOrdDouble, fcmpOrdDouble },
      { "fcmpUeqDouble", Subzero_fcmpUeqDouble, fcmpUeqDouble },
      { "fcmpUgtDouble", Subzero_fcmpUgtDouble, fcmpUgtDouble },
      { "fcmpUgeDouble", Subzero_fcmpUgeDouble, fcmpUgeDouble },
      { "fcmpUltDouble", Subzero_fcmpUltDouble, fcmpUltDouble },
      { "fcmpUleDouble", Subzero_fcmpUleDouble, fcmpUleDouble },
      { "fcmpUneDouble", Subzero_fcmpUneDouble, fcmpUneDouble },
      { "fcmpUnoDouble", Subzero_fcmpUnoDouble, fcmpUnoDouble },
      { "fcmpTrueDouble", Subzero_fcmpTrueDouble, fcmpTrueDouble },
    };
  const static unsigned NumFuncDouble =
      sizeof(FuncDouble) / sizeof(*FuncDouble);

  bool ResultSz, ResultLlc;

  unsigned TotalTests = 0;
  unsigned Passes = 0;
  unsigned Failures = 0;

  for (unsigned f = 0; f < NumFuncFloat; ++f) {
    for (unsigned i = 0; i < NumFloatValues; ++i) {
      for (unsigned j = 0; j < NumFloatValues; ++j) {
        ++TotalTests;
        ResultSz = FuncFloat[f].FuncSz(FloatValues[i], FloatValues[j]);
        ResultLlc = FuncFloat[f].FuncLlc(FloatValues[i], FloatValues[j]);
        if (ResultSz == ResultLlc) {
          ++Passes;
        } else {
          ++Failures;
          printf("%s(%g, %g): sz=%d llc=%d\n", FuncFloat[f].Name,
                 FloatValues[i], FloatValues[j], ResultSz, ResultLlc);
        }
      }
    }
  }

  for (unsigned f = 0; f < NumFuncDouble; ++f) {
    for (unsigned i = 0; i < NumDoubleValues; ++i) {
      for (unsigned j = 0; j < NumDoubleValues; ++j) {
        ++TotalTests;
        ResultSz = FuncDouble[f].FuncSz(DoubleValues[i], DoubleValues[j]);
        ResultLlc = FuncDouble[f].FuncLlc(DoubleValues[i], DoubleValues[j]);
        if (ResultSz == ResultLlc) {
          ++Passes;
        } else {
          ++Failures;
          printf("%s(%g, %g): sz=%d llc=%d\n", FuncDouble[f].Name,
                 DoubleValues[i], DoubleValues[j], ResultSz, ResultLlc);
        }
      }
    }
  }

  printf("TotalTests=%u Passes=%u Failures=%u\n", TotalTests, Passes, Failures);
  return Failures;
}
