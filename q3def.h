#ifndef Q3DEF_H

#define Q3DEF_H

//############################################################################
//##                                                                        ##
//##  Q3DEF.H                                                               ##
//##                                                                        ##
//##  Defines various Quake 3 BSP data structures.                          ##
//##                                                                        ##
//##  OpenSourced 12/5/2000 by John W. Ratcliff                             ##
//##                                                                        ##
//##  No warranty expressed or implied.                                     ##
//##                                                                        ##
//##  Part of the Q3BSP project, which converts a Quake 3 BSP file into a   ##
//##  polygon mesh.                                                         ##
//############################################################################
//##                                                                        ##
//##  Contact John W. Ratcliff at jratcliff@verant.com                      ##
//############################################################################

#include "stl.h"
#include "stringdict.h"
#include <string.h>
#include "vector.h"
#include "rect.h"
#include "plane.h"
#include "vformat.h"

typedef std::vector< Plane > PlaneVector;
class LightMapVertex;

// Face types in Quake3
enum FaceType
{
  FACETYPE_NORMAL=1,
  FACETYPE_MESH=2,
  FACETYPE_TRISURF=3,
  FACETYPE_FLARE=4
};


//**********************

typedef enum { qfalse, qtrue } qboolean;
typedef unsigned char byte;

typedef float vec_t;
typedef vec_t vec2_t[3];
typedef vec_t vec3_t[3];
typedef vec_t vec4_t[4];

#define	SIDE_FRONT		0
#define	SIDE_ON			2
#define	SIDE_BACK		1
#define	SIDE_CROSS		-2

#define	Q_PI	3.14159265358979323846


// the maximum size of game reletive pathnames
#define	MAX_QPATH		64

// qfiles.h
/*
==============================================================================

  .BSP file format

==============================================================================
*/


#define BSP_IDENT	(('P'<<24)+('S'<<16)+('B'<<8)+'I')
		// little-endian "IBSP"

#define BSP_VERSION			46


// there shouldn't be any problem with increasing these values at the
// expense of more memory allocation in the utilities
#define	MAX_MAP_MODELS		0x400
#define	MAX_MAP_BRUSHES		0x8000
#define	MAX_MAP_ENTITIES	0x800
#define	MAX_MAP_ENTSTRING	0x40000
#define	MAX_MAP_SHADERS		0x400

#define	MAX_MAP_AREAS		0x100	// MAX_MAP_AREA_BYTES in q_shared must match!
#define	MAX_MAP_FOGS		0x100
#define	MAX_MAP_PLANES		0x20000
#define	MAX_MAP_NODES		0x20000
#define	MAX_MAP_BRUSHSIDES	0x20000
#define	MAX_MAP_LEAFS		0x20000
#define	MAX_MAP_LEAFFACES	0x20000
#define	MAX_MAP_LEAFBRUSHES 0x40000
#define	MAX_MAP_PORTALS		0x20000
#define	MAX_MAP_LIGHTING	0x800000
#define	MAX_MAP_LIGHTGRID	0x800000
#define	MAX_MAP_VISIBILITY	0x200000

#define	MAX_MAP_DRAW_SURFS	0x20000
#define	MAX_MAP_DRAW_VERTS	0x80000
#define	MAX_MAP_DRAW_INDEXES	0x80000


// key / value pair sizes in the entities lump
#define	MAX_KEY				32
#define	MAX_VALUE			1024

// the editor uses these predefined yaw angles to orient entities up or down
#define	ANGLE_UP			-1
#define	ANGLE_DOWN			-2

#define	LIGHTMAP_WIDTH		128
#define	LIGHTMAP_HEIGHT		128

#define MIN_WORLD_COORD ( -65536 )
#define	MAX_WORLD_COORD	( 65536 )
#define WORLD_SIZE		( MAX_WORLD_COORD - MIN_WORLD_COORD )

//=============================================================================


typedef struct {
	int		fileofs, filelen;
} lump_t;

