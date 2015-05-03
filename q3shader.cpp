#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


//############################################################################
//##                                                                        ##
//##  Q3SHADER.CPP                                                          ##
//##                                                                        ##
//##  Reads a Quake3 Shader file.                                           ##
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

#include "q3shader.h"
#include "fload.h"

QuakeShaderFactory *QuakeShaderFactory::gSingleton=0; // global instance of data

QuakeShaderFactory::QuakeShaderFactory(void)
{
#if 0
// try to load dynamic 
  // shader where names not match ed skies ==> sky 	
  AddShader("base.shader");

  AddShader("liquid.shader");
  AddShader("liquid2.shader"); // xx
  AddShader("skin.shader");
  AddShader("sky.shader");
  AddShader("gfx.shader");
  AddShader("gfx2.shader"); //xx
  AddShader("sfx.shader");
  AddShader("sfx2.shader");

#else

// added teamarena3 & nv15 shaders 
  AddShader("sfx.shader");
  AddShader("sfx2.shader");

  AddShader("base.shader");
  AddShader("base_button.shader");
  AddShader("base_floor.shader");
  AddShader("base_floor2.shader"); // xx
  AddShader("base_light.shader");
  AddShader("base_object.shader");
  AddShader("base_support.shader");
  AddShader("base_trim.shader");
  AddShader("base_wall.shader");
  AddShader("base_wall2.shader"); // xx
  AddShader("common.shader");
  AddShader("ctf.shader");
  AddShader("ctf2.shader");	// xx
  AddShader("eerie.shader");
  AddShader("flayer.shader"); //xx 
  AddShader("gallery.shader"); //xx 
  AddShader("gfx.shader");
  AddShader("gfx2.shader"); //xx
  AddShader("gothic_block.shader");
  AddShader("gothic_floor.shader");
  AddShader("gothic_floor2.shader");
  AddShader("gothic_light.shader");
  AddShader("gothic_trim.shader");
  AddShader("gothic_wall.shader");
  AddShader("hell.shader");
  AddShader("jim_test.shader");
  AddShader("jk_dm1.shader");
  AddShader("jk_tourney1.shader");

  AddShader("liquid.shader");
  AddShader("liquid2.shader"); // xx
  AddShader("menu.shader");
  AddShader("models.shader");
  AddShader("models2.shader"); //x
  AddShader("models3.shader"); //x
  AddShader("mre.shader"); //x
  AddShader("multiplant.shader"); //x
  AddShader("museum.shader"); //x
  AddShader("nateleaf.shader"); //x
  AddShader("nateshad.shader"); //x

  AddShader("organics.shader");
  AddShader("outdoors.shader");
  AddShader("proto2.shader");
//  AddShader("sfx.shader");
  //AddShader("sfx2.shader");
  AddShader("shrine.shader");
  AddShader("skin.shader");
  AddShader("sky.shader");
  AddShader("stone2.shader");
  AddShader("team.shader");
  AddShader("terrain.shader");

  AddShader("test.shader");
  AddShader("tim.shader");
  AddShader("ui.shader");
  AddShader("ui_hud.shader");
  AddShader("ui_kc.shader");
  AddShader("work.shader");
  AddShader("work_tri1.shader");

  AddShader("nvidia.shader"); // NV15 

#endif
}

QuakeShaderFactory::~QuakeShaderFactory(void)
{
  QuakeShaderMap::iterator i;
  for (i=mShaders.begin(); i!=mShaders.end(); ++i)
  {
    QuakeShader *shader = (*i).second;
    delete shader;
  }
}

QuakeShader * QuakeShaderFactory::Locate(const String &str)
{
  return Locate(StringDict::gStringDict().Get(str));
}

QuakeShader * QuakeShaderFactory::Locate(const StringRef &str)
{
  QuakeShaderMap::iterator found;
  found = mShaders.find(str);
  if ( found != mShaders.end() ) return (*found).second;
  return 0;
}


bool 
QuakeShaderFactory::ShaderFileLoaded(const StringRef &str)
{
  QuakeShaderFileMap::iterator found;
  found = mShaderFiles.find(str);
  if ( found != mShaderFiles.end() ) 
	  return (*found).second;
  return false;
}


bool  QuakeShaderFactory::AddShader(const StringRef &sname)
{

  mBraceCount = 0;
  mCurrent    = 0;

  mCurrentStage = 0;

  Fload shader("scripts/"+String(sname));

  if (!shader.GetData()) {

	//String sname1;
	//sname1 = "scripts/" + sname;

	printf("***********SHADER FILE NOT FOUND in scripts/ : %s \n",sname.c_str());
	return false;;
  }	


  mShaderFiles[sname] = true; 

  printf("***********SHADER FILE PROCESS : %s \n",sname.c_str());

  while ( 1 )
  {

    char *str = shader.GetString();

    if ( !str ) break;

    ShaderString(str); // process one string.
  }

  delete mCurrent;
  mCurrent = 0;
  delete mCurrentStage;
  mCurrentStage = 0;
  return true;

}

