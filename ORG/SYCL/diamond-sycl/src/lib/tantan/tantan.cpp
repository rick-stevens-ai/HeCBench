// Copyright 2010 Martin C. Frith

#include "tantan.hh"

#include <algorithm>  // fill, max
#include <cassert>
#include <cmath>  // pow, abs
#include <iostream>  // cerr
#include <numeric>  // accumulate
#include <vector>

#define BEG(v) ((v).empty() ? 0 : &(v).front())
#define END(v) ((v).empty() ? 0 : &(v).back() + 1)

namespace tantan {

void multiplyAll(std::vector<DOUBLE> &v, DOUBLE factor) {
  for (std::vector<DOUBLE>::iterator i = v.begin(); i < v.end(); ++i)
    *i *= factor;
}

DOUBLE firstRepeatOffsetProb(DOUBLE probMult, int maxRepeatOffset) {
  if (probMult < 1 || probMult > 1)
    return (1 - probMult) / (1 - std::pow(probMult, maxRepeatOffset));
  else
    return 1.0 / maxRepeatOffset;
}

void checkForwardAndBackwardTotals(DOUBLE fTot, DOUBLE bTot) {
  DOUBLE x = std::abs(fTot);
  DOUBLE y = std::abs(bTot);

  // ??? Is 1e6 suitable here ???
  if (std::abs(fTot - bTot) > std::max(x, y) / 1e6)
    std::cerr << "tantan: warning: possible numeric inaccuracy\n"
              << "tantan:          forward algorithm total: " << fTot << "\n"
              << "tantan:          backward algorithm total: " << bTot << "\n";
}

struct Tantan {
  enum { scaleStepSize = 16 };

  const uchar *seqBeg;  // start of the sequence
  const uchar *seqEnd;  // end of the sequence
  const uchar *seqPtr;  // current position in the sequence

  int maxRepeatOffset;

  const const_double_ptr *likelihoodRatioMatrix;

  DOUBLE b2b;  // transition probability from background to background
  DOUBLE f2b;  // transition probability from foreground to background
  DOUBLE g2g;  // transition probability from gap/indel to gap/indel
  //DOUBLE f2g;  // transition probability from foreground to gap/indel
  //DOUBLE g2f;  // transition probability from gap/indel to foreground
  DOUBLE oneGapProb;  // f2g * g2f
  DOUBLE endGapProb;  // f2g * 1
  DOUBLE f2f0;  // foreground to foreground, if there are 0 indel transitions
  DOUBLE f2f1;  // foreground to foreground, if there is 1 indel transition
  DOUBLE f2f2;  // foreground to foreground, if there are 2 indel transitions
  DOUBLE b2fDecay;
  DOUBLE b2fGrowth;
  DOUBLE b2fFirst;  // background state to first foreground state
  DOUBLE b2fLast;  // background state to last foreground state

  DOUBLE backgroundProb;
  std::vector<DOUBLE> foregroundProbs;
  std::vector<DOUBLE> insertionProbs;

  std::vector<DOUBLE> scaleFactors;

  Tantan(const uchar *seqBeg,
         const uchar *seqEnd,
         int maxRepeatOffset,
         const const_double_ptr *likelihoodRatioMatrix,
         DOUBLE repeatProb,
         DOUBLE repeatEndProb,
         DOUBLE repeatOffsetProbDecay,
         DOUBLE firstGapProb,
         DOUBLE otherGapProb) {
    assert(maxRepeatOffset > 0);
    assert(repeatProb >= 0 && repeatProb < 1);
    // (if repeatProb==1, then any sequence is impossible)
    assert(repeatEndProb >= 0 && repeatEndProb <= 1);
    assert(repeatOffsetProbDecay > 0 && repeatOffsetProbDecay <= 1);
    assert(otherGapProb >= 0 && otherGapProb <= 1);
    assert(firstGapProb >= 0);
    assert(repeatEndProb + firstGapProb * 2 <= 1);

    this->seqBeg = seqBeg;
    this->seqEnd = seqEnd;
    this->seqPtr = seqBeg;
    this->maxRepeatOffset = maxRepeatOffset;
    this->likelihoodRatioMatrix = likelihoodRatioMatrix;

    b2b = 1 - repeatProb;
    f2b = repeatEndProb;
    g2g = otherGapProb;
    //f2g = firstGapProb;
    //g2f = 1 - otherGapProb;
    oneGapProb = firstGapProb * (1 - otherGapProb);
    endGapProb = firstGapProb * 1;
    f2f0 = 1 - repeatEndProb;
    f2f1 = 1 - repeatEndProb - firstGapProb;
    f2f2 = 1 - repeatEndProb - firstGapProb * 2;

    b2fDecay = repeatOffsetProbDecay;
    b2fGrowth = 1 / repeatOffsetProbDecay;

    b2fFirst = repeatProb * firstRepeatOffsetProb(b2fDecay, maxRepeatOffset);
    b2fLast = repeatProb * firstRepeatOffsetProb(b2fGrowth, maxRepeatOffset);

    foregroundProbs.resize(maxRepeatOffset);
    insertionProbs.resize(maxRepeatOffset - 1);

    scaleFactors.resize((seqEnd - seqBeg) / scaleStepSize);
  }

