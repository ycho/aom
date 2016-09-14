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

/* clang-format off */

#if !defined(_decint_H)
# define _decint_H (1)
# include "av1/common/pvq_state.h"

typedef struct daala_dec_ctx daala_dec_ctx;

typedef struct daala_dec_ctx od_dec_ctx;

/*Constants for the packet state machine specific to the decoder.*/
/*Next packet to read: Data packet.*/
# define OD_PACKET_DATA (0)

struct daala_dec_ctx {
  od_state state;
  od_ec_dec *ec;
  int qm;
};

#endif
