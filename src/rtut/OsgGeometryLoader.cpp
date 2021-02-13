#include <rtut/OsgGeometryLoader.h>

#include <rt/rt.h>
#include <rtu/float3.h>
#include <rtu/float4x4.h>
#include <rtu/stl.h>

#include <rtl/PhongColorMaterial.h>
#include <rtl/Texture2D.h>

#include <osgDB/ReadFile>
#include <osgUtil/TriStripVisitor>
#include <osgUtil/Optimizer>
#include <osg/Material>

namespace rtut {

// Helper classes and functions, hidden here in shame because of how this must be done with OSG
unsigned int toRtMode( unsigned int osgMode )
{
	switch( osgMode )
	{
	case osg::PrimitiveSet::TRIANGLES:
		return RT_TRIANGLES;

	case osg::PrimitiveSet::TRIANGLE_STRIP:
		return RT_TRIANGLE_STRIP;
		/*
		case osg::PrimitiveSet::POINTS:
		return GL_POINTS;

		case osg::PrimitiveSet::LINES:
		return GL_LINES;

		case osg::PrimitiveSet::LINE_STRIP:
		return GL_LINE_STRIP;

		case osg::PrimitiveSet::LINE_LOOP:
		return GL_LINE_LOOP;

		case osg::PrimitiveSet::TRIANGLE_FAN:
		return GL_TRIANGLE_FAN;

		case osg::PrimitiveSet::QUADS:
		return GL_QUADS;

		case osg::PrimitiveSet::QUAD_STRIP:
		return GL_QUAD_STRIP;

		case osg::PrimitiveSet::POLYGON:
		return GL_POLYGON;
		*/
	default:
		return 0;
	}
}

class rtVertex
{
public:
    rtVertex(const osg::Array* vertices)
		: _vertices(vertices)
    {
        _verticesType = _vertices ? _vertices->getType() : osg::Array::ArrayType;
    }

    inline void operator () (unsigned int pos)
    {
        switch(_verticesType)
        {
        case(osg::Array::Vec3ArrayType): 
            apply((*(static_cast<const osg::Vec3Array*>(_vertices)))[pos]);
            break;
        case(osg::Array::Vec2ArrayType): 
            apply((*(static_cast<const osg::Vec2Array*>(_vertices)))[pos]);
            break;
        case(osg::Array::Vec4ArrayType): 
            apply((*(static_cast<const osg::Vec4Array*>(_vertices)))[pos]);
            break;
        default:
            break;
        }
    }
    
    inline void apply(const osg::Vec2& v)
	{ 
		rtVertex3f( v[0], v[1], 0.0f );
	}
    inline void apply(const osg::Vec3& v)
	{
		rtVertex3f( v[0], v[1], v[2] );
	}
    inline void apply(const osg::Vec4& v)
	{
		rtVertex3f( v[0], v[1], v[2] );
	}

    const osg::Array*        _vertices;
    osg::Array::Type         _verticesType;
};

class rtNormal
{
public:
    rtNormal( const osg::Array* normals )
		: _normals(normals)
    {
        _normalsType = normals ? normals->getType() : osg::Array::ArrayType;
    }

    void operator()( unsigned int pos )
    {
        switch( _normalsType )
        {
        case (osg::Array::Vec3ArrayType):
            {
				const osg::Vec3Array& normals = *static_cast<const osg::Vec3Array*>(_normals);
				rtNormal3f( normals[pos][0], normals[pos][1], normals[pos][2] );
            }
            break;
        case (osg::Array::Vec3sArrayType):
            {
                const osg::Vec3sArray& normals = *static_cast<const osg::Vec3sArray*>(_normals);
                rtNormal3f( (float)normals[pos][0], (float)normals[pos][1], (float)normals[pos][2] );
            }
            break;
        case (osg::Array::Vec4sArrayType):
            {
                const osg::Vec4sArray& normals = *static_cast<const osg::Vec4sArray*>(_normals);
				rtNormal3f( (float)normals[pos][0], (float)normals[pos][1], (float)normals[pos][2] );
            }
            break;
        case (osg::Array::Vec3bArrayType):
            {
                const osg::Vec3bArray& normals = *static_cast<const osg::Vec3bArray*>(_normals);
				rtNormal3f( (float)normals[pos][0], (float)normals[pos][1], (float)normals[pos][2] );
            }
            break;
        case (osg::Array::Vec4bArrayType):
            {
                const osg::Vec4bArray& normals = *static_cast<const osg::Vec4bArray*>(_normals);
				rtNormal3f( (float)normals[pos][0], (float)normals[pos][1], (float)normals[pos][2] );
            }
            break;
        default:
            break;
        }
    }
    