  void initializeForwardAlgorithm() {
    backgroundProb = 1.0;
    std::fill(foregroundProbs.begin(), foregroundProbs.end(), 0.0);
    std::fill(insertionProbs.begin(), insertionProbs.end(), 0.0);
  }

  DOUBLE forwardTotal() {
    DOUBLE fromForeground = std::accumulate(foregroundProbs.begin(),
                                            foregroundProbs.end(), 0.0);
    fromForeground *= f2b;
    DOUBLE total = backgroundProb * b2b + fromForeground;
    assert(total > 0);
    return total;
  }

  void initializeBackwardAlgorithm() {
    backgroundProb = b2b;
    std::fill(foregroundProbs.begin(), foregroundProbs.end(), f2b);
    std::fill(insertionProbs.begin(), insertionProbs.end(), 0.0);
  }

  DOUBLE backwardTotal() {
    assert(backgroundProb > 0);
    return backgroundProb;
  }

  void calcForwardTransitionProbsWithGaps() {
    DOUBLE fromBackground = backgroundProb * b2fLast;
    DOUBLE *foregroundPtr = &foregroundProbs.back();
    DOUBLE f = *foregroundPtr;
    DOUBLE fromForeground = f;

    if (insertionProbs.empty()) {
      *foregroundPtr = fromBackground + f * f2f0;
    } else {
      DOUBLE *insertionPtr = &insertionProbs.back();
      DOUBLE i = *insertionPtr;
      *foregroundPtr = fromBackground + f * f2f1 + i * endGapProb;
      DOUBLE d = f;
      --foregroundPtr;
      fromBackground *= b2fGrowth;

      while (foregroundPtr > &foregroundProbs.front()) {
        f = *foregroundPtr;
        fromForeground += f;
        i = *(insertionPtr - 1);
        *foregroundPtr = fromBackground + f * f2f2 + (i + d) * oneGapProb;
        *insertionPtr = f + i * g2g;
        d = f + d * g2g;
        --foregroundPtr;
        --insertionPtr;
        fromBackground *= b2fGrowth;
      }

      f = *foregroundPtr;
      fromForeground += f;
      *foregroundPtr = fromBackground + f * f2f1 + d * endGapProb;
      *insertionPtr = f;
    }

    fromForeground *= f2b;
    backgroundProb = backgroundProb * b2b + fromForeground;
  }

  void calcBackwardTransitionProbsWithGaps() {
    DOUBLE toBackground = f2b * backgroundProb;
    DOUBLE *foregroundPtr = &foregroundProbs.front();
    DOUBLE f = *foregroundPtr;
    DOUBLE toForeground = f;

    if (insertionProbs.empty()) {
      *foregroundPtr = toBackground + f2f0 * f;
    } else {
      DOUBLE *insertionPtr = &insertionProbs.front();
      DOUBLE i = *insertionPtr;
      *foregroundPtr = toBackground + f2f1 * f + i;
      DOUBLE d = endGapProb * f;
      ++foregroundPtr;
      toForeground *= b2fGrowth;

      while (foregroundPtr < &foregroundProbs.back()) {
        f = *foregroundPtr;
        toForeground += f;
        i = *(insertionPtr + 1);
        *foregroundPtr = toBackground + f2f2 * f + (i + d);
        DOUBLE oneGapProb_f = oneGapProb * f;
        *insertionPtr = oneGapProb_f + g2g * i;
        d = oneGapProb_f + g2g * d;
        ++foregroundPtr;
        ++insertionPtr;
        toForeground *= b2fGrowth;
      }

      f = *foregroundPtr;
      toForeground += f;
      *foregroundPtr = toBackground + f2f1 * f + d;
      *insertionPtr = endGapProb * f;
    }

    toForeground *= b2fLast;
    backgroundProb = b2b * backgroundProb + toForeground;
  }

