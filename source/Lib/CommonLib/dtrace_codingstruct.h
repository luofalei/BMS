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

/** \file     dtrace_codingstruct.h
 *  \brief    Easy to use dtrace calls concerning coding structures
 */

#ifndef _DTRACE_CODINGSTRUCT_H_
#define _DTRACE_CODINGSTRUCT_H_

#include "dtrace.h"
#include "dtrace_next.h"

#include "CommonLib/CommonDef.h"
#include "CommonLib/CodingStructure.h"
#include "CommonLib/Slice.h"
#include "CommonLib/Mv.h"
#include "CommonLib/Unit.h"
#include "CommonLib/UnitTools.h"

#include <cmath>

#if ENABLE_TRACING

inline void dtracePicComp( DTRACE_CHANNEL channel, CodingStructure& cs, const CPelUnitBuf& pelUnitBuf, ComponentID compId )
{
  if( !g_trace_ctx ) return;
  if( pelUnitBuf.chromaFormat == CHROMA_400 && compId != COMPONENT_Y )  return;
  const Pel* piSrc    = pelUnitBuf.bufs[compId].buf;
  UInt       uiStride = pelUnitBuf.bufs[compId].stride;
  UInt       uiWidth  = pelUnitBuf.bufs[compId].width;
  UInt       uiHeight = pelUnitBuf.bufs[compId].height;
  UInt       uiChromaScaleX = getComponentScaleX( compId, pelUnitBuf.chromaFormat );
  UInt       uiChromaScaleY = getComponentScaleY( compId, pelUnitBuf.chromaFormat );

  DTRACE                ( g_trace_ctx, channel, "\n%s: poc = %d, size=%dx%d\n\n", g_trace_ctx->getChannelName(channel), cs.slice->getPOC(), uiWidth, uiHeight );
  DTRACE_FRAME_BLOCKWISE( g_trace_ctx, channel, piSrc, uiStride, uiWidth, uiHeight, cs.sps->getMaxCUWidth() >> uiChromaScaleX, cs.sps->getMaxCUHeight() >> uiChromaScaleY);
}

#define OLD_RDCOST 1

inline void dtraceModeCost(CodingStructure &cs, double lambda)
{
  CHECK( cs.cus.size() != 1, "Only the cost for a single CU can be show with dtraceModeCost!" );

  Distortion tempDist = cs.dist;

#if OLD_RDCOST
  UInt64 tempBits = cs.fracBits >> SCALE_BITS;
#if HM_16_6_BIT_EQUAL
  UInt64 tempCost = (UInt64)floor(cs.dist + (double)tempBits * lambda + 0.5);
#else
  UInt64 tempCost = (UInt64)(cs.dist + (double)tempBits * lambda);
#endif
#else
  UInt64 tempBits = cs.fracBits;
  UInt64 tempCost = (UInt64)cs.cost;
#endif

  if( cs.cost == MAX_DOUBLE )
  {
    tempCost = 0;
    tempBits = 0;
    tempDist = 0;
  }

  bool isIntra = CU::isIntra( *cs.cus.front() );
  int intraModeL = isIntra ? cs.pus.front()->intraDir[0] : 0;
  int intraModeC = isIntra ? cs.pus.front()->intraDir[1] : 0;
#if JEM_TOOLS
  bool is65Ang = cs.sps->getSpsNext().getUseIntra65Ang();
  if( isIntra && !is65Ang ) intraModeL = g_intraMode65to33AngMapping[intraModeL];
  if( isIntra && intraModeC == DM_CHROMA_IDX ) intraModeC = is65Ang ? 68 : 36;
  else if( isIntra && !is65Ang ) intraModeC = g_intraMode65to33AngMapping[intraModeC];
#else
  if( isIntra ) intraModeL = g_intraMode65to33AngMapping[intraModeL];
  if( isIntra && intraModeC == DM_CHROMA_IDX ) intraModeC = 36;
  else if( isIntra ) intraModeC = g_intraMode65to33AngMapping[intraModeC];
#endif
#if JEM_TOOLS
  DTRACE( g_trace_ctx, D_MODE_COST, "ModeCost: %6lld %3d @(%4d,%4d) [%2dx%2d] %d (qp%d,pm%d,ptSize%d,skip%d,mrg%d,fruc%d,obmc%d,ic%d,imv%d,affn%d,%d,%d) tempCS = %lld (%d,%d)\n",
    DTRACE_GET_COUNTER( g_trace_ctx, D_MODE_COST ),
    cs.slice->getPOC(),
    cs.area.Y().x, cs.area.Y().y,
    cs.area.Y().width, cs.area.Y().height,
    cs.cus[0]->qtDepth,
    cs.cus[0]->qp,
    cs.cus[0]->predMode,
    cs.cus[0]->partSize,
    cs.cus[0]->skip,
    cs.pus[0]->mergeFlag,
    cs.pus[0]->frucMrgMode,
    cs.cus[0]->obmcFlag,
    cs.cus[0]->LICFlag,
    cs.cus[0]->imv,
    cs.cus[0]->affine,
    intraModeL, intraModeC,
    tempCost, tempBits, tempDist );
#else
  DTRACE( g_trace_ctx, D_MODE_COST, "ModeCost: %6lld %3d @(%4d,%4d) [%2dx%2d] %d (qp%d,pm%d,ptSize%d,skip%d,mrg%d,fruc%d,obmc%d,ic%d,imv%d,affn%d,%d,%d) tempCS = %lld (%d,%d)\n",
    DTRACE_GET_COUNTER( g_trace_ctx, D_MODE_COST ),
    cs.slice->getPOC(),
    cs.area.Y().x, cs.area.Y().y,
    cs.area.Y().width, cs.area.Y().height,
    cs.cus[0]->qtDepth,
    cs.cus[0]->qp,
    cs.cus[0]->predMode,
    cs.cus[0]->partSize,
    cs.cus[0]->skip,
    cs.pus[0]->mergeFlag,
    0, 0, 0, 0, 0,
          intraModeL, intraModeC,
          tempCost, tempBits, tempDist );
#endif
}

