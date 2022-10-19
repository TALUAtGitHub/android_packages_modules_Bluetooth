/**
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*------------------------------------------------------------------------------
 *
 *  All declarations relevant for the SyncInserter class. This class exposes a
 *  public interface that lets a client supply two aptX encoder objects (left
 *  and right stereo channel) and have the current quantised codes adjusted to
 *  bury an autosync bit.
 *
 *----------------------------------------------------------------------------*/

#ifndef SYNCINSERTER_H
#define SYNCINSERTER_H
#ifdef _GCC
#pragma GCC visibility push(hidden)
#endif

#include "AptxParameters.h"

/* Function to insert sync information into one of the 8 quantised codes
 * spread across 2 aptX codewords (1 codeword per channel) */
XBT_INLINE_ void xbtEncinsertSync(Encoder_data* leftChannelEncoder,
                                  Encoder_data* rightChannelEncoder,
                                  uint32_t* syncWordPhase) {
  /* Currently using 0x1 as the 8-bit sync pattern */
  static const uint32_t syncWord = 0x1;
  uint32_t tmp_var;

  uint32_t i;

  /* Variable to hold the XOR of all the quantised code lsbs */
  uint32_t xorCodeLsbs;

  /* Variable to point to the quantiser with the minimum calculated distance
   * penalty. */
  Quantiser_data* minPenaltyQuantiser;

  /* Get the vector of quantiser pointers from the left and right encoders */
  Quantiser_data* leftQuant[4];
  Quantiser_data* rightQuant[4];
  leftQuant[0] = &leftChannelEncoder->m_qdata[0];
  leftQuant[1] = &leftChannelEncoder->m_qdata[1];
  leftQuant[2] = &leftChannelEncoder->m_qdata[2];
  leftQuant[3] = &leftChannelEncoder->m_qdata[3];
  rightQuant[0] = &rightChannelEncoder->m_qdata[0];
  rightQuant[1] = &rightChannelEncoder->m_qdata[1];
  rightQuant[2] = &rightChannelEncoder->m_qdata[2];
  rightQuant[3] = &rightChannelEncoder->m_qdata[3];

  /* Starting quantiser traversal with the LL quantiser from the left channel.
   * Initialise the pointer to the minimum penalty quantiser with the details
   * of the left LL quantiser. Initialise the code lsbs XOR variable with the
   * left LL quantised code lsbs and also XOR in the left and right random
   * dither bit generated by the 2 encoders. */
  xorCodeLsbs = ((rightQuant[LL]->qCode) & 0x1) ^
                leftChannelEncoder->m_dithSyncRandBit ^
                rightChannelEncoder->m_dithSyncRandBit;
  minPenaltyQuantiser = rightQuant[LH];

  /* Traverse across the LH, HL and HH quantisers from the right channel */
  for (i = LH; i <= HH; i++) {
    /* XOR in the lsb of the quantised code currently examined */
    xorCodeLsbs ^= (rightQuant[i]->qCode) & 0x1;
  }

  /* If the distance penalty associated with a quantiser is less than the
   * current minimum, then make that quantiser the minimum penalty
   * quantiser. */
  if (rightQuant[HL]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = rightQuant[HL];
  if (rightQuant[LL]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = rightQuant[LL];
  if (rightQuant[HH]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = rightQuant[HH];

  /* Traverse across all quantisers from the left channel */
  for (i = LL; i <= HH; i++) {
    /* XOR in the lsb of the quantised code currently examined */
    xorCodeLsbs ^= (leftQuant[i]->qCode) & 0x1;
  }

  /* If the distance penalty associated with a quantiser is less than the
   * current minimum, then make that quantiser the minimum penalty
   * quantiser. */
  if (leftQuant[LH]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = leftQuant[LH];
  if (leftQuant[HL]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = leftQuant[HL];
  if (leftQuant[LL]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = leftQuant[LL];
  if (leftQuant[HH]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = leftQuant[HH];

  /* If the lsbs of all 8 quantised codes don't happen to equal the desired
   * sync bit to embed, then force them to be by replacing the optimum code
   * with the alternate code in the minimum penalty quantiser (changes the lsb
   * of the code in this quantiser) */
  if (xorCodeLsbs != ((syncWord >> (*syncWordPhase)) & 0x1)) {
    minPenaltyQuantiser->qCode = minPenaltyQuantiser->altQcode;
  }

  /* Decrement the selected sync word bit modulo 8 for the next pass. */
  tmp_var = --(*syncWordPhase);
  (*syncWordPhase) = tmp_var & 0x7;
}

XBT_INLINE_ void xbtEncinsertSyncDualMono(Encoder_data* leftChannelEncoder,
                                          Encoder_data* rightChannelEncoder,
                                          uint32_t* syncWordPhase) {
  /* Currently using 0x1 as the 8-bit sync pattern */
  static const uint32_t syncWord = 0x1;
  uint32_t tmp_var;

  uint32_t i;

  /* Variable to hold the XOR of all the quantised code lsbs */
  uint32_t xorCodeLsbs;

  /* Variable to point to the quantiser with the minimum calculated distance
   * penalty. */
  Quantiser_data* minPenaltyQuantiser;

  /* Get the vector of quantiser pointers from the left and right encoders */
  Quantiser_data* leftQuant[4];
  Quantiser_data* rightQuant[4];
  leftQuant[0] = &leftChannelEncoder->m_qdata[0];
  leftQuant[1] = &leftChannelEncoder->m_qdata[1];
  leftQuant[2] = &leftChannelEncoder->m_qdata[2];
  leftQuant[3] = &leftChannelEncoder->m_qdata[3];
  rightQuant[0] = &rightChannelEncoder->m_qdata[0];
  rightQuant[1] = &rightChannelEncoder->m_qdata[1];
  rightQuant[2] = &rightChannelEncoder->m_qdata[2];
  rightQuant[3] = &rightChannelEncoder->m_qdata[3];

  /* Starting quantiser traversal with the LL quantiser from the left channel.
   * Initialise the pointer to the minimum penalty quantiser with the details
   * of the left LL quantiser. Initialise the code lsbs XOR variable with the
   * left LL quantised code lsbs */
  xorCodeLsbs = leftChannelEncoder->m_dithSyncRandBit;

  minPenaltyQuantiser = leftQuant[LH];

  /* Traverse across all the quantisers from the left channel */
  for (i = LL; i <= HH; i++) {
    /* XOR in the lsb of the quantised code currently examined */
    xorCodeLsbs ^= (leftQuant[i]->qCode) & 0x1;
  }

  /* If the distance penalty associated with a quantiser is less than the
   * current minimum, then make that quantiser the minimum penalty
   * quantiser. */
  if (leftQuant[LH]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = leftQuant[LH];
  if (leftQuant[HL]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = leftQuant[HL];
  if (leftQuant[LL]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = leftQuant[LL];
  if (leftQuant[HH]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = leftQuant[HH];

  /* If the lsbs of all 4 quantised codes don't happen to equal the desired
   * sync bit to embed, then force them to be by replacing the optimum code
   * with the alternate code in the minimum penalty quantiser (changes the lsb
   * of the code in this quantiser) */
  if (xorCodeLsbs != ((syncWord >> (*syncWordPhase)) & 0x1)) {
    minPenaltyQuantiser->qCode = minPenaltyQuantiser->altQcode;
  }

  /****  Insert sync on the Right channel  ****/
  xorCodeLsbs = rightChannelEncoder->m_dithSyncRandBit;

  minPenaltyQuantiser = rightQuant[LH];

  /* Traverse across all quantisers from the right channel */
  for (i = LL; i <= HH; i++) {
    /* XOR in the lsb of the quantised code currently examined */
    xorCodeLsbs ^= (rightQuant[i]->qCode) & 0x1;
  }

  /* If the distance penalty associated with a quantiser is less than the
   * current minimum, then make that quantiser the minimum penalty
   * quantiser. */
  if (rightQuant[LH]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = rightQuant[LH];
  if (rightQuant[HL]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = rightQuant[HL];
  if (rightQuant[LL]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = rightQuant[LL];
  if (rightQuant[HH]->distPenalty < minPenaltyQuantiser->distPenalty)
    minPenaltyQuantiser = rightQuant[HH];

  /* If the lsbs of all 4 quantised codes don't happen to equal the desired
   * sync bit to embed, then force them to be by replacing the optimum code
   * with the alternate code in the minimum penalty quantiser (changes the lsb
   * of the code in this quantiser) */
  if (xorCodeLsbs != ((syncWord >> (*syncWordPhase)) & 0x1)) {
    minPenaltyQuantiser->qCode = minPenaltyQuantiser->altQcode;
  }

  /*  End of Right channel autosync insert*/
  /* Decrement the selected sync word bit modulo 8 for the next pass. */
  tmp_var = --(*syncWordPhase);
  (*syncWordPhase) = tmp_var & 0x7;
}

#ifdef _GCC
#pragma GCC visibility pop
#endif
#endif  // SYNCINSERTER_H