  void calcForwardTransitionProbs() {
    if (endGapProb > 0) return calcForwardTransitionProbsWithGaps();

    DOUBLE fromBackground = backgroundProb * b2fLast;
    DOUBLE fromForeground = 0;
    DOUBLE *foregroundPtr = END(foregroundProbs);
    DOUBLE *foregroundBeg = BEG(foregroundProbs);

    while (foregroundPtr > foregroundBeg) {
      --foregroundPtr;
      DOUBLE f = *foregroundPtr;
      fromForeground += f;
      *foregroundPtr = fromBackground + f * f2f0;
      fromBackground *= b2fGrowth;
    }

    fromForeground *= f2b;
    backgroundProb = backgroundProb * b2b + fromForeground;
  }

  void calcBackwardTransitionProbs() {
    if (endGapProb > 0) return calcBackwardTransitionProbsWithGaps();

    DOUBLE toBackground = f2b * backgroundProb;
    DOUBLE toForeground = 0;
    DOUBLE *foregroundPtr = BEG(foregroundProbs);
    DOUBLE *foregroundEnd = END(foregroundProbs);

    while (foregroundPtr < foregroundEnd) {
      toForeground *= b2fGrowth;
      DOUBLE f = *foregroundPtr;
      toForeground += f;
      *foregroundPtr = toBackground + f2f0 * f;
      ++foregroundPtr;
    }

    toForeground *= b2fLast;
    backgroundProb = b2b * backgroundProb + toForeground;
  }

  void addEndCounts(DOUBLE forwardProb,
                    DOUBLE totalProb,
                    DOUBLE *transitionCounts) {
    DOUBLE toEnd = forwardProb * b2b / totalProb;
    transitionCounts[0] += toEnd;
  }

  void addTransitionCounts(DOUBLE forwardProb,
                           DOUBLE totalProb,
                           DOUBLE *transitionCounts) {
    DOUBLE toBg = forwardProb * b2b / totalProb;
    DOUBLE toFg = forwardProb * b2fFirst / totalProb;

    transitionCounts[0] += backgroundProb * toBg;

    for (DOUBLE *i = BEG(foregroundProbs); i < END(foregroundProbs); ++i) {
      ++transitionCounts;
      *transitionCounts += *i * toFg;
      toFg *= b2fDecay;
    }
  }

  void calcEmissionProbs() {
    const DOUBLE *lrRow = likelihoodRatioMatrix[*seqPtr];

    bool isNearSeqBeg = (seqPtr - seqBeg < maxRepeatOffset);
    const uchar *seqStop = isNearSeqBeg ? seqBeg : seqPtr - maxRepeatOffset;

    DOUBLE *foregroundPtr = BEG(foregroundProbs);
    const uchar *offsetPtr = seqPtr;

    while (offsetPtr > seqStop) {
      --offsetPtr;
      *foregroundPtr *= lrRow[*offsetPtr];
      ++foregroundPtr;
    }

    while (foregroundPtr < END(foregroundProbs)) {
      *foregroundPtr *= 0;
      ++foregroundPtr;
    }
  }

  void rescale(DOUBLE scale) {
    backgroundProb *= scale;
    multiplyAll(foregroundProbs, scale);
    multiplyAll(insertionProbs, scale);
  }

  void rescaleForward() {
    if ((seqPtr - seqBeg) % scaleStepSize == scaleStepSize - 1) {
      assert(backgroundProb > 0);
      DOUBLE scale = 1 / backgroundProb;
      scaleFactors[(seqPtr - seqBeg) / scaleStepSize] = scale;
      rescale(scale);
    }
  }

  void rescaleBackward() {
    if ((seqPtr - seqBeg) % scaleStepSize == scaleStepSize - 1) {
      DOUBLE scale = scaleFactors[(seqPtr - seqBeg) / scaleStepSize];
      rescale(scale);
    }
  }