#define	LUMP_ENTITIES		0
#define	LUMP_SHADERS		1
#define	LUMP_PLANES			2
#define	LUMP_NODES			3
#define	LUMP_LEAFS			4
#define	LUMP_LEAFSURFACES	5
#define	LUMP_LEAFBRUSHES	6
#define	LUMP_MODELS			7
#define	LUMP_BRUSHES		8
#define	LUMP_BRUSHSIDES		9
#define	LUMP_DRAWVERTS		10
#define	LUMP_DRAWINDEXES	11
#define	LUMP_FOGS			12
#define	LUMP_SURFACES		13
#define	LUMP_LIGHTMAPS		14
#define	LUMP_LIGHTGRID		15
#define	LUMP_VISIBILITY		16
#define	HEADER_LUMPS		17

typedef struct {
	int			ident;
	int			version;

	lump_t		lumps[HEADER_LUMPS];
} dheader_t;

typedef struct {
	float		mins[3], maxs[3];
	int			firstSurface, numSurfaces;
	int			firstBrush, numBrushes;
} dmodel_t;

typedef struct {
	char		shader[MAX_QPATH];
	int			surfaceFlags;
	int			contentFlags;
} dshader_t;

// planes x^1 is allways the opposite of plane x

typedef struct {
	float		normal[3];
	float		dist;
} dplane_t;

typedef struct {
	int			planeNum;
	int			children[2];	// negative numbers are -(leafs+1), not nodes
	int			mins[3];		// for frustom culling
	int			maxs[3];
} dnode_t;

typedef struct {
	int			cluster;			// -1 = opaque cluster (do I still store these?)
	int			area;

	int			mins[3];			// for frustum culling
	int			maxs[3];

	int			firstLeafSurface;
	int			numLeafSurfaces;

	int			firstLeafBrush;
	int			numLeafBrushes;
} dleaf_t;

typedef struct {
	int			planeNum;			// positive plane side faces out of the leaf
	int			shaderNum;
} dbrushside_t;

typedef struct {
	int			firstSide;
	int			numSides;
	int			shaderNum;		// the shader that determines the contents flags
} dbrush_t;

typedef struct {
	char		shader[MAX_QPATH];
	int			brushNum;
	int			visibleSide;	// the brush side that ray tests need to clip against (-1 == none)
} dfog_t;

typedef struct {
	vec3_t		xyz;
	float		st[2];
	float		lightmap[2];
	vec3_t		normal;
	byte		color[4];
} drawVert_t;

typedef enum {
	MST_BAD,
	MST_PLANAR,
	MST_PATCH,
	MST_TRIANGLE_SOUP,
	MST_FLARE
} mapSurfaceType_t;

typedef struct {
	int			shaderNum;
	int			fogNum;
	int			surfaceType;

	int			firstVert;
	int			numVerts;

	int			firstIndex;
	int			numIndexes;

	int			lightmapNum;
	int			lightmapX, lightmapY;
	int			lightmapWidth, lightmapHeight;

	vec3_t		lightmapOrigin;
	vec3_t		lightmapVecs[3];	// for patches, [0] and [1] are lodbounds

	int			patchWidth;
	int			patchHeight;
} dsurface_t;



// shaders.h

typedef struct shaderInfo_s {
	char		shader[MAX_QPATH];
	int			surfaceFlags;
	int			contents;
	int			value;

	char		backShader[MAX_QPATH];	// for surfaces that generate different front and back passes
	char		flareShader[MAX_QPATH];	// for light flares

	float		subdivisions;			// from a "tesssize xxx"
	float		backsplashFraction;		// floating point value, usually 0.05
	float		backsplashDistance;		// default 16
	float		lightSubdivide;			// default 120
	int			lightmapSampleSize;		// lightmap sample size

	qboolean	hasPasses;				// false if the shader doesn't define any rendering passes

	qboolean	globalTexture;			// don't normalize texture repeats

	qboolean	twoSided;				// cull none
	qboolean	autosprite;				// autosprite shaders will become point lights
										// instead of area lights
	qboolean	lightFilter;			// light rays that cross surfaces of this type
										// should test against the filter image
	qboolean	forceTraceLight;		// always use -light for this surface
	qboolean	forceVLight;			// always use -vlight for this surface
	qboolean	patchShadows;			// have patches casting shadows when using -light for this surface
	qboolean	vertexShadows;			// shadows will be casted at this surface even when vertex lit
	qboolean	noVertexShadows;		// no shadows will be casted at this surface in vertex lighting
	qboolean	forceSunLight;			// force sun light at this surface even tho we might not calculate shadows in vertex lighting
	qboolean	notjunc;				// don't use this surface for tjunction fixing
	float		vertexScale;			// vertex light scale

	char		editorimage[MAX_QPATH];	// use this image to generate texture coordinates
	char		lightimage[MAX_QPATH];	// use this image to generate color / averageColor
	vec3_t		color;					// colorNormalized
	vec3_t		averageColor;

	int			width, height;
	byte		*pixels;

	vec3_t		sunLight;
	vec3_t		sunDirection;
} shaderInfo_t;

