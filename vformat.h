#ifndef VFORMAT_H

#define VFORMAT_H

//############################################################################
//##                                                                        ##
//##  VFORMAT.H                                                             ##
//##                                                                        ##
//##  Defines a LightMapVertex, which is a vertex with two U/V channels.    ##
//##  Also defines a class to organize a triangle soup into an ordered mesh ##
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


#include "vector.h"
#include "stringdict.h"
#include "rect.h"


class QuakeShader;

// mapping a shader texture to VRML ImageTexture DEF Name 
typedef std::map< StringRef, StringRef > TextureDefMap;

// mapping a shader to VRML Appearance DEF Name 
typedef std::map< StringRef, StringRef > AppearanceDefMap;



// VRML 2 export options
class VFormatOptions {

public :

	// FOR DEF USE of nodes 
	TextureDefMap textureDefMap;
	AppearanceDefMap appearanceDefMap;
	
	// for creting DEF names 
	int appearanceCount;
	int textureCount;


	bool vrml2;			// want VRML 2 output 
	bool verbose;		// want verbose output

	bool useMultiTexturing; // emit VRML Contact 3D MultiTexture proposed nodes
	bool useBsp;			// emit BSP tree
	bool tex1;		// if exporting 2 separate files the current path 
	int maxStage;		// max number of stages 
	bool usePng; // use PNG format for images, else BMP
	bool useLighting; // emit VRML 2 unlit geometry 
	bool useMat; // dim world using mat emissive color
	bool useEffects; // emit special effects 
	bool yzFlip; // true VRML Y <=> Z 
  bool noTextureCoordinates; // do not output texture coordinates for vrml2 files

	// printf format for vertex coordinates
	const char * VFORMAT;

	// printf format for texture coordinates
	const char * TFORMAT;

	// printf format for vertex colors
	const char * CFORMAT;

	// the default light map blending mode 
	const char *blendMode;
	const char *stage0Mode;
	const char *lightMapMode;

	bool matDefined;

	VFormatOptions() {
		appearanceCount=0;
		textureCount=0;
		matDefined = false;

		vrml2=true;
		verbose=false;
		useBsp=false;

		useEffects=true;

		usePng = true; // use PNG format for lightmap-images, else BMP
		useLighting = false; // emit VRML 2 unlit geometry 
		useMat = false;

		useMultiTexturing = false; // emit VRML Contact 3D MultiTexture proposed nodes

		maxStage=8;
		yzFlip = true; // true VRML Y <=> Z 

		//VFORMAT ="%f %f %f";
		VFORMAT = "%g %g %g";
		TFORMAT  = "%g %g";
		CFORMAT = "%.3f %.3f %.3f";

		useMat = true;

		if (useMat) blendMode = "mode [\"MODULATE\" \"ADD\" ]";
		else blendMode = "mode [\"REPLACE\" \"MODULATE\" ]";
		//blendMode = "mode [\"MODULATE\" \"MODULATE\" ]"; // too dark
		//blendMode = "mode [\"MODULATE\" \"ADDSIGNED\" ]";

		if (useMat) blendMode = "mode [\"DIFFUSE_ADD,SELECTARG2\" \"MODULATE\" ]";

		stage0Mode= "MODULATE";
		lightMapMode = "MODULATE";

	};

	// map from quake to float coordinates 
	void MapVertex (float v[3]) 
	{
    
		const float recip = (1.0f/45.0f);

		v[0]*=recip;
		float tmp=v[1];
		v[1]=v[2]*recip;
		v[2]=tmp * -recip;
	}

	// map a plane normal
	// fix me 
	void MapVector(const float v[3],float vout[3]) 
	{
    
		const float recip = (45.0f/1.0f);

		vout[0]=v[0]*recip;
		float tmp=v[1];
		vout[1]=v[2]*recip;
		vout[2]=tmp * -recip;
	}


};



class LightMapVertex
{
public:
  LightMapVertex(void)
  {
  };

  LightMapVertex(float x,float y,float z,float u1,float v1,float u2,float v2)
  {
    mPos.Set(x,y,z);
    mTexel1.Set(u1,v1);
    mTexel2.Set(u2,v2);
  }


  void GetPos(Vector3d<float> &pos) const { pos = mPos; };
  const Vector3d<float>& GetPos(void) const { return mPos; };

  float GetX(void) const { return mPos.x; };
  float GetY(void) const { return mPos.y; };
  float GetZ(void) const { return mPos.z; };

