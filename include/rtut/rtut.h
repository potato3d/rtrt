#pragma once
#ifndef _RTUT_H_
#define _RTUT_H_

#include <rtu/common.h>
#include <rtu/float3.h>

/************************************************************************/
/* Ray Tracing Utility Toolkit                                          */
/************************************************************************/

void rtutLogo();

// Basic shapes
void rtutCube();
void rtutSphere( unsigned int slices, unsigned int stacks );
void rtutTeapot();

// Geometry loading

// Simple triangle scene for tests
bool rtutLoadRa2( char* filename, unsigned int& geometryId );

// Accepts any file format supported by OpenSceneGraph
bool rtutLoadOpenSceneGraph( char* filename, unsigned int& geometryId );

#endif // _RTUT_H_