// mesh.h


typedef struct {
	int			width, height;
	drawVert_t	*verts;
} mesh_t;

// polylib.h
typedef struct
{
	int		numpoints;
	vec3_t	p[4];		// variable sized
} winding_t;

#define	MAX_POINTS_ON_WINDING	64

// you can define on_epsilon in the makefile as tighter
#ifndef	ON_EPSILON
#define	ON_EPSILON	0.1
#endif


//bspfile.h

typedef struct epair_s {
	struct epair_s	*next;
	char	*key;
	char	*value;
} epair_t;

typedef struct {
	vec3_t		origin;
	struct bspbrush_s	*brushes;
	struct parseMesh_s	*patches;
	int			firstDrawSurf;
	epair_t		*epairs;
} entity_t;


// from qbsp.h


#define	MAX_PATCH_SIZE	32

#define	CLIP_EPSILON		0.1
#define	PLANENUM_LEAF		-1

#define	HINT_PRIORITY		1000

typedef struct parseMesh_s {
	struct parseMesh_s	*next;
	mesh_t			mesh;
	shaderInfo_t	*shaderInfo;

	qboolean	grouped;			// used during shared edge grouping
	struct parseMesh_s *groupChain;
} parseMesh_t;

typedef struct bspface_s {
	struct bspface_s	*next;
	int					planenum;
	int					priority;	// added to value calculation
	qboolean			checked;
	qboolean			hint;
	winding_t			*w;
} bspface_t;

typedef struct plane_s {
	vec3_t	normal;
	vec_t	dist;
	int		type;
	struct plane_s	*hash_chain;
} plane_t;

typedef struct side_s {
	int			planenum;

	float		texMat[2][3];	// brush primitive texture matrix
	// for old brush coordinates mode
	float		vecs[2][4];		// texture coordinate mapping

	winding_t	*winding;
	winding_t	*visibleHull;	// convex hull of all visible fragments

	struct shaderInfo_s	*shaderInfo;

	int			contents;		// from shaderInfo
	int			surfaceFlags;	// from shaderInfo
	int			value;			// from shaderInfo

	qboolean	visible;		// choose visble planes first
	qboolean	bevel;			// don't ever use for bsp splitting, and don't bother
								// making windings for it
	qboolean	backSide;		// generated side for a q3map_backShader
} side_t;


#define	MAX_BRUSH_SIDES		1024

typedef struct bspbrush_s {
	struct bspbrush_s	*next;

	int			entitynum;			// editor numbering
	int			brushnum;			// editor numbering

	struct shaderInfo_s	*contentShader;

	int			contents;
	qboolean	detail;
	qboolean	opaque;
	int			outputNumber;		// set when the brush is written to the file list

	int			portalareas[2];

	struct bspbrush_s	*original;	// chopped up brushes will reference the originals

	vec3_t		mins, maxs;
	int			numsides;
	side_t		sides[6];			// variably sized
} bspbrush_t;