void QuakeShaderFactory::ShaderString(const char *str)
{
  char workspace[1024];

  char *dest = workspace;

  while ( *str )
  {
    if ( str[0] == '/' && str[1] == '/' ) 
		break;
    *dest++ = *str++;
  }
  *dest = 0;
  int len = strlen(workspace);
  if ( len == 0 ) return;

  ArgList::Set( workspace ); // crunch it into arguments.
  // now ready to process it as a series of arguments!
  if ( mArgs.size() ) Process(mArgs);
}


// bad code, but what 
void strlwr(const String &l,String &dest)
{
 char workspace[1024];
 strcpy(workspace,l.c_str());
 dest = strlwr(workspace);
}
void strupr(const String &l,String &dest)
{
 char workspace[1024];
 strcpy(workspace,l.c_str());
 dest = strupr(workspace);
}

void QuakeShaderFactory::Process(const StringVector &args)
{

  if ( args[0] == "{" )
  {
    mBraceCount++;
  }
  else
  {
    if ( args[0] == "}" )
    {
      mBraceCount--;

      if ( mBraceCount == 0 ) // shader finished 
      {
        if ( mCurrent )
        {
          const StringRef& ref = mCurrent->GetName();
          QuakeShaderMap::iterator found;
          found = mShaders.find(ref);
          if ( found != mShaders.end() )
          {
            printf("Can't add shader %s, it already exists!!\n",ref);
          }
          else
          {
            mShaders[ref] = mCurrent;
            printf("Added shader: %s\n",ref);
            mCurrent = 0;
          }
        }
        else
        {
          printf("Got closing brace without valid shader defined.\n");
        }
      }
      else
      {
		  if ( mBraceCount == 1 ) {// shader stage finished        
			if (mCurrent && mCurrentStage)
				mCurrent->AddStage(mCurrentStage);
			delete mCurrentStage;
			mCurrentStage = 0;

		} 
		else  if ( mBraceCount < 0 )
        {
          printf("Missmatched closing brace situation!??\n");
          mBraceCount = 0;
        }
      }
    }
    else
    {
      if ( !mBraceCount )
      {
        delete mCurrent; // if didn't process the last one
        mCurrent = 0;
		delete mCurrentStage; // if didn't process the last one
        mCurrentStage = 0;

		// new shader start
        char name[256];
        //if ( GetName(args[0],name))
		strcpy(name,args[0].c_str()); // we take the full path name 
        {
          StringRef ref = StringDict::gStringDict().Get(name);
          mCurrent = new QuakeShader(ref);
        }
      }
      else
      if (mCurrent) {
		  if (mBraceCount == 1) { // at shader level 
		    if ( args[0] == "cull" && args.size() == 2)
	        {		// disable none trans alphashadow nomarks
				 mCurrent->mCull = args[1];
			}
		    else if ( args[0] == "surfaceparm" && args.size() == 2)
	        {
				if ( args[1] == "nolightmap" )
					 mCurrent->mNoLightMap = true;
				else if ( args[1] == "sky" )
					 mCurrent->mSky = true;
			}
			else if ( args[0] == "skyparms")
	        {

			}
		  } else 	
		  if (mBraceCount == 2) {	// at stage lavel 
		    if (!mCurrentStage) 
				mCurrentStage = new ShaderStage;


			String arg0; 
			strlwr(args[0],arg0);

			// process command!
			// store data in mCurrentStage

			if ( arg0 == "map"  && args.size() == 2  )
			{
			  //char name[256];
			  // this strips the extension ?	
			  //if ( GetName(args[1],name) )
			  if (args[1] != "$lightmap")	
			  {
				const StringRef ref = StringDict::gStringDict().Get(args[1]);
				mCurrentStage->map = ref;
				mCurrent->AddTexture(ref);
				//printf("Adding texture %s\n",ref.Get());
			  } else {
				  mCurrentStage->isLightMap = true;
			  }	
			}
			else if ( arg0 == "clampmap"  && args.size() == 2  )
			{
			  if (args[1] != "$lightmap")	
			  {
				const StringRef ref = StringDict::gStringDict().Get(args[1]);
				mCurrentStage->map = ref;
 				mCurrentStage->clamp = true;

				mCurrent->AddTexture(ref);
				//printf("Adding texture %s\n",ref.Get());
			  } else {
				  mCurrentStage->isLightMap = true;
				  //if (mCurrentStage->textureBlendMode == String(""))
				  //mCurrentStage->textureBlendMode = "ADD";
			  }	
			}
			else if ( arg0 == "animmap") {
				mCurrentStage->isAnimMap = true;
				mCurrentStage->animMapFrequency = atof(args[1].c_str());
				for (int i=2; i< args.size(); i++) 
					mCurrentStage->animMap.push_back(args[i]);
			}
			else if ( arg0 == "blendfunc")
			{
					//blendFunc GL_DST_COLOR GL_ZERO
					//blendFunc GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
					//blendFunc GL_ONE GL_ONE
					//blendFunc GL_ONE GL_ZERO
					//blendFunc blend
					//blendFunc filter
					// [Source * <srcBlend>] + [Destination * <dstBlend>]
					if (args.size() == 3) {

						String arg1; 
						strupr(args[1],arg1);
						String arg2; 
						strupr(args[2],arg2);


						mCurrentStage->blendFuncSrc = arg1;
						mCurrentStage->blendFuncDst = arg2;

						if (arg1=="GL_SRC_ALPHA")
							mCurrentStage->textureBlendMode = "BLENDTEXTUREALPHA";

						if (arg1=="GL_ONE" &&  arg2=="GL_ZERO")
							mCurrentStage->textureBlendMode = "REPLACE";
						else 
						if (arg1=="GL_ONE" &&  arg2=="GL_ONE")
							mCurrentStage->textureBlendMode = "ADD";
						else 
						if (arg1=="GL_SRC_ALPHA" && arg2=="GL_ONE_MINUS_SRC_ALPHA")
							mCurrentStage->textureBlendMode = "BLENDTEXTUREALPHA";
						else 
						if (arg1=="GL_DST_COLOR" &&  arg2 =="GL_ONE_MINUS_DST_ALPHA")
							//mCurrentStage->textureBlendMode = "BLENDCURRENTALPHA"; // ??? 
							mCurrentStage->textureBlendMode = "MODULATE"; // ??? 
						else if (arg1=="GL_DST_COLOR" &&  arg2=="GL_ZERO")
							mCurrentStage->textureBlendMode = "MODULATE"; 


					   // GL_DST_COLOR GL_ONE_MINUS_DST_ALPHA
					   // GL_DST_COLOR GL_ZERO
					}
					else mCurrentStage->blendFuncSrc = args[1];
					if (args[1]=="add") {
						mCurrentStage->textureBlendMode = "ADD";
						mCurrentStage->blendFuncSrc = "GL_ONE";
						mCurrentStage->blendFuncDst = "GL_ONE";
					} else 
					if (args[1]=="filter") {
						mCurrentStage->textureBlendMode = "MODULATE";
						mCurrentStage->blendFuncSrc = "GL_DST_COLOR";
						mCurrentStage->blendFuncDst = "GL_ZERO";
					} else 
					if (args[1]=="blend") { // ??
						mCurrentStage->textureBlendMode = "BLENDTEXTUREALPHA";
						mCurrentStage->blendFuncSrc = "GL_SRC_ALPHA";
						mCurrentStage->blendFuncDst = "GL_ONE_MINUS_SRC_ALPHA";
					}

					//tcmod scroll 0 1
					//tcMod turb 0 .25 0 5.6
					//tcmod scale 1.5 1.5
					//tcGen environment
					//tcMod scroll 0.15 0.15
					//depthWrite
					//map $lightmap
					
					//rgbGen identity
					//rgbGen wave sin .5 .5 0 .5	
					//depthfunc equal



			}
			else if (arg0 == "alphafunc") 
			{  // GE128 
			}
			else if (arg0 == "tcmod")
			{
				if (mCurrentStage->tcmod.length()>0) 	mCurrentStage->tcmod += ',';

				if (args[1] == "scale" || args[1] == "scroll" || args[1] == "rotate") {
					for (int i=1; i< args.size(); i++) {
						mCurrentStage->tcmodOk += args[i];
						mCurrentStage->tcmodOk += ' ';
					}

				} else {
					for (int i=1; i< args.size(); i++) {
						mCurrentStage->tcmod += args[i];
						mCurrentStage->tcmod += ' ';
					}
				}
			}
			else if (arg0 == "tcgen")
			{
				if (mCurrentStage->tcmod.length()>0) 	mCurrentStage->tcmod += ',';
				mCurrentStage->tcmod += "tcgen ";
				for (int i=1; i< args.size(); i++) {
					mCurrentStage->tcmod += args[i];
					mCurrentStage->tcmod += ' ';
				}
			}
			else if (arg0 == "rgbgen")
			{
				// identityLighting
				// identity
				// wave <func> <base> <amp> <phase> <freq>
				for (int i=1; i< args.size(); i++) {
					if (i>1) mCurrentStage->rgbGen += ' ';
					mCurrentStage->rgbGen += args[i];
				}
			}
			else {

			}
		  } // braceCount == 2
      }
    }
  }
}

bool QuakeShaderFactory::GetName(const String &str,char *tname)
{
  int len = str.size();
  if ( !len ) return false;
  const char *foo = str.c_str();
  foo = &foo[len-1];
  while ( *foo && *foo != '/' ) foo--;
  if ( !*foo ) return false;
  foo++;
  char *dest = tname;
  while ( *foo )
  {
    *dest++ = *foo++;
  }
  *dest = 0;
  
  len = strlen(tname);
  if ( len >= 4 )
  {
    if ( tname[len-4] == '.' )
    {
      tname[len-4] = 0;
    }
  }
  return true;
}