    const osg::Array* _normals;
    osg::Array::Type  _normalsType;
};

class rtColor
{
public:
	rtColor(const osg::Array* colors)
		: _colors(colors)
	  {
		  _colorsType = _colors ? _colors->getType() : osg::Array::ArrayType;
	  }

	  void operator () (unsigned int pos)
	  {
		  switch(_colorsType)
		  {
		  case(osg::Array::Vec4ArrayType):
			  apply((*static_cast<const osg::Vec4Array*>(_colors))[pos]);
			  break;
		  case(osg::Array::Vec4ubArrayType):
			  apply((*static_cast<const osg::Vec4ubArray*>(_colors))[pos]);
			  break;
		  case(osg::Array::Vec3ArrayType):
			  apply((*static_cast<const osg::Vec3Array*>(_colors))[pos]);
			  break;
		  default:
			  break;
		  }
	  }

	  void apply(const osg::Vec4ub& v) 
	  { 
		  rtColor3f( (float)v[0], (float)v[1], (float)v[2] );
	  }
	  void apply(const osg::Vec3& v)   
	  { 
		  rtColor3f( v[0], v[1], v[2] );
	  }
	  void apply(const osg::Vec4& v)   
	  { 
		  rtColor3f( v[0], v[1], v[2] );
	  }

	  const osg::Array*        _colors;
	  osg::Array::Type         _colorsType;
};

class rtDrawTexCoord : public osg::ConstValueVisitor
{
public:

	rtDrawTexCoord( const osg::Array* texcoords ):
	  _texcoords(texcoords)
	  {
	  }

	  inline void operator () (unsigned int pos)
	  {
		  _texcoords->accept(pos,*this);
	  }

	  virtual void apply(const GLfloat& v)
	  { 
		  rtTexCoord2f( v, 0.0f );
	  }
	  virtual void apply(const osg::Vec2& v)
	  { 
		  rtTexCoord2f( v[0], v[1] );
	  }
	  virtual void apply(const osg::Vec3& v)
	  { 
		  rtTexCoord2f( v[0], v[1] );
	  }
	  virtual void apply(const osg::Vec4& v)
	  { 
		  rtTexCoord2f( v[0], v[1] );
	  }

	  const osg::Array*        _texcoords;
};

class CollectDataFunctor : public osg::Drawable::ConstAttributeFunctor
{
public:
	CollectDataFunctor( std::vector<float>& _vertices, std::vector<float>& _normals, std::vector<float>& _colors, std::vector<float>& _texCoords )
		:vertices( _vertices ), normals( _normals ), colors( _colors ), texCoords( _texCoords )
	{
		// empty
	}

