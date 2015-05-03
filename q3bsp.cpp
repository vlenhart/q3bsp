#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

//############################################################################
//##                                                                        ##
//##  Q3BSP.CPP                                                             ##
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

#include "q3bsp.h"
#include "q3shader.h"
#include "patch.h"

#include "fload.h"
#include "loadbmp.h"


Quake3BSP::Quake3BSP(const StringRef &fname,
                     const StringRef &code)
{
  mMesh = 0;

  mOk = false;
  mName     = fname;
  mCodeName = code;

  mUsePng = true; // hg 

  String ifoo = fname;
  String str = ifoo+".bsp";

  // hg prefix for light map files 
  const char * del = strrchr(fname,'\\');
  if (del) mLmPrefix = (del+1); // use file name part only
  else mLmPrefix=fname.Get();

  Fload data(str);

  void *mem = data.GetData();
  int   len = data.GetLen();
  if ( mem )
  {

    mOk = mHeader.SetHeader(mem);

    if ( mOk )
    {
      ReadFaces(mem);
      ReadElements(mem);
      ReadVertices(mem);
      ReadLightmaps(mem);
      ReadShaders(mem);
      BuildVertexBuffers();

	  ReadPlanes(mem);
	  ReadLeaves(mem);
	  ReadLeafSurfaces(mem);

	  ReadNodes(mem);
	  // brushes 
	  ReadEntities(mem);
    }
  }
}

Quake3BSP::~Quake3BSP(void)
{
  delete mMesh;
}



bool QuakeHeader::SetHeader(const void *mem)  // returns true if valid quake header.
{
  const int *ids = (const int *) mem;
  #define BSPHEADERID  (*(int*)"IBSP")
  #define BSPVERSION 46
  if ( ids[0] != BSPHEADERID ) return false;
  if ( ids[1] != BSPVERSION ) return false;
  mId      = ids[0];
  mVersion = ids[1];
  const QuakeLump *lump = (const QuakeLump *) &ids[2];
  for (int i=0; i<NUM_LUMPS; i++) mLumps[i] = *lump++;
  return true;
}

void Quake3BSP::ReadShaders(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;

  const unsigned char *smem = (const unsigned char *) mHeader.LumpInfo(Q3_SHADERREFS,mem,lsize,lcount);

  mShaders.clear();
  mShaders.reserve( lcount );

  for (int i=0; i<lcount; i++)
  {
    ShaderReference s(smem);
    mShaders.push_back(s);
    smem+=lsize;
  }

}

const void * QuakeHeader::LumpInfo(QuakeLumps lump,
                                 const void *mem,
                                 int &lsize,
                                 int &lcount)
{

  switch ( lump )
  {
   case LUMP_ENTITIES :
      lsize = 1;
      break;

    case Q3_SHADERREFS: // LUMP_SHADERS
      lsize = sizeof(dshader_t); 
      break;
    case Q3_PLANES: // LUMP_PLANES
      lsize = sizeof(float)*4; // size of a plane equations.
      break;
    case Q3_NODES: //LUMP_NODES
      lsize = sizeof(dnode_t); // sizeof(int)*9;
      break;
    case Q3_LEAFS:
      lsize = sizeof(dleaf_t); // sizeof(int)*12;
      break;
    case Q3_FACES:
      lsize = sizeof(int)*26;
      break;
    case Q3_VERTS:
      lsize = sizeof(int)*11;
      break;
    case Q3_LFACES:
      lsize = sizeof(int);
      break;
    case Q3_ELEMS:
      lsize = sizeof(int);
      break;
    case Q3_MODELS:
      lsize = sizeof(int)*10;
      break;
    case Q3_LIGHTMAPS:
      lsize = 1;
      break;
    case Q3_VISIBILITY:
      lsize = 1;
      break;
    default:
      assert( 0 ); // unsupported query type
  }

  int flen = mLumps[lump].GetFileLength();

  assert( flen );
  assert( (flen%lsize) == 0 ); // must be evenly divisible by lump size.
  lcount = flen/lsize; // number of lump items.

  const char *foo = (const char *)mem;
  foo = &foo[ mLumps[lump].GetFileOffset() ];
  return (const void *) foo;
}

