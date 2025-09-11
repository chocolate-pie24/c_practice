#ifndef DEFINE_H
#define DEFINE_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __clang__
  #define NO_COVERAGE __attribute__((no_profile_instrument_function))
#else
  #define NO_COVERAGE
#endif

#ifdef __cplusplus
}
#endif
#endif
