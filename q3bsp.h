#ifndef Q3BSP_H

#define Q3BSP_H

//############################################################################
//##                                                                        ##
//##  Q3BSP.H                                                               ##
//##                                                                        ##
//##  Class to load Quake3 BSP files, interpret them, organize them into    ##
//##  a triangle mesh, and save them out in various ASCII file formats.     ##
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

#include "stl.h" // common STL definitions
#include "q3def.h" // include quake3 data structures.
#include "stringdict.h"
#include "vector.h"

class VFormatOptions;

// Loads a quake3 bsp file.
class Quake3BSP
{
public:
  Quake3BSP(const StringRef &fname,
            const StringRef &code);

  ~Quake3BSP(void);


  VertexMesh * GetVertexMesh(void) const { return mMesh; };


private:
  void ReadFaces(const void *mem); // load all faces (suraces) in the bsp
  void ReadVertices(const void *mem); // load all vertex data.
  void ReadLightmaps(const void *mem); // load all lightmap data.
  void ReadElements(const void *mem); // load indices for indexed primitives
  void ReadShaders(const void *mem); // names of the 'shaders' (i.e. texture)

  // read the array of planes
  void ReadPlanes(const void *mem);

  // read the dnode_t nodes 
  void ReadNodes(const void *mem);

  // read the dleaf_t nodes 
  void ReadLeaves(const void *mem);

  // read the leaf surface indices
  void ReadLeafSurfaces(const void *mem);


  
  void ReadEntities(const void *mem); // entities

  void BuildVertexBuffers(void);

  bool              mOk;       // quake BSP properly loaded.
  StringRef         mName;     // name of quake BSP
  StringRef         mCodeName;     // short reference code for BSP
  QuakeHeader       mHeader;   // header of quake BSP loaded.
  QuakeFaceVector   mFaces;    // all faces
  QuakeVertexVector mVertices; // all vertices.
  ShaderReferenceVector mShaders; // shader references
  UShortVector      mElements; // indices for draw primitives.
  Rect3d<float>     mBound;
  VertexMesh       *mMesh; // organized mesh

  std::vector< dplane_t > mPlanes; // the planes 
  std::vector< dnode_t > mNodes; // the nodes

  std::vector<int>		mLeafSurfaces;
  std::vector<int>		mLeafBrushes;

  std::vector<dbrush_t > mBrushes;
  std::vector<dbrushside_t > mBbrushSides;

  std::vector< dleaf_t > mLeaves; // the leaves

  EntityReferenceVector mEntities;	// list of entities

public :

  bool				mUsePng; // use PNG format for export
  StringRef			mLmPrefix; // prefix used for light map files 


  // save the entities 
  void SaveEntitiesVRML2(
			FILE *fph,
            VFormatOptions &options) const ;

  void SaveEntity(const EntityReference &entity,FILE *f,VFormatOptions &options) const;

  // save the BSP Node tree 
  void SaveNodesBsp(
			FILE *fph,
            VFormatOptions &options);

  void SaveNode(
			int nodeNum, 
			FILE *fph,
            VFormatOptions &options); 


  void SaveNodeBsp(
			const dnode_t *node, 

			FILE *fph,
            VFormatOptions &options);

  void SaveNodeBsp(
			const dleaf_t *node, 

			FILE *fph,
            VFormatOptions &options);

};

#endif