QuakeNode::QuakeNode(const int *node)
{
  const dnode_t *n = (const dnode_t*) node;

  mPlane = node[0];
  mLeftChild = node[1];
  mRightChild = node[2];

  mBound.r1.x = float( node[3] );
  mBound.r1.y = float( node[4] );
  mBound.r1.z = float( node[5] );
  mBound.r2.x = float( node[6] );
  mBound.r2.y = float( node[7] );
  mBound.r2.z = float( node[8] );

}

QuakeLeaf::QuakeLeaf(const int *node)
{
  const dleaf_t *n = (const dleaf_t*) node;

  mCluster = node[0];
  mArea    = node[1];

  mBound.r1.x = float( node[2] );
  mBound.r1.y = float( node[3] );
  mBound.r1.z = float( node[4] );

  mBound.r2.x = float( node[5] );
  mBound.r2.y = float( node[6] );
  mBound.r2.z = float( node[7] );

  mFirstFace      = node[8];
  mFaceCount      = node[9];
  mFirstUnknown   = node[10];
  mNumberUnknowns = node[11];
}


// read the array of planes
void Quake3BSP::ReadPlanes(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;
  const dplane_t *planes = (const dplane_t *) mHeader.LumpInfo(Q3_PLANES,mem,lsize,lcount);

  mPlanes.assign(planes,planes+lcount);

}
// read the dnode_t nodes 
void Quake3BSP::ReadNodes(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;
  const dnode_t *nodes= (const dnode_t *) mHeader.LumpInfo(Q3_NODES,mem,lsize,lcount);


  mNodes.assign(nodes,nodes+lcount);

}

// read the dleaf_t nodes 
void Quake3BSP::ReadLeaves(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;
  const dleaf_t *nodes= (const dleaf_t *) mHeader.LumpInfo(Q3_LEAFS,mem,lsize,lcount);


  mLeaves.assign(nodes,nodes+lcount);
  assert(mLeaves.size() == lcount);

}

// read the leaf surface indices
void Quake3BSP::ReadLeafSurfaces(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;
  const int *nodes= (const int *) mHeader.LumpInfo(Q3_LFACES,mem,lsize,lcount);


  mLeafSurfaces.assign(nodes,nodes+lcount);

}


QuakeFace::QuakeFace(const int *face)
{

  const dsurface_t *n = (const dsurface_t *) face;

  mFrameNo = 0;

  const float *fface = (const float *) face;

  mShader       = face[0];
  mUnknown      = face[1];

  mType         = (FaceType)face[2];

  mFirstVertice = face[3];
  mVcount       = face[4];
  mFirstElement = face[5];
  mEcount       = face[6];
  mLightmap     = face[7];
  mOffsetX      = face[8];
  mOffsetY      = face[9];
  mSizeX        = face[10];
  mSizeY        = face[11];

  mOrig.x       = fface[12];
  mOrig.y       = fface[13];
  mOrig.z       = fface[14];

  mBound.r1.x   = fface[15];
  mBound.r1.y   = fface[16];
  mBound.r1.z   = fface[17];

  mBound.r2.x   = fface[18];
  mBound.r2.y   = fface[19];
  mBound.r2.z   = fface[20];

  mNormal.x     = fface[21];
  mNormal.y     = fface[22];
  mNormal.z     = fface[23];

  mControlX     = face[24];
  mControlY     = face[25];

  static int faceno=0;

  if ( mType == FACETYPE_TRISURF )
  {
    printf("Face: %d\n",faceno++);
    printf("Shader: %d\n",mShader);
    printf("Unknown: %d\n",mUnknown);
    printf("Type: %d\n",mType);
    printf("FirstVertice: %d\n",mFirstVertice);
    printf("Vcount: %d\n",mVcount);
    printf("FirstElement: %d\n",mFirstElement);
    printf("Ecount: %d\n",mEcount);
    printf("Lightmap: %d\n",mLightmap);
    printf("OffsetX: %d\n",mOffsetX);
    printf("OffsetY: %d\n",mOffsetY);
    printf("SizeX: %d\n",mSizeX);
    printf("SizeY: %d\n",mSizeY);
    printf("ControlX: %d\n",mControlX);
    printf("ControlY: %d\n",mControlY);
  }


}