	virtual void apply(osg::Drawable::AttributeType type, const unsigned int size, const GLbyte* buffer ) 
	{
		copyBuffer( type, size, buffer );
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const GLshort* buffer ) 
	{
		copyBuffer( type, size, buffer );
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const GLint* buffer ) 
	{
		copyBuffer( type, size, buffer );
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const GLubyte* buffer ) 
	{
		copyBuffer( type, size, buffer );
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const GLushort* buffer ) 
	{
		copyBuffer( type, size, buffer );
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const GLuint* buffer ) 
	{
		copyBuffer( type, size, buffer );
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const float* buffer ) 
	{
		copyBuffer( type, size, buffer );
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const osg::Vec2* buffer ) 
	{
		switch( type )
		{
		case osg::Drawable::VERTICES:
			vertices.reserve( vertices.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				vertices.push_back( buffer[i][0] );
				vertices.push_back( buffer[i][1] );
				vertices.push_back(	0.0f );
			}
			break;
		case osg::Drawable::NORMALS:
			normals.reserve( normals.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				normals.push_back( buffer[i][0] );
				normals.push_back( buffer[i][1] );
				normals.push_back( 0.0f );
			}
			break;
		case osg::Drawable::COLORS:
			colors.reserve( colors.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				colors.push_back( buffer[i][0] );
				colors.push_back( buffer[i][1] );
				colors.push_back( 0.0f );
			}
			break;
		case osg::Drawable::TEXTURE_COORDS_0:
			texCoords.reserve( texCoords.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				texCoords.push_back( buffer[i][0] );
				texCoords.push_back( buffer[i][1] );
				texCoords.push_back( 0.0f );
			}
			break;
		default:
			break;
		}
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const osg::Vec3* buffer ) 
	{
		switch( type )
		{
		case osg::Drawable::VERTICES:
			vertices.reserve( vertices.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				vertices.push_back( buffer[i][0] );
				vertices.push_back( buffer[i][1] );
				vertices.push_back(	buffer[i][2] );
			}
			break;
		case osg::Drawable::NORMALS:
			normals.reserve( normals.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				normals.push_back( buffer[i][0] );
				normals.push_back( buffer[i][1] );
				normals.push_back( buffer[i][2] );
			}
			break;
		case osg::Drawable::COLORS:
			colors.reserve( colors.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				colors.push_back( buffer[i][0] );
				colors.push_back( buffer[i][1] );
				colors.push_back( buffer[i][2] );
			}
			break;
		case osg::Drawable::TEXTURE_COORDS_0:
			texCoords.reserve( texCoords.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				texCoords.push_back( buffer[i][0] );
				texCoords.push_back( buffer[i][1] );
				texCoords.push_back( buffer[i][2] );
			}
			break;
		default:
			break;
		}
	}

	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const osg::Vec4* buffer ) 
	{
		switch( type )
		{
		case osg::Drawable::VERTICES:
			vertices.reserve( vertices.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				vertices.push_back( buffer[i][0] );
				vertices.push_back( buffer[i][1] );
				vertices.push_back(	buffer[i][2] );
			}
			break;
		case osg::Drawable::NORMALS:
			normals.reserve( normals.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				normals.push_back( buffer[i][0] );
				normals.push_back( buffer[i][1] );
				normals.push_back( buffer[i][2] );
			}
			break;
		case osg::Drawable::COLORS:
			colors.reserve( colors.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				colors.push_back( buffer[i][0] );
				colors.push_back( buffer[i][1] );
				colors.push_back( buffer[i][2] );
			}
			break;
		case osg::Drawable::TEXTURE_COORDS_0:
			texCoords.reserve( texCoords.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				texCoords.push_back( buffer[i][0] );
				texCoords.push_back( buffer[i][1] );
				texCoords.push_back( buffer[i][2] );
			}
			break;
		default:
			break;
		}
	}
	virtual void apply(osg::Drawable::AttributeType type,const unsigned int size,const osg::Vec4ub* buffer ) 
	{
		switch( type )
		{
		case osg::Drawable::VERTICES:
			vertices.reserve( vertices.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				vertices.push_back( (float)buffer[i][0] );
				vertices.push_back( (float)buffer[i][1] );
				vertices.push_back(	(float)buffer[i][2] );
			}
			break;
		case osg::Drawable::NORMALS:
			normals.reserve( normals.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				normals.push_back( (float)buffer[i][0] );
				normals.push_back( (float)buffer[i][1] );
				normals.push_back( (float)buffer[i][2] );
			}
			break;
		case osg::Drawable::COLORS:
			colors.reserve( colors.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				colors.push_back( (float)buffer[i][0] );
				colors.push_back( (float)buffer[i][1] );
				colors.push_back( (float)buffer[i][2] );
			}
			break;
		case osg::Drawable::TEXTURE_COORDS_0:
			texCoords.reserve( texCoords.size() + size*3 );
			for( unsigned int i = 0; i < size; ++i )
			{
				texCoords.push_back( (float)buffer[i][0] );
				texCoords.push_back( (float)buffer[i][1] );
				texCoords.push_back( (float)buffer[i][2] );
			}
			break;
		default:
			break;
		}
	}

