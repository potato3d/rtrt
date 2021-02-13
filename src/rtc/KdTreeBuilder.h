#pragma once
#ifndef _RTC_KDTREEBUILDER_H_
#define _RTC_KDTREEBUILDER_H_

#include <rtu/common.h>
#include <rtc/KdTree.h>
#include <rtc/TriangleTreeBuilder.h>
#include <rtc/InstanceTreeBuilder.h>

namespace rtc {

class KdTreeBuilder
{
public:
	static void buildTree( Geometry* geometry );
	static void buildTree( KdTree& result, const std::vector<Instance>& instances );
	static void convertRawTree( KdTree& result, RawKdTree* tree );

private:
	static TriangleTreeBuilder _triangleTreeBuilder;
	static InstanceTreeBuilder _instanceTreeBuilder;
};

} // namespace rtc

#endif // _RTC_KDACCELTREEBUILDER_H_