QuakeFace::~QuakeFace(void)
{
}

QuakeVertex::QuakeVertex(const int *vert)
{

  const drawVert_t *n = (const drawVert_t *) vert;

  const float *fvert = (const float *) vert;

  mPos.x = fvert[0];
  mPos.y = fvert[1];
  mPos.z = fvert[2];

  mTexel1.x = fvert[3];
  mTexel1.y = fvert[4];

  mTexel2.x = fvert[5];
  mTexel2.y = fvert[6];

  mNormal.x = fvert[7];
  mNormal.y = fvert[8];
  mNormal.z = fvert[9];

  mColor    = (unsigned int) vert[10];

}

void Quake3BSP::ReadFaces(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;
  const int *faces = (const int *) mHeader.LumpInfo(Q3_FACES,mem,lsize,lcount);
  assert( lsize == sizeof(int)*26 );

  mFaces.clear();
  mFaces.reserve( lcount );

  for (int i=0; i<lcount; i++)
  {
    QuakeFace f(faces);

    mFaces.push_back(f);

    faces+=26;
  }
}

void Quake3BSP::ReadVertices(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;
  const int *vertices = (const int *) mHeader.LumpInfo(Q3_VERTS,mem,lsize,lcount);
  assert( lsize == sizeof(int)*11 );

  mVertices.clear();
  mVertices.reserve( lcount );

  mBound.InitMinMax();

  for (int i=0; i<lcount; i++)
  {
    QuakeVertex v(vertices);

    #define RECIP (1.0f/45.0f)

    v.mPos.x*=RECIP;
    v.mPos.y*=-RECIP;
    v.mPos.z*=RECIP;

    mBound.MinMax(v.mPos);
    mVertices.push_back(v);
    vertices+=11;
  }
}

void Quake3BSP::ReadLightmaps(const void *mem)
{
  assert( mOk );

  int lsize;
  int lcount;
  const unsigned char *maps = (const unsigned char *) mHeader.LumpInfo(Q3_LIGHTMAPS,mem,lsize,lcount);
  unsigned char *lmaps = new unsigned char[lcount];
  memcpy(lmaps,maps,lcount);

  for (int j=0; j<lcount; j++)
  {
    unsigned int c = lmaps[j];
    lmaps[j] = (unsigned char) c;
  }

  #define LMWID 128
  #define LMHIT 128
  #define LMSIZE (LMWID*LMHIT*3)

  int texcount = lcount / LMSIZE;

  Bmp bmp;

  unsigned char *map = lmaps;

  for (int i=0; i<texcount; i++)
  {
    char scratch[256];
    sprintf(scratch,"%slm%s%02d",mLmPrefix.Get(),mCodeName,i);
    String sname = scratch;
	if (mUsePng) {
		String bname = sname+".png";
		bmp.SavePNG(bname.c_str(),map,LMWID,LMHIT,3);
	} else {
		String bname = sname+".bmp";
		bmp.SaveBMP(bname.c_str(),map,LMWID,LMHIT,3);
	}
    map+=LMSIZE;
  }


  delete lmaps;

}

void Quake3BSP::ReadElements(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;
  // LUMP_DRAWINDEXES
  const int *elements = (const int *) mHeader.LumpInfo(Q3_ELEMS,mem,lsize,lcount);
  assert( lsize == sizeof(int) );

  mElements.clear();
  mElements.reserve( lcount );

  for (int i=0; i<lcount; i++)
  {
    unsigned short ic = (unsigned short)(*elements);
    mElements.push_back(ic);
    elements++;
  }
}