inline void dtraceBestMode(CodingStructure *&tempCS, CodingStructure *&bestCS, double lambda)
{
  Bool bSplitCS = tempCS->cus.size() > 1 || bestCS->cus.size() > 1;
  ChannelType chType = tempCS->cus.back()->chType;

  // if the last CU does not align with the CS, we probably are at the edge
  bSplitCS |= tempCS->cus.back()->blocks[chType].bottomRight() != tempCS->area.blocks[chType].bottomRight();

  Distortion tempDist = tempCS->dist;

#if OLD_RDCOST
  UInt64 tempBits = tempCS->fracBits >> SCALE_BITS;
  UInt64 bestBits = bestCS->fracBits >> SCALE_BITS;
#if HM_16_6_BIT_EQUAL
  UInt64 tempCost = (UInt64)floor(tempCS->dist + (double)tempBits * lambda + 0.5);
  UInt64 bestCost = (UInt64)floor(bestCS->dist + (double)bestBits * lambda + 0.5);
#else
  UInt64 tempCost = (UInt64)(tempCS->dist + (double)tempBits * lambda);
  UInt64 bestCost = (UInt64)(bestCS->dist + (double)bestBits * lambda);
#endif
#else
  UInt64 tempBits = tempCS->fracBits;
  UInt64 bestBits = bestCS->fracBits;
  UInt64 tempCost = (UInt64)tempCS->cost;
  UInt64 bestCost = (UInt64)bestCS->cost;
#endif

  if( tempCS->cost == MAX_DOUBLE )
  {
    tempCost = 0;
    tempBits = 0;
    tempDist = 0;
  }

  bool isIntra = CU::isIntra( *tempCS->cus[0] );
  int intraModeL = isIntra ? tempCS->pus[0]->intraDir[0] : 0;
  int intraModeC = isIntra ? tempCS->pus[0]->intraDir[1] : 0;
#if JEM_TOOLS
  bool is65Ang = tempCS->sps->getSpsNext().getUseIntra65Ang();
  if( isIntra && !is65Ang ) intraModeL = g_intraMode65to33AngMapping[intraModeL];
  if( isIntra && !is65Ang && intraModeC == DM_CHROMA_IDX ) intraModeC = 36;
  else if( isIntra && !is65Ang ) intraModeC = g_intraMode65to33AngMapping[intraModeC];
#else
  if( isIntra ) intraModeL = g_intraMode65to33AngMapping[intraModeL];
  if( isIntra && intraModeC == DM_CHROMA_IDX ) intraModeC = 36;
  else if( isIntra ) intraModeC = g_intraMode65to33AngMapping[intraModeC];
#endif

  if(!bSplitCS)
  {
    DTRACE( g_trace_ctx, D_BEST_MODE, "CheckModeCost: %6lld %3d @(%4d,%4d) [%2dx%2d] %d (%d,%d,%2d,%d,%d,%d) tempCS = %lld (%d,%d), bestCS = %lld (%d,%d): --> choose %s\n",
            DTRACE_GET_COUNTER( g_trace_ctx, D_BEST_MODE ),
            tempCS->slice->getPOC(),
            tempCS->area.Y().x, tempCS->area.Y().y,
            tempCS->area.Y().width, tempCS->area.Y().height,
            tempCS->cus[0]->qtDepth,
            tempCS->cus[0]->qp,
            tempCS->cus[0]->predMode,
            tempCS->cus[0]->partSize,
            tempCS->pus[0]->mergeFlag,
            intraModeL, intraModeC,
            tempCost, tempBits, tempDist,
            bestCost, bestBits, bestCS->dist,
            tempCS->cost < bestCS->cost ? "TEMP" : "BEST" );
  }
  else
  {
    DTRACE( g_trace_ctx, D_BEST_MODE, "CheckModeSplitCost: %6lld %3d @(%4d,%4d) [%2dx%2d] -------------------------- tempCS = %lld (%d,%d), bestCS = %lld (%d,%d): --> choose %s\n",
            DTRACE_GET_COUNTER( g_trace_ctx, D_BEST_MODE ),
            tempCS->slice->getPOC(),
            tempCS->area.Y().x, tempCS->area.Y().y,
            tempCS->area.Y().width, tempCS->area.Y().height,
            tempCost, tempBits, tempDist,
            bestCost, bestBits, bestCS->dist,
            tempCS->cost < bestCS->cost ? "TEMP STRUCTURE" : "BEST STRUCTURE");
  }
}


#define DTRACE_PIC_COMP(...)             dtracePicComp( __VA_ARGS__ )
#define DTRACE_BEST_MODE(...)            dtraceBestMode(__VA_ARGS__)
#define DTRACE_MODE_COST(...)            dtraceModeCost(__VA_ARGS__)
#define DTRACE_STAT(...)                 dtraceComprPicStat(__VA_ARGS__)

#else

#define DTRACE_BEST_MODE(...)
#define DTRACE_MODE_COST(...)
#define DTRACE_STAT(...)
#define DTRACE_PIC_COMP(...)

#endif

#endif // _DTRACE_HEVC_H_