typedef struct drawsurf_s {
	shaderInfo_t	*shaderInfo;

	bspbrush_t	*mapBrush;			// not valid for patches
	side_t		*side;				// not valid for patches

	struct drawsurf_s	*nextOnShader;	// when sorting by shader for lightmaps

	int			fogNum;				// set by FogDrawSurfs

	int			lightmapNum;		// -1 = no lightmap
	int			lightmapX, lightmapY;
	int			lightmapWidth, lightmapHeight;

	int			numVerts;
	drawVert_t	*verts;

	int			numIndexes;
	int			*indexes;

	// for faces only
	int			planeNum;

	vec3_t		lightmapOrigin;		// also used for flares
	vec3_t		lightmapVecs[3];	// also used for flares

	// for patches only
	qboolean	patch;
	int			patchWidth;
	int			patchHeight;

	// for misc_models only
	qboolean	miscModel;

	qboolean	flareSurface;
} mapDrawSurface_t;

typedef struct drawSurfRef_s {
	struct drawSurfRef_s	*nextRef;
	int						outputNumber;
} drawSurfRef_t;

typedef struct node_s {
	// both leafs and nodes
	int				planenum;	// -1 = leaf node
	struct node_s	*parent;
	vec3_t			mins, maxs;	// valid after portalization
	bspbrush_t		*volume;	// one for each leaf/node

	// nodes only
	side_t			*side;		// the side that created the node
	struct node_s	*children[2];
	qboolean		hint;
	int				tinyportals;
	vec3_t			referencepoint;

	// leafs only
	qboolean		opaque;		// view can never be inside
	qboolean		areaportal;
	int				cluster;	// for portalfile writing
	int				area;		// for areaportals
	bspbrush_t		*brushlist;	// fragments of all brushes in this leaf
	drawSurfRef_t	*drawSurfReferences;	// references to patches pushed down

	int				occupied;	// 1 or greater can reach entity
	entity_t		*occupant;	// for leak file testing

	struct portal_s	*portals;	// also on nodes during construction
} node_t;

typedef struct portal_s {
	plane_t		plane;
	node_t		*onnode;		// NULL = outside box
	node_t		*nodes[2];		// [0] = front side of plane
	struct portal_s	*next[2];
	winding_t	*winding;

	qboolean	sidefound;		// false if ->side hasn't been checked
	qboolean	hint;
	side_t		*side;			// NULL = non-visible
} portal_t;

typedef struct {
	node_t		*headnode;
	node_t		outside_node;
	vec3_t		mins, maxs;
} tree_t;



//**********************




// Reference to a type of shader.
class ShaderReference
{
public:
  ShaderReference(const unsigned char *mem)
  {
    memcpy(mName,mem,64);
    memcpy(mUnknown,&mem[64],sizeof(int)*2);
  }
  void GetTextureName(char *tname);

  void GetTextureFullName(char *tname) { strcpy(tname,mName); }

  // try to get shader file file name 
  void GetShaderFileName(char *tname);

private:
  char mName[64];     // hard coded by this size, the shader name.
  int  mUnknown[2];   // unknown 2 integer data in shader reference.
};

// Reference to a entity
class EntityReference
{
public:

  EntityReference(const char *mem,int cnt)
  {
	  mBody.assign(mem,cnt); // store the string
  }

  std::string  mBody;
};


typedef std::vector< ShaderReference > ShaderReferenceVector;
typedef std::vector< EntityReference > EntityReferenceVector;

/* BSP lumps in the order they appear in the header */
enum QuakeLumps
{
  Q3_ENTITIES=0,
  Q3_SHADERREFS,
  Q3_PLANES,
  Q3_NODES,
  Q3_LEAFS,
  Q3_LFACES,
  Q3_LBRUSHES, // leaf brushes
  Q3_MODELS,
  Q3_BRUSHES,
  Q3_BRUSH_SIDES,
  Q3_VERTS,
  Q3_ELEMS,
  Q3_FOG,
  Q3_FACES,
  Q3_LIGHTMAPS,
  Q3_LIGHTGRID,
  Q3_VISIBILITY,
  NUM_LUMPS
};