	template<typename T>
	void copyBuffer( osg::Drawable::AttributeType type, const unsigned int size, const T* buffer )
	{
		switch( type )
		{
		case osg::Drawable::VERTICES:
			copyBufferToFloat( vertices, size, buffer );
			break;
		case osg::Drawable::NORMALS:
			copyBufferToFloat( normals, size, buffer );
			break;
		case osg::Drawable::COLORS:
			copyBufferToFloat( colors, size, buffer );
			break;
		case osg::Drawable::TEXTURE_COORDS_0:
			copyBufferToFloat( texCoords, size, buffer );
			break;
		default:
			break;
		}
	}

	template<typename T>
	void copyBufferToFloat( std::vector<float>& dest, unsigned int size, const T* buffer )
	{
		dest.reserve( dest.size() + size );
		for( unsigned int i = 0; i < size; ++i )
		{
			dest.push_back( (float)buffer[i] );
		}
	}

	std::vector<float>& vertices;
	std::vector<float>& normals;
	std::vector<float>& colors;
	std::vector<float>& texCoords;
};

class PrimitiveCollector : public osg::PrimitiveIndexFunctor
{
public:
	PrimitiveCollector( std::vector<float>& _vertices, std::vector<float>& _normals, std::vector<float>& _colors, std::vector<float>& _texCoords )
		:vertices( _vertices ), normals( _normals ), colors( _colors ), texCoords( _texCoords )
	{
		// empty
	}

	void setBindings( osg::Geometry::AttributeBinding nb,osg::Geometry::AttributeBinding cb )
	{
		normalBinding = nb;
		colorBinding = cb;
	}

	virtual void setVertexArray(unsigned int count,const osg::Vec2* vertices){}
	virtual void setVertexArray(unsigned int count,const osg::Vec3* vertices){}
	virtual void setVertexArray(unsigned int count,const osg::Vec4* vertices){}

	virtual void drawArrays(GLenum mode,GLint first,GLsizei count)
	{
		unsigned int rtMode = toRtMode( mode );
		if( rtMode == 0 )
			return;

		rtBegin( rtMode );
		for( int i = 0, idx = first*3; i < count; ++i, idx+=3 )
		{
			// TODO: not supported at this time
			//if( ( colorBinding == osg::Geometry::BIND_PER_VERTEX ) && ( idx < (int)colors.size() ) )
			//	rtColor3fv( &colors[idx] );
			if( ( normalBinding == osg::Geometry::BIND_PER_VERTEX ) && ( idx < (int)normals.size() ) )
				rtNormal3fv( &normals[idx] );
			if( ( !texCoords.empty() ) && ( idx < (int)texCoords.size() ) )
				rtTexCoord2fv( &texCoords[idx] );

			rtVertex3fv( &vertices[idx] );
		}
		rtEnd();
	}

	virtual void drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
	{
		myDrawElements( mode, count, indices );
	}
	virtual void drawElements(GLenum mode,GLsizei count,const GLushort* indices)
	{
		myDrawElements( mode, count, indices );
	}
	virtual void drawElements(GLenum mode,GLsizei count,const GLuint* indices)
	{
		myDrawElements( mode, count, indices );
	}

	template<typename T>
	void myDrawElements( GLenum mode,GLsizei count,const T* indices )
	{
		unsigned int rtMode = toRtMode( mode );
		if( rtMode == 0 )
			return;

		T idx = 0;

		rtBegin( rtMode );
		for( int i = 0; i < count; ++i )
		{
			idx = indices[i]*(T)3;

			// TODO: not supported at this time
			//if( ( colorBinding == osg::Geometry::BIND_PER_VERTEX ) && ( idx < colors.size() ) )
			//	rtColor3fv( &colors[idx] );
			if( ( normalBinding == osg::Geometry::BIND_PER_VERTEX ) && ( idx < normals.size() ) )
				rtNormal3fv( &normals[idx] );
			if( ( !texCoords.empty() ) && ( idx < (int)texCoords.size() ) )
				rtTexCoord2fv( &texCoords[idx] );

			rtVertex3fv( &vertices[idx] );
		}
		rtEnd();
	}

	virtual void begin(GLenum mode) {}
	virtual void vertex(unsigned int pos) {}
	virtual void end() {}

