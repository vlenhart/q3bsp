#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>


//############################################################################
//##                                                                        ##
//##  VFORMAT.CPP                                                           ##
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


#include "vformat.h"

LightMapVertex VertexLess::mFind;
VertexVector * VertexLess::mList=0;

bool VertexLess::operator()(int v1,int v2) const
{

  const LightMapVertex& a = Get(v1);
  const LightMapVertex& b = Get(v2);

  if ( a.GetX() < b.GetX() ) return true;
  if ( a.GetX() > b.GetX() ) return false;

  if ( a.GetY() < b.GetY() ) return true;
  if ( a.GetY() > b.GetY() ) return false;

  if ( a.GetZ() < b.GetZ() ) return true;
  if ( a.GetZ() > b.GetZ() ) return false;

  if ( a.mTexel1.x < b.mTexel1.x ) return true;
  if ( a.mTexel1.x > b.mTexel1.x ) return false;

  if ( a.mTexel1.y < b.mTexel1.y ) return true;
  if ( a.mTexel1.y > b.mTexel1.y ) return false;

  if ( a.mTexel2.x < b.mTexel2.x ) return true;
  if ( a.mTexel2.x > b.mTexel2.x ) return false;

  if ( a.mTexel2.y < b.mTexel2.y ) return true;
  if ( a.mTexel2.y > b.mTexel2.y ) return false;


  return false;
};


VertexMesh::~VertexMesh(void)
{
  VertexSectionMap::iterator i;
  for (i=mSections.begin(); i!=mSections.end(); ++i)
  {
    VertexSection *section = (*i).second;
    delete section;
  }
}


void VertexMesh::AddTri(const StringRef &name,const LightMapVertex &v1,const LightMapVertex &v2,const LightMapVertex &v3)
{
  VertexSection *section;

  if ( name == mLastName )
  {
    section = mLastSection;
  }
  else
  {
    VertexSectionMap::iterator found;
    found = mSections.find( name );
    if ( found != mSections.end() )
    {
      section = mLastSection = (*found).second;
      mLastName = name;
    }
    else
    {
      mLastSection = section = new VertexSection( name );
      mSections[name] = section;
      mLastName = name;
    }
  }

  assert( section );

  section->AddTri( v1, v2, v3 );

  mBound.MinMax( v1.mPos );
  mBound.MinMax( v2.mPos );
  mBound.MinMax( v3.mPos );

}


void VertexSection::AddTri(const LightMapVertex &v1,
            const LightMapVertex &v2,
            const LightMapVertex &v3)
{
  mBound.MinMax(v1.mPos);
  mBound.MinMax(v2.mPos);
  mBound.MinMax(v3.mPos);

  AddPoint(v1);
  AddPoint(v2);
  AddPoint(v3);

}


void VertexSection::AddPoint(const LightMapVertex &p)
{
  unsigned short idx = (unsigned short)mPoints.GetVertex(p);
  mIndices.push_back(idx);
};


void VertexMesh::SaveVRML(const String &name,  // base file name
              bool tex1) const           // texture channel 1=(true)
{
  if ( mSections.size() )
  {
    String oname = name+".wrl";
    FILE *fph = fopen(oname.c_str(),"wb");

    if ( fph )
    {
      fprintf(fph,"#VRML V1.0 ascii\n");
      fprintf(fph,"Separator {\n");
      fprintf(fph,"  ShapeHints {\n");
      fprintf(fph,"    shapeType SOLID\n");
      fprintf(fph,"    vertexOrdering COUNTERCLOCKWISE\n");
      fprintf(fph,"    faceType CONVEX\n");
      fprintf(fph,"  }\n");

      VertexSectionMap::const_iterator i;

      for (i=mSections.begin(); i!=mSections.end(); ++i)
      {
        (*i).second->SaveVRML(fph,tex1);
      }

      fprintf(fph,"}\n");

      fclose(fph);
    }
  }
}