void QuakeVertex::Get(LightMapVertex &vtx) const
{
  vtx.mPos.x    = mPos.x;
  vtx.mPos.y    = mPos.y;
  vtx.mPos.z    = mPos.z;


  vtx.mTexel1.x = mTexel1.x;
  vtx.mTexel1.y = mTexel1.y;

  vtx.mTexel2.x = mTexel2.x;
  vtx.mTexel2.y = mTexel2.y;

  vtx.mColor.x    = float((mColor>>0) & 0xFF) / 255.0f;
  vtx.mColor.y    = float((mColor>>8) & 0xFF) / 255.0f;
  vtx.mColor.z    = float((mColor>>16) & 0xFF) / 255.0f;
}

void Quake3BSP::BuildVertexBuffers(void)
{
  mMesh = 0;
  mMesh = new VertexMesh;

  QuakeFaceVector::iterator i;
  for (i=mFaces.begin(); i!=mFaces.end(); ++i)
  {
    (*i).Build(mElements,mVertices,mShaders,mLmPrefix,mName,mCodeName,*mMesh);
  }
}

void QuakeFace::Build(const UShortVector &elements,
                      const QuakeVertexVector &vertices,
                      ShaderReferenceVector &shaders,
                      const StringRef &lmPrefix,
                      const StringRef &sourcename,
                      const StringRef &code,
                      VertexMesh &mesh)
{
  assert( mVcount < 1024 );
  assert( mShader >= 0 && mShader < shaders.size() );
  StringRef mat;


  static StringRef oname = StringDict::gStringDict().Get("outside");
  static StringRef inside  = StringDict::gStringDict().Get("inside");

  StringRef name = inside;


  int type = 0;

  LightMapVertex verts[1024];

  for (int i=0; i<mVcount; i++)
  {
    vertices[ i+mFirstVertice ].Get(verts[i]);
  }

  char scratch[256];
  char texname[256];

  //shaders[ mShader ].GetTextureName(texname);
  shaders[ mShader ].GetTextureFullName(texname); // get full name with path

/* hg why ?
  if ( strcmp("toxicskytim_ctf1",texname) == 0 ) 
	  return;
*/

#if 0
  if ( mLightmap < 0 ) 
	  return;
#endif

  StringRef basetexture = StringDict::gStringDict().Get(texname);

  QuakeShader *shader = QuakeShaderFactory::gQuakeShaderFactory().Locate(basetexture);

  if ( shader )
  {
    //shader->GetBaseTexture(basetexture);
    //printf("Shader uses texture: %s\n",basetexture);
  }
  else {

	  if ( strcmp("textures/liquids/lavahell_1000",basetexture) == 0 )  {
	  }
	  if ( strstr(basetexture,"lavahell") )  {
	    printf("shader for : %s\n",basetexture);
	  }

	// try to load a matching shader from disk 
    shaders[mShader].GetShaderFileName(scratch); 
	strcat(scratch,".shader");
    mat = StringDict::gStringDict().Get(scratch);

	if (!QuakeShaderFactory::gQuakeShaderFactory().ShaderFileLoaded(mat)
		&& 	QuakeShaderFactory::gQuakeShaderFactory().AddShader(mat)) 
	{
	     shader = QuakeShaderFactory::gQuakeShaderFactory().Locate(basetexture);
	}

	if (!shader) {
	    printf("No shader for : '%s'\n",basetexture);

	}
  }	
  
  // geometry sorted by shader  string

  if ( mLightmap < 0 ) // no lightmap
	  sprintf(scratch,"%s+",basetexture);
  else	
	sprintf(scratch,"%s+%slm%s%02d",basetexture,lmPrefix,
          code,
          mLightmap);

  mat = StringDict::gStringDict().Get(scratch);

  char lmapspace[256];
  sprintf(lmapspace,"lm%s%02d",code,mLightmap);
  StringRef lmap = SGET(lmapspace);

  switch ( mType )
  {
    case FACETYPE_NORMAL:
    case FACETYPE_TRISURF:
          if ( 1 )
          {
            assert( (mEcount%3) == 0 );
            const unsigned short *idx = &elements[mFirstElement];
            int tcount = mEcount/3;
            for (int j=0; j<tcount; j++)
            {
              int i1 = idx[0];
              int i2 = idx[1];
              int i3 = idx[2];

              mesh.AddTri(mat,verts[i1], verts[i2], verts[i3] );

              idx+=3;
            }
          }
          break;
    case FACETYPE_MESH:
        if ( 1 )
        {
          PatchSurface surface(verts,mVcount,mControlX,mControlY);
          const LightMapVertex *vlist = surface.GetVerts();
          const unsigned short *indices = surface.GetIndices();
          int tcount = surface.GetIndiceCount()/3;

          for (int j=0; j<tcount; j++)
          {

            int i1 = *indices++;
            int i2 = *indices++;
            int i3 = *indices++;

            mesh.AddTri(mat,vlist[i1], vlist[i2], vlist[i3] );

          }
        }
        break;
    case FACETYPE_FLARE:
      break;
    default:
//      assert( 0 );
      break;
  }
  if (mesh.mLastSection && shader) 
	  mesh.mLastSection->SetShader(shader);


}