	osg::Geometry::AttributeBinding normalBinding;
	osg::Geometry::AttributeBinding colorBinding;
	std::vector<float>& vertices;
	std::vector<float>& normals;
	std::vector<float>& colors;
	std::vector<float>& texCoords;
};

class LoadOsgGeometryVisitor : public osg::NodeVisitor
{
public:
	LoadOsgGeometryVisitor();

	virtual void apply( osg::Transform& node );
    virtual void apply( osg::Group& node );
    virtual void apply( osg::Geode& node );

private:
    osgUtil::TriStripVisitor _tsv;
	std::vector<osg::Matrix> _matrixStack;
};

LoadOsgGeometryVisitor::LoadOsgGeometryVisitor()
	: osg::NodeVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN )
{
	// empty
}

void LoadOsgGeometryVisitor::apply( osg::Group& node )
{
    node.traverse( *this );

    // Free subgraph to save some memory
    node.removeChildren( 0, node.getNumChildren() );
}

void LoadOsgGeometryVisitor::apply( osg::Transform& trans )
{
	osg::Matrix matrix;

	if( !_matrixStack.empty() )
		matrix = _matrixStack.back();

	trans.computeLocalToWorldMatrix( matrix, NULL );
	_matrixStack.push_back( matrix );

	trans.traverse( *this );

    // Free subgraph to save some memory
    trans.removeChildren( 0, trans.getNumChildren() );

	_matrixStack.pop_back();
}