  void calcRepeatProbs(float *letterProbs) {
    initializeForwardAlgorithm();

    while (seqPtr < seqEnd) {
      calcForwardTransitionProbs();
      calcEmissionProbs();
      rescaleForward();
      *letterProbs = static_cast<float>(backgroundProb);
      ++letterProbs;
      ++seqPtr;
    }

    DOUBLE z = forwardTotal();

    initializeBackwardAlgorithm();

    while (seqPtr > seqBeg) {
      --seqPtr;
      --letterProbs;
      DOUBLE nonRepeatProb = *letterProbs * backgroundProb / z;
      // Convert nonRepeatProb to a float, so that it is more likely
      // to be exactly 1 when it should be, e.g. for the 1st letter of
      // a sequence:
      *letterProbs = 1 - static_cast<float>(nonRepeatProb);
      rescaleBackward();
      calcEmissionProbs();
      calcBackwardTransitionProbs();
    }

    DOUBLE z2 = backwardTotal();
    checkForwardAndBackwardTotals(z, z2);
  }

  void countTransitions(DOUBLE *transitionCounts) {
    std::vector<float> p(seqEnd - seqBeg);
    float *letterProbs = BEG(p);

    initializeForwardAlgorithm();

    while (seqPtr < seqEnd) {
      *letterProbs = static_cast<float>(backgroundProb);
      calcForwardTransitionProbs();
      calcEmissionProbs();
      rescaleForward();
      ++letterProbs;
      ++seqPtr;
    }

    DOUBLE z = forwardTotal();

    addEndCounts(backgroundProb, z, transitionCounts);

    initializeBackwardAlgorithm();

    while (seqPtr > seqBeg) {
      --seqPtr;
      --letterProbs;
      rescaleBackward();
      calcEmissionProbs();
      addTransitionCounts(*letterProbs, z, transitionCounts);
      calcBackwardTransitionProbs();
    }

    DOUBLE z2 = backwardTotal();
    checkForwardAndBackwardTotals(z, z2);
  }
};

void maskSequences(uchar *seqBeg,
                   uchar *seqEnd,
                   int maxRepeatOffset,
                   const const_double_ptr *likelihoodRatioMatrix,
                   DOUBLE repeatProb,
                   DOUBLE repeatEndProb,
                   DOUBLE repeatOffsetProbDecay,
                   DOUBLE firstGapProb,
                   DOUBLE otherGapProb,
                   DOUBLE minMaskProb,
                   const uchar *maskTable) {
  std::vector<float> p(seqEnd - seqBeg);
  float *probabilities = BEG(p);

  getProbabilities(seqBeg, seqEnd, maxRepeatOffset,
                   likelihoodRatioMatrix, repeatProb, repeatEndProb,
                   repeatOffsetProbDecay, firstGapProb, otherGapProb,
                   probabilities);

  maskProbableLetters(seqBeg, seqEnd, probabilities, minMaskProb, maskTable);
}

void getProbabilities(const uchar *seqBeg,
                      const uchar *seqEnd,
                      int maxRepeatOffset,
                      const const_double_ptr *likelihoodRatioMatrix,
                      DOUBLE repeatProb,
                      DOUBLE repeatEndProb,
                      DOUBLE repeatOffsetProbDecay,
                      DOUBLE firstGapProb,
                      DOUBLE otherGapProb,
                      float *probabilities) {
  Tantan tantan(seqBeg, seqEnd, maxRepeatOffset, likelihoodRatioMatrix,
                repeatProb, repeatEndProb, repeatOffsetProbDecay,
                firstGapProb, otherGapProb);
  tantan.calcRepeatProbs(probabilities);
}

void maskProbableLetters(uchar *seqBeg,
                         uchar *seqEnd,
                         const float *probabilities,
                         DOUBLE minMaskProb,
                         const uchar *maskTable) {
  while (seqBeg < seqEnd) {
    if (*probabilities >= minMaskProb)
      *seqBeg = maskTable[*seqBeg];
    ++probabilities;
    ++seqBeg;
  }
}

void countTransitions(const uchar *seqBeg,
                      const uchar *seqEnd,
                      int maxRepeatOffset,
                      const const_double_ptr *likelihoodRatioMatrix,
                      DOUBLE repeatProb,
                      DOUBLE repeatEndProb,
                      DOUBLE repeatOffsetProbDecay,
                      DOUBLE firstGapProb,
                      DOUBLE otherGapProb,
                      DOUBLE *transitionCounts) {
  Tantan tantan(seqBeg, seqEnd, maxRepeatOffset, likelihoodRatioMatrix,
                repeatProb, repeatEndProb, repeatOffsetProbDecay,
                firstGapProb, otherGapProb);
  tantan.countTransitions(transitionCounts);
}

}