void ShaderReference::GetTextureName(char *tname)
{
  strcpy(tname,"null");
  int len = strlen(mName);
  if ( !len ) return;
  const char *foo = &mName[len-1];
  while ( *foo && *foo != '/' ) foo--;
  if ( !*foo ) return;
  foo++;
  char *dest = tname;
  while ( *foo )
  {
    *dest++ = *foo++;
  }
  *dest = 0;
}



void ShaderReference::GetShaderFileName(char *tname)
{
  strcpy(tname,"");
  int len = strlen(mName);
  if ( !len ) 
	  return;
  
  const char *foo = &mName[0];
  while ( *foo && *foo != '/' ) foo++;

  if ( !*foo ) return;
  foo++;
  
  char *dest = tname;
  while ( *foo && *foo != '/' )
  {
    *dest++ = *foo++;
  }
  *dest = 0;
}



void Quake3BSP::ReadEntities(const void *mem)
{
  assert( mOk );
  int lsize;
  int lcount;
  
  const char *elements = (const char *) mHeader.LumpInfo(Q3_ENTITIES,mem,lsize,lcount);
  assert( lsize == sizeof(char) );

  mEntities.clear();

  const char *estart=elements;
  const char *p=estart;
  int level=0;
  for (int i=0; i<lcount; i++)
  {
	 if (*p == '{') {
		 level++;
		 if (level == 1) estart = p+1;
	 } else if (*p == '}') {
		 level--;
		 if (level == 0) {
			EntityReference *e=new EntityReference(estart,p-estart-1);

		    mEntities.push_back(*e);
			estart = p+1;
		 }
	 }
	 p++;

  }
}
 
// parse an entity

void Quake3BSP::SaveEntity(const EntityReference &entity,
						   FILE *f
						   ,VFormatOptions &options
						   ) const
{
   
   std::string classname;
   std::string targetname;
   float angle = 0;
   vec3_t origin;
   // targetname	
   // model
   // dmg
   // spawnflags
   //_color
   //message
   //music
 
  ArgList args;
  
  args.Set( entity.mBody.c_str(),false); // crunch it into arguments.
  // now ready to process it as a series of arguments!

  int argi=0;
  while (argi < args.size()) 
  {
	String&  key = args[argi];
	
	argi++;

	if (key == "classname") {
		classname = args[argi];
		argi++;
	}
	else if (key == "targetname") {
		targetname = args[argi];
		argi++;
	}
	else if (key == "origin") {
		sscanf(args[argi].c_str(),"%f %f %f",&origin[0],&origin[1],&origin[2]);
		options.MapVertex(origin);
		argi++;
	}
	else if (key == "angle") {
		sscanf(args[argi].c_str(),"%f",&angle);
		angle -=90;
		angle *= (float)Q_PI/180.0f;

		argi++;
	}
	else {
		argi += 1;
	}

  }	

  //
  // info_player_intermission
  if (classname == "target_position") {
	  fprintf(f,"DEF %s Viewpoint { position %g %g %g fieldOfView 1.0 jump TRUE orientation 0 1 0 %g\n\tdescription %c%s%c }\n",targetname.c_str(),origin[0],origin[1],origin[2],angle,'"',targetname.c_str(),'"');
  }	
  if (classname == "info_player_deathmatch") {
	  fprintf(f,"Viewpoint { position %g %g %g fieldOfView 1.0 jump TRUE orientation 0 1 0 %g\n\tdescription %c%s%c }\n",origin[0],origin[1],origin[2],angle,'"',classname.c_str(),'"');
  }	
  // worldspawn	
  //misc_model .. model


}