void LoadOsgGeometryVisitor::apply( osg::Geode& node )
{
	// Load current matrix from stack
	if( !_matrixStack.empty() )
	{
		rtPushMatrix();
		const osg::Matrix& matrix = _matrixStack.back();
		rtu::float4x4 m( (float)matrix( 0, 0 ), (float)matrix( 0, 1 ), (float)matrix( 0, 2 ), (float)matrix( 0, 3 ),
			(float)matrix( 1, 0 ), (float)matrix( 1, 1 ), (float)matrix( 1, 2 ), (float)matrix( 1, 3 ),
			(float)matrix( 2, 0 ), (float)matrix( 2, 1 ), (float)matrix( 2, 2 ), (float)matrix( 2, 3 ),
			(float)matrix( 3, 0 ), (float)matrix( 3, 1 ), (float)matrix( 3, 2 ), (float)matrix( 3, 3 ) );
		rtLoadMatrixfv( m.ptr() );
	}

	for( unsigned int i = 0, size = node.getNumDrawables(); i < size; ++i )
	{
		osg::Geometry* geometry = dynamic_cast<osg::Geometry*>( node.getDrawable( i ) );
		if( geometry == NULL )
			continue;

		osg::Geometry& geom = *geometry;

		if( !geom.getVertexArray() )
			continue;

		// This is where the magic happens
		// First, we verify the geometry
		geom.computeCorrectBindingsAndArraySizes();

        // Try to capture primitives we would miss since we only support triangles
        _tsv.stripify( geom );

		// Just in case, we make sure there are no indices in geometry (TriStripVisitor should have done this already)
		if( geom.suitableForOptimization() )
			geom.copyToAndOptimize( geom );

		// Initial setup
		bool fastPaths = geom.computeFastPathsUsed();
		unsigned int normalIndex = 0;
		unsigned int colorIndex = 0;

		// Setup material
		unsigned int matId = rtGenMaterials( 1 );
		rtBindMaterial( matId );
		rtl::PhongColorMaterial* mat = new rtl::PhongColorMaterial;
		rtMaterialClass( mat );

		// TODO: see if we can use rtGenMaterials here, etc
		osg::Material* m = dynamic_cast<osg::Material*>( geom.getOrCreateStateSet()->getAttribute( osg::StateAttribute::MATERIAL ) );
		if( m != NULL )
		{
			const osg::Vec4& dif = m->getDiffuse( osg::Material::FRONT_AND_BACK );
			mat->setDiffuse( dif[0], dif[1], dif[2] );
		}

		// TODO: adjust texture loading: share textures, share images
		osg::Texture2D* t = dynamic_cast<osg::Texture2D*>( geom.getOrCreateStateSet()->getTextureAttribute( 0, osg::StateAttribute::TEXTURE ) );
		if( t != NULL )
		{
			unsigned char* texels = t->getImage()->data();

			rtl::Texture2D* texture2d = new rtl::Texture2D();

			GLuint texId = rtGenTextures( 1 );
			rtBindTexture( texId );
			rtTextureClass( texture2d );
			rtTextureParameter( RT_TEXTURE_FILTER, (void*)( RT_NEAREST ) );
			rtTextureParameter( RT_TEXTURE_WRAP_S, (void*)( RT_CLAMP ) );
			rtTextureParameter( RT_TEXTURE_WRAP_T, (void*)( RT_CLAMP ) );
			rtTextureParameter( RT_TEXTURE_ENV_MODE, (void*)( RT_MODULATE ) );

			rtTextureImage2D( t->getImage()->s(), t->getImage()->t(), texels );

			rtMaterialParameter( RT_TEXTURE_ID, &texId );
		}

		rtVertex drawVertex( geom.getVertexArray() );
		rtNormal drawNormal( geom.getNormalArray() );
		// TODO: not supported at this time
		//rtColor  drawColor( geom.getColorArray() );
		rtDrawTexCoord drawTexCoord( geom.getTexCoordArray( 0 ) );

		// Now, pretend we are drawing the geometry
		// The following is a copy/paste from OpenSceneGraph Geometry.cpp drawImplementation,
		// adjusted for our needs (hate the way this must be done...)
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//
		// pass the overall binding values onto OpenGL.
		//
		if (geom.getNormalBinding()==osg::Geometry::BIND_OVERALL)
			drawNormal(normalIndex++);
		//if (geom.getColorBinding()==osg::Geometry::BIND_OVERALL)
		//	drawColor(colorIndex++);

		if( fastPaths )
		{
			// Destination data
			std::vector<float> vertices;
			std::vector<float> normals;
			std::vector<float> colors;
			std::vector<float> texCoords;

			// Collect geometry data
			CollectDataFunctor cdf( vertices, normals, colors, texCoords );
			geom.accept( cdf );

			// Send to ray tracer according to primitive sets
			PrimitiveCollector pc( vertices, normals, colors, texCoords );
			pc.setBindings( geom.getNormalBinding(), geom.getColorBinding() );
			geom.accept( pc );

			// Finished this geometry
			continue;
		}

		// Slow path, need to send each vertex, normal and color by hand
		///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//
		// draw the primitives themselves.
		//
		for(osg::Geometry::PrimitiveSetList::const_iterator itr=geom.getPrimitiveSetList().begin();
			itr!=geom.getPrimitiveSetList().end();
			++itr)
		{
			const osg::PrimitiveSet* primitiveset = itr->get();

			// Check if we support this primitive mode (currently only triangles or triangle_strips are supported)
			unsigned int mode = toRtMode( primitiveset->getMode() );
			if( mode == 0 )
			{
				printf( "rtut: warning! primitive mode not supported, skipping primitive set.\n" );
				continue;
			}

			if (geom.getNormalBinding()==osg::Geometry::BIND_PER_PRIMITIVE_SET)
				drawNormal(normalIndex++);
			//if (geom.getColorBinding()==osg::Geometry::BIND_PER_PRIMITIVE_SET)            
			//	drawColor(colorIndex++);

			unsigned int primLength;
			switch(mode)
			{
			case(RT_TRIANGLES): primLength=3; break;
			default:            primLength=0; break; // compute later when =0.
			}

			// draw primitives by the more flexible "slow" path,
			// sending OpenGL glBegin/glVertex.../glEnd().
			switch(primitiveset->getType())
			{
			case(osg::PrimitiveSet::DrawArraysPrimitiveType):
				{
					if (primLength==0) 
						primLength=primitiveset->getNumIndices();

					const osg::DrawArrays* drawArray = static_cast<const osg::DrawArrays*>(primitiveset);
					rtBegin(mode);

					unsigned int primCount=0;
					unsigned int indexEnd = drawArray->getFirst()+drawArray->getCount();
					for(unsigned int vindex=drawArray->getFirst();
						vindex<indexEnd;
						++vindex,++primCount)
					{

						if ((primCount%primLength)==0)
						{
							if (geom.getNormalBinding()==osg::Geometry::BIND_PER_PRIMITIVE)           
								drawNormal(normalIndex++);
							//if (geom.getColorBinding()==osg::Geometry::BIND_PER_PRIMITIVE)            
							//	drawColor(colorIndex++);
						}

						if (geom.getNormalBinding()==osg::Geometry::BIND_PER_VERTEX)           
							drawNormal(vindex);
						//if (geom.getColorBinding()==osg::Geometry::BIND_PER_VERTEX)            
						//	drawColor(vindex);
						if ( drawTexCoord._texcoords != NULL )
							drawTexCoord( vindex );

						drawVertex(vindex);
					}
					rtEnd();
					break;
				}
			case(osg::PrimitiveSet::DrawArrayLengthsPrimitiveType):
				{
					const osg::DrawArrayLengths* drawArrayLengths = static_cast<const osg::DrawArrayLengths*>(primitiveset);
					unsigned int vindex=drawArrayLengths->getFirst();
					for(osg::DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
						primItr!=drawArrayLengths->end();
						++primItr)
					{
						unsigned int localPrimLength;
						if (primLength==0) 
							localPrimLength=*primItr;
						else 
							localPrimLength=primLength;

						rtBegin(mode);

						for(GLsizei primCount=0;primCount<*primItr;++primCount)
						{
							if ((primCount%localPrimLength)==0)
							{
								if (geom.getNormalBinding()==osg::Geometry::BIND_PER_PRIMITIVE)           
									drawNormal(normalIndex++);
								//if (geom.getColorBinding()==osg::Geometry::BIND_PER_PRIMITIVE)            
								//	drawColor(colorIndex++);
							}

							if (geom.getNormalBinding()==osg::Geometry::BIND_PER_VERTEX)           
								drawNormal(vindex);
							//if (geom.getColorBinding()==osg::Geometry::BIND_PER_VERTEX)            
							//	drawColor(vindex);
							if ( drawTexCoord._texcoords != NULL )
								drawTexCoord( vindex );

							drawVertex(vindex);

							++vindex;
						}
						rtEnd();
					}
					break;
				}
			case(osg::PrimitiveSet::DrawElementsUBytePrimitiveType):
				{
					if (primLength==0) 
						primLength=primitiveset->getNumIndices();

					const osg::DrawElementsUByte* drawElements = static_cast<const osg::DrawElementsUByte*>(primitiveset);
					rtBegin(mode);

					unsigned int primCount=0;
					for(osg::DrawElementsUByte::const_iterator primItr=drawElements->begin();
						primItr!=drawElements->end();
						++primCount,++primItr)
					{

						if ((primCount%primLength)==0)
						{
							if (geom.getNormalBinding()==osg::Geometry::BIND_PER_PRIMITIVE)           
								drawNormal(normalIndex++);
							//if (geom.getColorBinding()==osg::Geometry::BIND_PER_PRIMITIVE)            
							//	drawColor(colorIndex++);
						}

						unsigned int vindex=*primItr;

						if (geom.getNormalBinding()==osg::Geometry::BIND_PER_VERTEX)           
							drawNormal(vindex);
						//if (geom.getColorBinding()==osg::Geometry::BIND_PER_VERTEX)            
						//	drawColor(vindex);
						if ( drawTexCoord._texcoords != NULL )
							drawTexCoord( vindex );

						drawVertex(vindex);
					}
					rtEnd();
					break;
				}
			case(osg::PrimitiveSet::DrawElementsUShortPrimitiveType):
				{
					if (primLength==0) 
						primLength=primitiveset->getNumIndices();

					const osg::DrawElementsUShort* drawElements = static_cast<const osg::DrawElementsUShort*>(primitiveset);
					rtBegin(mode);

					unsigned int primCount=0;
					for(osg::DrawElementsUShort::const_iterator primItr=drawElements->begin();
						primItr!=drawElements->end();
						++primCount,++primItr)
					{

						if ((primCount%primLength)==0)
						{
							if (geom.getNormalBinding()==osg::Geometry::BIND_PER_PRIMITIVE)           
								drawNormal(normalIndex++);
							//if (geom.getColorBinding()==osg::Geometry::BIND_PER_PRIMITIVE)            
							//	drawColor(colorIndex++);
						}

						unsigned int vindex=*primItr;

						if (geom.getNormalBinding()==osg::Geometry::BIND_PER_VERTEX)           
							drawNormal(vindex);
						//if (geom.getColorBinding()==osg::Geometry::BIND_PER_VERTEX)            
						//	drawColor(vindex);
						if ( drawTexCoord._texcoords != NULL )
							drawTexCoord( vindex );

						drawVertex(vindex);
					}

					rtEnd();
					break;
				}
			case(osg::PrimitiveSet::DrawElementsUIntPrimitiveType):
				{
					if (primLength==0) 
						primLength=primitiveset->getNumIndices();

					const osg::DrawElementsUInt* drawElements = static_cast<const osg::DrawElementsUInt*>(primitiveset);
					rtBegin(mode);

					unsigned int primCount=0;
					for(osg::DrawElementsUInt::const_iterator primItr=drawElements->begin();
						primItr!=drawElements->end();
						++primCount,++primItr)
					{

						if ((primCount%primLength)==0)
						{
							if (geom.getNormalBinding()==osg::Geometry::BIND_PER_PRIMITIVE)           
								drawNormal(normalIndex++);
							//if (geom.getColorBinding()==osg::Geometry::BIND_PER_PRIMITIVE)            
							//	drawColor(colorIndex++);
						}

						unsigned int vindex=*primItr;

						if (geom.getNormalBinding()==osg::Geometry::BIND_PER_VERTEX)           
							drawNormal(vindex);
						//if (geom.getColorBinding()==osg::Geometry::BIND_PER_VERTEX)            
						//	drawColor(vindex);
						if ( drawTexCoord._texcoords != NULL )
							drawTexCoord( vindex );

						drawVertex(vindex);
					}
					rtEnd();
					break;
				}
			default:
				{
					break;
				}
			}
		}
	}

	// Unload current matrix
	if( !_matrixStack.empty() )
		rtPopMatrix();
}