void VertexSection::SaveVRML(FILE *fph,bool tex1)
{
  // save it into a VRML file!
  static int itemcount=1;

  fprintf(fph,"DEF item%d Separator {\n",itemcount++);
  fprintf(fph,"Translation { translation 0 0 0 }\n");
  fprintf(fph,"Material {\n");
  fprintf(fph,"  ambientColor 0.1791 0.06536 0.06536\n");
  fprintf(fph,"  diffuseColor 0.5373 0.1961 0.1961\n");
  fprintf(fph,"  specularColor 0.9 0.9 0.9\n");
  fprintf(fph,"  shininess 0.25\n");
  fprintf(fph,"  transparency 0\n");
  fprintf(fph,"}\n");
  fprintf(fph,"Texture2 {\n");


  const char *foo = mName;
  char scratch[256];

  if ( !tex1 )
  {
    while ( *foo && *foo != '+' ) foo++;
    char *dest = scratch;
    if ( *foo == '+' )
    {
      foo++;
      while ( *foo ) *dest++ = *foo++;
    }
    *dest = 0;
  }
  else
  {
    char *dest = scratch;
    while ( *foo && *foo != '+' ) *dest++ = *foo++;
    *dest = 0;
  }

  if ( tex1 )
    fprintf(fph,"  filename %c%s.tga%c\n",0x22,scratch,0x22);
  else
    fprintf(fph,"  filename %c%s.bmp%c\n",0x22,scratch,0x22);

  fprintf(fph,"}\n");

  mPoints.SaveVRML(fph,tex1);

  int tcount = mIndices.size()/3;

  if ( 1 )
  {
    fprintf(fph,"  IndexedFaceSet { coordIndex [\n");
    UShortVector::iterator j= mIndices.begin();
    for (int i=0; i<tcount; i++)
    {
      int i1 = *j;
      j++;
      int i2 = *j;
      j++;
      int i3 = *j;
      j++;
      if ( i == (tcount-1) )
        fprintf(fph,"  %d, %d, %d, -1]\n",i1,i2,i3);
      else
        fprintf(fph,"  %d, %d, %d, -1,\n",i1,i2,i3);
    }
  }

  if ( 1 )
  {
    fprintf(fph,"  textureCoordIndex [\n");
    UShortVector::iterator j= mIndices.begin();
    for (int i=0; i<tcount; i++)
    {
      int i1 = *j;
      j++;
      int i2 = *j;
      j++;
      int i3 = *j;
      j++;
      if ( i == (tcount-1) )
        fprintf(fph,"  %d, %d, %d, -1]\n",i1,i2,i3);
      else
        fprintf(fph,"  %d, %d, %d, -1,\n",i1,i2,i3);
    }
  }
  fprintf(fph,"  }\n");
  fprintf(fph,"}\n");
};


void VertexPool::SaveVRML(FILE *fph,bool tex1)
{
  if ( 1 )
  {
    fprintf(fph,"  Coordinate3 { point [\n");
    int count = mVtxs.size();
    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];
      if ( i == (count-1) )
        fprintf(fph,"  %f %f %f]\n",vtx.mPos.x,vtx.mPos.y,vtx.mPos.z);
      else
        fprintf(fph,"  %f %f %f,\n",vtx.mPos.x,vtx.mPos.y,vtx.mPos.z);
    }
    fprintf(fph,"   }\n");
  }
  if ( 1 )
  {
    fprintf(fph,"  TextureCoordinate2 { point [\n");
    int count = mVtxs.size();

    for (int i=0; i<count; i++)
    {
      const LightMapVertex &vtx = mVtxs[i];

      if ( !tex1 ) // if saving second U/V channel.
      {
        if ( i == (count-1) )
          fprintf(fph,"  %f %f]\n",vtx.mTexel2.x,1.0f-vtx.mTexel2.y);
        else
          fprintf(fph,"  %f %f,\n",vtx.mTexel2.x,1.0f-vtx.mTexel2.y);
      }
      else
      {
        if ( i == (count-1) )
          fprintf(fph,"  %f %f]\n",vtx.mTexel1.x,1.0f-vtx.mTexel1.y);
        else
          fprintf(fph,"  %f %f,\n",vtx.mTexel1.x,1.0f-vtx.mTexel1.y);
      }
    }

    fprintf(fph,"   }\n");
  }
}