class QuakeLump
{
public:
  int GetFileOffset(void) const { return mFileOffset; };
  int GetFileLength(void) const { return mFileLength; };
private:
// Exactly comforms to raw data in Quake3 BSP file.
  int mFileOffset;         // offset address of 'lump'
  int mFileLength;         // file length of lump.
};

class QuakeHeader
{
public:
  bool SetHeader(const void *mem); // returns true if valid quake header.
  const void * LumpInfo(QuakeLumps lump,const void *mem,int &lsize,int &lcount);
private:
// Exactly conforms to raw data in Quake3 BSP file.
  int  mId;        // id number.
  int  mVersion;   // version number.
  QuakeLump mLumps[NUM_LUMPS];
};

class QuakeNode : public node_t
{
public:
  QuakeNode(void) { };
  QuakeNode(const int *node);
  int GetLeftChild(void) const { return mLeftChild; };
  int GetRightChild(void) const { return mRightChild; };
  const Rect3d<float>& GetBound(void) const { return mBound; };
  int GetPlane(void) const { return mPlane; };
private:
  int mPlane;          // index to dividing plane.
  int mLeftChild;      // index to left child node.
  int mRightChild;     // index to right child node.
  Rect3d<float> mBound; // bounding box for node.
};

typedef std::vector< QuakeNode > QuakeNodeVector;

class QuakeLeaf
{
public:
  QuakeLeaf(void) { };
  QuakeLeaf(const int *leaf);
  const Rect3d<float>& GetBound(void) const { return mBound; };
  int GetCluster(void) const { return mCluster; };
  int GetFirstFace(void) const { return mFirstFace; };
  int GetFaceCount(void) const { return mFaceCount; };
private:
  int mCluster; // which visibility cluster we are in.
  int mArea;    // unknown ?
  Rect3d<float> mBound; // bounding box for leaf node.
  int mFirstFace;   // index to first face.
  int mFaceCount;   // number of faces.
  int mFirstUnknown; // unknown 'first' indicator.
  int mNumberUnknowns; // number of unknown thingies.
};

typedef std::vector< QuakeLeaf > QuakeLeafVector;


class QuakeVertex
{
public:
  QuakeVertex(void) { };
  QuakeVertex(const int *vert);

  void Get(LightMapVertex &vtx) const;

  void Get(Vector3d<float> &p) const
  {
    p = mPos;
  };

//**private:
  Vector3d<float>   mPos;
  Vector2d<float>   mTexel1;
  Vector2d<float>   mTexel2;
  Vector3d<float>   mNormal;
  unsigned int       mColor;
};

typedef std::vector< QuakeVertex > QuakeVertexVector;

class QuakeModel
{
public:
  QuakeModel(void) { };
  QuakeModel(const int *mem);
private:
  Rect3d<float> mBound;
  int           mFirstFace;
  int           mFcount;
  int           mFirstUnknown;
  int           mUcount;
};

typedef std::vector< QuakeModel > QuakeModelVector;

class QuakeFace
{
public:
  QuakeFace(void) { };
  QuakeFace(const int *face);
  ~QuakeFace(void);

  void Build(const UShortVector &elements,
             const QuakeVertexVector &vertices,
             ShaderReferenceVector &shaders,
             const StringRef &lmPrefix,
             const StringRef &name,
             const StringRef &code,
             VertexMesh &mesh);

  
  bool HasLightMap() const 	{ return mLightmap >= 0; }

private:
  int      mFrameNo;
  unsigned int mShader;       // 'shader' numer used by this face.
  int      mUnknown;      // Unknown integer in the face specification.
  FaceType mType;    // type of face.
  int      mFirstVertice; // index into vertex list.
  int      mVcount;       // number of vertices.
  int      mFirstElement; // start of indexed list
  int      mEcount;       // number of elements.
  int      mLightmap;     // lightmap index
  int      mOffsetX;
  int      mOffsetY;
  int      mSizeX;
  int      mSizeY;
  Vector3d<float> mOrig;
  Rect3d<float>  mBound;
  Vector3d<float> mNormal;
  int        mControlX;
  int        mControlY;
};

typedef std::vector< QuakeFace > QuakeFaceVector;

#endif
