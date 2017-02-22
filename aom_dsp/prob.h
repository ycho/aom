/*
 * Copyright (c) 2016, Alliance for Open Media. All rights reserved
 *
 * This source code is subject to the terms of the BSD 2 Clause License and
 * the Alliance for Open Media Patent License 1.0. If the BSD 2 Clause License
 * was not distributed with this source code in the LICENSE file, you can
 * obtain it at www.aomedia.org/license/software. If the Alliance for Open
 * Media Patent License 1.0 was not distributed with this source code in the
 * PATENTS file, you can obtain it at www.aomedia.org/license/patent.
 */

#ifndef AOM_DSP_PROB_H_
#define AOM_DSP_PROB_H_

#include "./aom_config.h"
#include "./aom_dsp_common.h"

#include "aom_ports/bitops.h"
#include "aom_ports/mem.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t aom_prob;

// TODO(negge): Rename this aom_prob once we remove vpxbool.
typedef uint16_t aom_cdf_prob;

#define CDF_PROB_BITS 15
#define CDF_PROB_TOP (1 << CDF_PROB_BITS)

#define MAX_PROB 255

#define aom_prob_half ((aom_prob)128)

typedef int8_t aom_tree_index;

#define TREE_SIZE(leaf_count) (-2 + 2 * (leaf_count))

#define MODE_MV_COUNT_SAT 20

/* We build coding trees compactly in arrays.
   Each node of the tree is a pair of aom_tree_indices.
   Array index often references a corresponding probability table.
   Index <= 0 means done encoding/decoding and value = -Index,
   Index > 0 means need another bit, specification at index.
   Nonnegative indices are always even;  processing begins at node 0. */

typedef const aom_tree_index aom_tree[];

static INLINE aom_prob clip_prob(int p) {
  return (p > 255) ? 255 : (p < 1) ? 1 : p;
}

static INLINE aom_prob get_prob(int num, int den) {
  return (den == 0) ? 128u : clip_prob(((int64_t)num * 256 + (den >> 1)) / den);
}

static INLINE aom_prob get_binary_prob(int n0, int n1) {
  return get_prob(n0, n0 + n1);
}

/* This function assumes prob1 and prob2 are already within [1,255] range. */
static INLINE aom_prob weighted_prob(int prob1, int prob2, int factor) {
  return ROUND_POWER_OF_TWO(prob1 * (256 - factor) + prob2 * factor, 8);
}

static INLINE aom_prob merge_probs(aom_prob pre_prob, const unsigned int ct[2],
                                   unsigned int count_sat,
                                   unsigned int max_update_factor) {
  const aom_prob prob = get_binary_prob(ct[0], ct[1]);
  const unsigned int count = AOMMIN(ct[0] + ct[1], count_sat);
  const unsigned int factor = max_update_factor * count / count_sat;
  return weighted_prob(pre_prob, prob, factor);
}

// MODE_MV_MAX_UPDATE_FACTOR (128) * count / MODE_MV_COUNT_SAT;
static const int count_to_update_factor[MODE_MV_COUNT_SAT + 1] = {
  0,  6,  12, 19, 25, 32,  38,  44,  51,  57, 64,
  70, 76, 83, 89, 96, 102, 108, 115, 121, 128
};

static INLINE aom_prob mode_mv_merge_probs(aom_prob pre_prob,
                                           const unsigned int ct[2]) {
  const unsigned int den = ct[0] + ct[1];
  if (den == 0) {
    return pre_prob;
  } else {
    const unsigned int count = AOMMIN(den, MODE_MV_COUNT_SAT);
    const unsigned int factor = count_to_update_factor[count];
    const aom_prob prob =
        clip_prob(((int64_t)(ct[0]) * 256 + (den >> 1)) / den);
    return weighted_prob(pre_prob, prob, factor);
  }
}

void aom_tree_merge_probs(const aom_tree_index *tree, const aom_prob *pre_probs,
                          const unsigned int *counts, aom_prob *probs);

#if CONFIG_EC_MULTISYMBOL
int tree_to_cdf(const aom_tree_index *tree, const aom_prob *probs,
                aom_tree_index root, aom_cdf_prob *cdf, aom_tree_index *ind,
                int *pth, int *len);

static INLINE void av1_tree_to_cdf(const aom_tree_index *tree,
                                   const aom_prob *probs, aom_cdf_prob *cdf) {
  aom_tree_index index[16];
  int path[16];
  int dist[16];
  tree_to_cdf(tree, probs, 0, cdf, index, path, dist);
}

#define av1_tree_to_cdf_1D(tree, probs, cdf, u) \
  do {                                          \
    int i;                                      \
    for (i = 0; i < u; i++) {                   \
      av1_tree_to_cdf(tree, probs[i], cdf[i]);  \
    }                                           \
  } while (0)

#define av1_tree_to_cdf_2D(tree, probs, cdf, v, u)     \
  do {                                                 \
    int j;                                             \
    int i;                                             \
    for (j = 0; j < v; j++) {                          \
      for (i = 0; i < u; i++) {                        \
        av1_tree_to_cdf(tree, probs[j][i], cdf[j][i]); \
      }                                                \
    }                                                  \
  } while (0)

void av1_indices_from_tree(int *ind, int *inv, int len,
                           const aom_tree_index *tree);
#endif

DECLARE_ALIGNED(16, extern const uint8_t, aom_norm[256]);

#if CONFIG_EC_ADAPT
static INLINE void update_cdf(aom_cdf_prob *cdf, int val, int nsymbs) {
#if 0
  const int rate = 4 + (cdf[nsymbs] > 31) + get_msb(nsymbs);
  const int rate2 = 12 - rate;
  int i, tmp;
  int diff;
#if 1
  const int tmp0 = 1 << rate2;
  tmp = tmp0;
  diff = ((CDF_PROB_TOP - (nsymbs << rate2)) >> rate) << rate;
  // Single loop (faster)
  for (i = 0; i < nsymbs - 1; ++i, tmp += tmp0) {
    tmp += (i == val ? diff : 0);
    cdf[i] -= ((cdf[i] - tmp) >> rate);
  }
#else
  for (i = 0; i < nsymbs; ++i) {
    tmp = (i + 1) << rate2;
    cdf[i] -= ((cdf[i] - tmp) >> rate);
  }
  diff = CDF_PROB_TOP - cdf[nsymbs - 1];

  for (i = val; i < nsymbs; ++i) {
    cdf[i] += diff;
  }
#endif
  cdf[nsymbs]++;
#else
  int rate;
  int count;

  extern void aom_cdf_adapt_q15(int val, uint16_t *cdf, int n, int *count, int rate);

  rate = 8;
  count = cdf[nsymbs];
  aom_cdf_adapt_q15(val, (uint16_t *)cdf, nsymbs, &count, rate);

  cdf[nsymbs] = count;
#endif
}
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // AOM_DSP_PROB_H_
