#pragma once
#ifndef _RTUT_OSGGEOMETRYLOADER_H_
#define _RTUT_OSGGEOMETRYLOADER_H_

namespace rtut {

class OsgGeometryLoader
{
public:
	bool loadFile( char* filename, unsigned int& geometryId );
};

} // namespace rtut

#endif // _RTUT_OSGGEOMETRYLOADER_H_
