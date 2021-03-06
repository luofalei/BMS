/* The copyright in this software is being made available under the BSD
 * License, included below. This software may be subject to other third party
 * and contributor rights, including patent rights, and no such rights are
 * granted under this license.
 *
 * Copyright (c) 2010-2017, ITU/ISO/IEC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *  * Neither the name of the ITU/ISO/IEC nor the names of its contributors may
 *    be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

/** \file     DecCu.h
    \brief    CU decoder class (header)
*/

#ifndef __DECCU__
#define __DECCU__

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "CABACReader.h"

#include "CommonLib/TrQuant.h"
#include "CommonLib/InterPrediction.h"
#include "CommonLib/IntraPrediction.h"
#include "CommonLib/Unit.h"
#if JEM_TOOLS
#include "CommonLib/BilateralFilter.h"
#endif

//! \ingroup DecoderLib
//! \{

// ====================================================================================================================
// Class definition
// ====================================================================================================================

/// CU decoder class
class DecCu
{
public:
  DecCu();
  virtual ~DecCu();

  /// initialize access channels
  Void  init              ( TrQuant* pcTrQuant, IntraPrediction* pcIntra, InterPrediction* pcInter );

  /// destroy internal buffers
  Void  decompressCtu     ( CodingStructure& cs, const UnitArea& ctuArea );

  /// reconstruct Ctu information
protected:
  Void xIntraRecQT        ( CodingUnit&      cu, const ChannelType chType );

  Void xReconInter        ( CodingUnit&      cu );
  Void xDecodeInterTexture( CodingUnit&      cu );
  Void xReconIntraQT      ( CodingUnit&      cu );
  Void xFillPCMBuffer     ( CodingUnit&      cu );

  Void xIntraRecBlk       ( TransformUnit&   tu, const ComponentID compID );
  Void xReconPCM          ( TransformUnit&   tu);
  Void xDecodePCMTexture  ( TransformUnit&   tu, const ComponentID compID );
  Void xDecodeInterTU     ( TransformUnit&   tu, const ComponentID compID );

  Void xDeriveCUMV        ( CodingUnit&      cu );

private:
  TrQuant*          m_pcTrQuant;
  IntraPrediction*  m_pcIntraPred;
  InterPrediction*  m_pcInterPred;
#if JEM_TOOLS
  BilateralFilter   m_bilateralFilter;
#endif

#if JEM_TOOLS
  MotionInfo        m_SubPuMiBuf   [( MAX_CU_SIZE * MAX_CU_SIZE ) >> ( MIN_CU_LOG2 << 1 )];
  MotionInfo        m_SubPuExtMiBuf[( MAX_CU_SIZE * MAX_CU_SIZE ) >> ( MIN_CU_LOG2 << 1 )];
#endif
};

//! \}

#endif

