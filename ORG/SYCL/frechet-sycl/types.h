#ifndef TYPES_H
#define TYPES_H

// Precision switch for Intel Arc A770 (no FP64 support)
#ifndef USE_FLOAT
#define DOUBLE double
#else
#define DOUBLE float
#endif

#endif // TYPES_H