// OsgGeometryLoader implementation
bool OsgGeometryLoader::loadFile( char* filename, unsigned int& geometryId )
{
	printf( "osg: loading '%s'... ", filename );
	// First, we ask OpenSceneGraph to load the model file
	osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFile( filename );

	if( !loadedModel )
	{
		printf( "error!\n" );
		return false;
	}

	printf( "done\n" );

	std::string ext;
	rtu::stringGetExtension( ext, filename );
	rtu::stringToLowercase( ext );

    // Then, we optimize the graph to make things easier for us in the future
    // Current ray tracing framework only supports triangles and triangle_strip
    // To get as many geometries as we can, we need to use a TriStripVisitor in each geometry
    printf( "osg: optimizing... " );

    osgUtil::Optimizer optimizer;
    optimizer.optimize( loadedModel.get(),  
        osgUtil::Optimizer::FLATTEN_STATIC_TRANSFORMS | 
        osgUtil::Optimizer::REMOVE_REDUNDANT_NODES |
        osgUtil::Optimizer::REMOVE_LOADED_PROXY_NODES |
        osgUtil::Optimizer::SHARE_DUPLICATE_STATE |
        //osgUtil::Optimizer::COPY_SHARED_NODES |
        //osgUtil::Optimizer::TRISTRIP_GEOMETRY |
        osgUtil::Optimizer::CHECK_GEOMETRY );

    printf( "done\n" );

    // Finally, we load the geometries to the ray tracer
	printf( "rtut: begin loading geometries...\n" );

	rtPushAttributeBindings();

	geometryId = rtGenGeometries( 1 );
	rtNewGeometry( geometryId );

	rtSetAttributeBinding( RT_COLOR, RT_BIND_PER_MATERIAL );
	if( ext == "tdgn" )
		rtSetAttributeBinding( RT_TEXTURE_COORD, RT_BIND_PER_MATERIAL );

	LoadOsgGeometryVisitor geoLoader;
	loadedModel->accept( geoLoader );

	printf( "rtut: geometries loaded successfully!\n" );

	rtEndGeometry();
	rtBindMaterial( 0 );
	rtPopAttributeBindings();

	return true;
}
	
} // namespace rtut
