#ifndef DIAMOND_TYPES_H
#define DIAMOND_TYPES_H

// Precision switch for Intel Arc A770 (no FP64 support)
#ifndef USE_FLOAT
#define DOUBLE double
#else
#define DOUBLE float
#endif

#endif // DIAMOND_TYPES_H
