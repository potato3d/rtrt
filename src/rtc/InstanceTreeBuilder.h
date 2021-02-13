#pragma once
#ifndef _RTC_INSTANCETREEBUILDER_H_
#define _RTC_INSTANCETREEBUILDER_H_

#include <rtu/common.h>
#include <rtc/RawKdTree.h>
#include <rtc/Instance.h>

namespace rtc {

class InstanceTreeBuilder
{
public:
	RawKdTree* buildTree( const std::vector<Instance>& instances );

private:
	RawKdNode* recursiveBuild( const RawKdNode::Elements& instances, const AABB& bbox, unsigned int treeDepth );

	// Find axis in descending order of maximum extent
	void orderAxis( unsigned int* axis, const AABB& bbox );
	RawKdNode* leafNode( const RawKdNode::Elements& instances, unsigned int treeDepth );

	const std::vector<Instance>* _instances;
	RawKdTree::Statistics* _stats;
};

} // namespace rtc

#endif // _RTC_INSTANCETREEBUILDER_H_
