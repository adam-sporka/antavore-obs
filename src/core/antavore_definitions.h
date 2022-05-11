#pragma once

#include <stdint.h>

namespace antavore
{

using TAudioSample = double; 
using TAudioCursor = int64_t;
using TBlipBitArray = uint32_t;  // data bits at given position
using TBlipPosIndex = int32_t;  // type used for index variables of the blip position arrays
using TBitIndex = uint8_t; // type used for indexing bits at the same position
using TMessageWord = uint16_t;   // message word stored

const int g_nMscFrameLen = 256;
const int g_nMscLo = 64;
const int g_nMscHi = g_nMscLo + 64 - 1;
const int g_nMscBandCount = g_nMscHi - g_nMscLo + 1;

};

#include "circular_buffer.h"
#include "frame.h"
#include "spread_spectrum_encoder_decoder.h"
#include "imprinter.h"
#include "signal_pipeline.h"