  void Lerp(const LightMapVertex &a,const LightMapVertex &b,float p)
  {
    mPos.Lerp(a.mPos,b.mPos,p);
    mTexel1.Lerp(a.mTexel1,b.mTexel1,p);
    mTexel2.Lerp(a.mTexel2,b.mTexel2,p);
    mColor.Lerp(a.mColor,b.mColor,p);
  };

  void Set(int index,const float *pos,const float *texel1,const float *texel2)
  {
    const float * p = &pos[index*3];

    const float * tv1 = &texel1[index*2];
    const float * tv2 = &texel2[index*2];

    mPos.x     = p[0];
    mPos.y     = p[1];
    mPos.z     = p[2];
    mTexel1.x  = tv1[0];
    mTexel1.y  = tv1[1];
    mTexel2.x  = tv2[0];
    mTexel2.y  = tv2[1];
  };

  Vector3d<float> mPos;
  Vector2d<float> mTexel1;
  Vector2d<float> mTexel2;
  Vector3d<float> mColor;

};


typedef std::vector< LightMapVertex > VertexVector;

class VertexLess
{
public:

	bool operator()(int v1,int v2) const;

  static void SetSearch(const LightMapVertex& match,VertexVector *list)
  {
    mFind = match;
    mList = list;
  };

private:
  const LightMapVertex& Get(int index) const
  {
    if ( index == -1 ) return mFind;
	  VertexVector &vlist = *mList;
    return vlist[index];
  }
  static LightMapVertex mFind; // vertice to locate.
  static VertexVector  *mList;
};

typedef std::set<int, VertexLess > VertexSet;

class VertexPool
{
public:

  int GetVertex(const LightMapVertex& vtx)
  {
    VertexLess::SetSearch(vtx,&mVtxs);
    VertexSet::iterator found;
    found = mVertSet.find( -1 );
    if ( found != mVertSet.end() )
    {
      return *found;
    }
    int idx = mVtxs.size();
    assert( idx >= 0 && idx < 65536 );
    mVtxs.push_back( vtx );
    mVertSet.insert( idx );
    return idx;
  };

  void GetPos(int idx,Vector3d<float> &pos) const
  {
    pos = mVtxs[idx].mPos;
  }

  const LightMapVertex& Get(int idx) const
  {
    return mVtxs[idx];
  };

  int GetSize(void) const
  {
    return mVtxs.size();
  };

  void Clear(int reservesize)  // clear the vertice pool.
  {
    mVertSet.clear();
    mVtxs.clear();
    mVtxs.reserve(reservesize);
  };

  const VertexVector& GetVertexList(void) const { return mVtxs; };

  void Set(const LightMapVertex& vtx)
  {
    mVtxs.push_back(vtx);
  }

  int GetVertexCount(void) const
  {
    return mVtxs.size();
  };

  bool GetVertex(int i,Vector3d<float> &vect) const
  {
    vect = mVtxs[i].mPos;
    return true;
  };


  void SaveVRML(FILE *fph,bool tex1);
  void SaveVRML2(FILE *fph,int lightMapStage, VFormatOptions &options);

private:
  VertexSet      mVertSet; // ordered list.
  VertexVector   mVtxs;  // set of vertices.
};


class VertexSection
{
public:
  VertexSection(const StringRef &name)
  {
    mName = name;
	mShader = NULL;
    mBound.InitMinMax();
  };

  void AddTri(const LightMapVertex &v1,
              const LightMapVertex &v2,
              const LightMapVertex &v3);


  void SaveVRML(FILE *fph,bool tex1);
  void SaveVRML2(FILE *fph,VFormatOptions &options);

  void SetShader(QuakeShader	*shader) { mShader = shader; }
  QuakeShader* GetShader(QuakeShader	*shader) { return mShader; }

private:

  void AddPoint(const LightMapVertex &p);

  StringRef     mName;
  Rect3d<float> mBound;
  UShortVector  mIndices;
  VertexPool    mPoints;
  QuakeShader	*mShader; // tmp pointer to shader 
};

typedef std::map< StringRef, VertexSection * > VertexSectionMap;



class VertexMesh
{
public:
  VertexMesh(void)
  {
    mLastSection = 0;
    mBound.InitMinMax();
  };

  ~VertexMesh(void);

  void AddTri(const StringRef &name,
              const LightMapVertex &v1,
              const LightMapVertex &v2,
              const LightMapVertex &v3);

  void SaveVRML(const String &name,  // base file name
                bool tex1) const;          // texture channel 1=(true)

   void SaveVRML2(FILE *fph,
                 VFormatOptions &options) const;   


   // current section in progress 
   VertexSection   *mLastSection;

private:
  StringRef        mLastName;
  VertexSectionMap mSections;
  Rect3d<float>    mBound; // bounding region for whole mesh
};

#endif
