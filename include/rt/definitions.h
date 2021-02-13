#pragma once
#ifndef _RTC_DEFINITIONS_H_
#define _RTC_DEFINITIONS_H_

//////////////////////////////////////////////////////////////////////////
// Common definitions to Ray Tracing libraries
// Note: commented out means not currently used/supported
//////////////////////////////////////////////////////////////////////////

// Maximum number of threads supported
#define RT_MAX_THREAD_COUNT 4

// Ray packet dimension (2x2, 4x4, etc)
#define RT_PACKET_DIM 4

// Total number of rays in packet
#define RT_PACKET_SIZE ( RT_PACKET_DIM * RT_PACKET_DIM )

// Number of rays packed in SIMD style
#define RT_PACKET_SIMD_SIZE ( RT_PACKET_SIZE / 4 )

// Primitive types
#define RT_TRIANGLES				0x0001
#define RT_TRIANGLE_STRIP			0x0002
//#define RT_TRIANGLE_FAN				0x0003

// Primitive attributes
#define RT_VERTEX					0x3000
#define RT_NORMAL					0x3001
#define RT_COLOR					0x3002
#define RT_TEXTURE_COORD			0x3003

// Attribute bindings
#define RT_BIND_PER_VERTEX			0x3300
#define RT_BIND_PER_MATERIAL		0x3301

// Data types
/*
#define RT_BYTE						0x1100
#define RT_UNSIGNED_BYTE			0x1101
#define RT_SHORT					0x1102
#define RT_UNSIGNED_SHORT			0x1103
#define RT_INT						0x1104
#define RT_UNSIGNED_INT				0x1105
#define RT_FLOAT					0x1106
#define RT_DOUBLE					0x1107
*/

// Plug-in parameters
#define RT_TRANSLATE				0x1000
#define RT_ROTATE_X					0x1001
#define RT_ROTATE_Y					0x1002
#define RT_ROTATE_Z					0x1003
#define RT_TEXTURE_ID				0x1004

// Texture parameter identifiers
#define RT_TEXTURE_FILTER			0x2000
#define RT_TEXTURE_WRAP_S			0x2001
#define RT_TEXTURE_WRAP_T			0x2002
#define RT_TEXTURE_ENV_MODE			0x2003
/*
#define RT_TEXTURE_INTERNAL_FORMAT	0x2004
#define RT_TEXTURE_FORMAT			0x2005
*/

// Texture parameter values
// Filter
#define RT_NEAREST					0x2100
#define RT_LINEAR					0x2101
/*
#define RT_NEAREST_MIPMAP_NEAREST	0x2102
#define RT_NEAREST_MIPMAP_LINEAR	0x2103
#define RT_LINEAR_MIPMAP_NEAREST	0x2104
#define RT_LINEAR_MIPMAP_LINEAR		0x2105
#define RT_EWA						0x2106
*/

// Wrap mode
#define RT_REPEAT					0x2200
#define RT_CLAMP					0x2201
/*
#define RT_CLAMP_TO_EDGE			0x2202
#define RT_CLAMP_TO_BORDER			0x2203
#define RT_MIRROR_REPEAT			0x2204
#define RT_MIRROR_CLAMP				0x2205
#define RT_MIRROR_CLAMP_TO_EDGE		0x2206
#define RT_MIRROR_CLAMP_TO_BORDER	0x2207
*/

// Environment mode
#define RT_MODULATE					0x2300
#define RT_REPLACE					0x2301
/*
#define RT_DECAL					0x2302
#define RT_BLEND					0x2303
#define RT_ADD						0x2304
*/

// Format
#define RT_RGB						0x2400
/*
#define RT_RGBA						0x2401
#define RT_RED						0x2402
#define RT_GREEN					0x2403
#define RT_BLUE						0x2404
#define RT_ALPHA					0x2405
#define RT_LUMINANCE				0x2406
#define RT_INTENSITY				0x2407
*/

#endif // _RTC_DEFINITIONS_H_