// save the entities 
void Quake3BSP::SaveEntitiesVRML2(
			FILE *fph,
            VFormatOptions &options) const 
{
  if ( mEntities.size() )
  {
    if ( fph )
    {
      fprintf(fph,"DEF Entities Group {\n");
      fprintf(fph,"children [\n");

      EntityReferenceVector::const_iterator i;

      for (i=mEntities.begin(); i!=mEntities.end(); ++i)
      {
			SaveEntity(*i,fph,options);
      }

      fprintf(fph,"]}\n");

    }
  }
}

void Quake3BSP::SaveNode(
			int nodeNum, 
			FILE *fph,
            VFormatOptions &options)

{
	if (nodeNum<0) {
		// negative numbers are -(leafs+1), not nodes
		int leafNum = - (nodeNum+1);
		SaveNodeBsp(&mLeaves[leafNum],fph,options);
	}
	else {
		SaveNodeBsp(&mNodes[nodeNum],fph,options);
	}
}

// save the BSP Node tree 
void Quake3BSP::SaveNodesBsp(
			FILE *fph,
            VFormatOptions &options)
{
	if (mNodes.size()>0)
	  SaveNodeBsp(&mNodes[0],fph,options);
}

// save a node 
void Quake3BSP::SaveNodeBsp(
			const dnode_t *node, 

			FILE *fph,
            VFormatOptions &options)
{

	const dplane_t	*plane;
	
	if (node->planeNum != PLANENUM_LEAF) {
		plane = &mPlanes[node->planeNum];
		//d = DotProduct (origin, plane->normal) - plane->dist;
		// if (d >= 0) node = node->children[0];
		vec3_t normal;

		// is this correct ??
		options.MapVector(plane->normal,normal) ;
	
		fprintf(fph,"BspTree { plane %g %g %g %g \n",normal[0], normal[1],normal[2],plane->dist);

		fprintf(fph,"front "); SaveNode(node->children[0],fph,options);

		fprintf(fph,"back ");SaveNode(node->children[1],fph,options);

		fprintf(fph,"}\n");

	}	else {
#if 0
		portal_t	**pp, *t;
	
		pp = &node->portals;
		while (1)
		{
			t = *pp;
			if (!t)
				break;

/*
			if (t->nodes[0] == l)
				pp = &t->next[0];
			else if (t->nodes[1] == l)
				pp = &t->next[1];
			else
				Error ("RemovePortalFromNode: portal not bounding leaf");
*/
		}

		if (!node->brushlist)
			fprintf (fph,"NULL\n");
		else
		{
			for (bb=node->brushlist ; bb ; bb=bb->next) {
				//_printf ("%i ", bb->original->brushnum);
			}
			//_printf ("\n");
		}
		return;
#endif

	}

}

// mm problems:
// save face references in same leaf node ?? 
// where are the portals ??

// save a node 
void Quake3BSP::SaveNodeBsp(
			const dleaf_t *node, 

			FILE *fph,
            VFormatOptions &options) 
{

	// save the leaf surfaces & brushes 
	if (node->numLeafSurfaces == 0) {
		fprintf(fph,"NULL ");
		return;
	}
	if (node->numLeafSurfaces >1) {
		//fprintf(fph,"Group { children [\n");
	}
    VertexMesh mesh;

	// print all the surfaces 
	for (int i=0; i<node->numLeafSurfaces;i++) {
		int surface = node->firstLeafSurface + i;
		surface = mLeafSurfaces[surface];
		fprintf(fph,"## surface %d \n",surface);

		mFaces[surface].Build(mElements,mVertices,mShaders,mLmPrefix,mName,mCodeName,mesh);
	}

	mesh.SaveVRML2(fph,options);

	if (node->numLeafSurfaces >1) {
		//fprintf(fph,"]}");
	}

}
