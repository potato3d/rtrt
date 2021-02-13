#include <rtc/KdTree.h>

namespace rtc {

void KdNode::setInternalNode( const SplitPlane& plane, const KdNode* leftChild )
{
	// Store plane information
	_data = plane.axis;
	_split = plane.position;

	// Store pointer to left child
#pragma warning( disable : 4311 )
	_data += reinterpret_cast<unsigned int>( leftChild );
#pragma warning( default : 4311 )

	// Reset leaf flag
	_data &= 0x7FFFFFFF;
}

void KdNode::setLeafNode( unsigned int elementStart, unsigned int elementCount )
{
	// Store start of elements
	_elements = elementStart;

	// Store number of elements
	_data = elementCount;

	// Set leaf flag
	_data |= 0x80000000;
}

KdTree::KdTree()
: root( NULL ), elements( NULL )
{
}

void KdTree::erase()
{
	if( root != NULL )
		delete [] root;
	if( elements != NULL )
		delete [] elements;
}

} // namespace rtc
