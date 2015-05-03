
//############################################################################
//##                                                                        ##
//##  LOADBMP.CPP                                                           ##
//##                                                                        ##
//##  Utility routines, reads and writes BMP files.  This is *windows*      ##
//##  specific, the only piece of code that relies on windows and would     ##
//##  need to be ported for other OS implementations.                       ##
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

#include "loadbmp.h"

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

#include "fload.h"

unsigned char * Bmp::LoadBMP(const String &fname, int &wid, int &hit, int &bpp)
{
  Fload data(fname);

  unsigned char *mem = (unsigned char *)data.GetData();
  if ( !mem ) return 0;
  return LoadBMP(mem,wid,hit,bpp);
}




unsigned char *Bmp::LoadBMP(const unsigned char *data, int &wid, int &hit, int &Bpp)
{
  //First things first, init working variables
  if ( !data ) return NULL;
  const unsigned char *p = data;   //working pointer into DIB
  wid = 0;
  hit = 0;
  Bpp = 0;

  //Note: We declare and use a BITMAPINFOHEADER, even though the header
  //  may be of one of the newer types, because we don't need to know the
  //  actual header size in order to find the bitmap bits.  (The offset
  //  is given by bfOffBits)
  BITMAPFILEHEADER *filehdr;
  BITMAPINFOHEADER *infohdr;

  filehdr = (BITMAPFILEHEADER *) p;
  infohdr = (BITMAPINFOHEADER *) (p + sizeof(BITMAPFILEHEADER));
  char *blah = (char*)(infohdr + sizeof(infohdr));

  if ( infohdr->biSize == sizeof(BITMAPCOREHEADER) )
  {
    //Old-style OS/2 bitmap header, we don't support it
    return NULL;
  }
  else
  {
    wid = infohdr->biWidth;
    hit = infohdr->biHeight;
    Bpp = (infohdr->biBitCount / 8);
  }

//  if ( Bpp != 1 && Bpp != 3 ) return NULL;    //We only support 8bit and 24bit files


  //Set pointer to beginning of packed pixel data
  p = data + filehdr->bfOffBits;

  //FIXME: This assumes a non-compressed bitmap (no RLE)
  long siz;
#if 0
  int remainder = wid % 4;
  if (remainder != 0)
    siz = (wid + (4-remainder)) * hit * Bpp;
  else
    siz = wid * hit * Bpp;
#else
  int linesize = ((((Bpp * wid)-1)/4)+1)*4;
  siz = hit * linesize;
#endif
  unsigned char *mem = new unsigned char[siz];

  assert (mem);

  if ( Bpp == 1 )
  {

    const unsigned char *base_source = &p[(hit-1)*wid];
    unsigned char *base_dest   = mem;

    for (int y=0; y<hit; y++)
    {
      unsigned char *dest = base_dest;
      const unsigned char *source = base_source;
      memcpy(dest,source,wid);
      base_dest+=(wid);
      base_source-=(wid);
    }



  }
  else
  {
    const unsigned char *base_source = &p[(hit-1)*wid*3];
    unsigned char *base_dest   = mem;

    for (int y=0; y<hit; y++)
    {
      unsigned char *dest = base_dest;
      const unsigned char *source = base_source;

      for (int x=0; x<wid; x++)
      {
        dest[0] = source[2];
        dest[1] = source[1];
        dest[2] = source[0];
        dest+=3;
        source+=3;
      }

      base_dest+=(wid*3);
      base_source-=(wid*3);


    }

  }
  return mem;
}



void Bmp::SaveBMP(const char *fname,const unsigned char *inputdata, int wid, int hit, int Bpp)
{
  FILE *fph = fopen(fname,"wb");
  if ( !fph ) return;

  unsigned char *data = 0;
  if ( Bpp == 1 )
  {
    data = (unsigned char *) inputdata;
  }
  else
  {
    const unsigned char *source;
    unsigned char *dest   = new unsigned char[wid*hit*3];
    data = dest; // data to save has flipped RGB order.

    int bwid = wid*3;

    source = &inputdata[(hit-1)*bwid];

    for (int y=0; y<hit; y++)
    {
      memcpy(dest,source,bwid);
      dest+=bwid;
      source-=bwid;
    }

    //swap RGB!!
    if ( 1 )
    {
      unsigned char *swap = data;
      int size = wid*hit;
      for (int i=0; i<size; i++)
      {
        unsigned char c = swap[0];
        swap[0] = swap[2];
        swap[2] = c;
        swap+=3;
      }
    }
  }


  BITMAPFILEHEADER filehdr;
  BITMAPINFOHEADER infohdr;

  DWORD offset = sizeof(filehdr) + sizeof(infohdr);
  if ( Bpp == 1 )
  {
    //Leave room in file for the color table
    offset += (256 * sizeof(RGBQUAD));
  }

  DWORD sizeImage;
  int remainder = wid % 4;
  int linesize;
  if (remainder != 0)
    linesize = wid + (4-remainder);
  else
    linesize = wid;
  sizeImage = linesize * hit * Bpp;

  filehdr.bfType = *((WORD*)"BM");
  filehdr.bfSize = offset + sizeImage;
  filehdr.bfReserved1 = 0;
  filehdr.bfReserved2 = 0;
  filehdr.bfOffBits = offset;

  infohdr.biSize = sizeof(infohdr);
  infohdr.biWidth = wid;
  infohdr.biHeight = hit;
  infohdr.biPlanes = 1;
  infohdr.biBitCount = Bpp * 8;
  infohdr.biCompression = BI_RGB;
  infohdr.biSizeImage = sizeImage;
  infohdr.biClrUsed = 0;
  infohdr.biClrImportant = 0;
  infohdr.biXPelsPerMeter = 72;
  infohdr.biYPelsPerMeter = 72;

  unsigned int writtencount = 0;

  fwrite(&filehdr, sizeof(filehdr),1,fph);

  writtencount += sizeof(filehdr);

  fwrite(&infohdr, sizeof(infohdr),1,fph);
  writtencount += sizeof(infohdr);

  if ( Bpp == 1 )
  {
    //Generate a greyscale color table for this image
    RGBQUAD rgbq;
    rgbq.rgbReserved = 0;
    for ( int i=0; i<256; i++ )
    {
      rgbq.rgbBlue = i;
      rgbq.rgbGreen = i;
      rgbq.rgbRed = i;
      fwrite(&rgbq, sizeof(RGBQUAD),1,fph);
      writtencount += sizeof(RGBQUAD);
    }
  }

  unsigned char *p = data;
  for ( unsigned int i = 0; i < sizeImage; i+=linesize)
  {
    fwrite((void*)p, linesize,1,fph);
    p+=linesize;
    writtencount+=linesize;
  }

  assert ( writtencount == filehdr.bfSize );

  if ( Bpp == 3 )
  {
    delete data;
  }

  fclose(fph);
}



void Bmp::SwapRGB(unsigned char *dest, const unsigned char *src, unsigned int size)
{
  const unsigned char *s = src;
  unsigned char *d = dest;

  for ( unsigned int i=0; i< size; i+=3  )
  {
    unsigned char tmp = s[2];   //in case src and dest point to the same place
    d[2] = s[0];
    d[1] = s[1];
    d[0] = tmp;
    d += 3;
    s += 3;
  }
}


void Bmp::SwapVertically(unsigned char *dest, const unsigned char *src, int wid, int hit, int Bpp)
{
  const unsigned char *s = src;

  int rowSize = wid * Bpp;

  unsigned char *d = dest + (rowSize * (hit-1));

  for ( int r=0 ; r < hit; ++r )
  {
    memcpy(d, s, rowSize);
    s += rowSize;
    d -= rowSize;
  }
}



#include "zlib/zlib.h"

#include "libpng/png.h"

/* write a png file */
int Bmp::SavePNG(const char *file_name,const unsigned char *inputdata, int wid, int hit, int Bpp)
{
   
	
   FILE *fp=NULL;
   png_structp png_ptr=NULL;
   png_infop info_ptr=NULL;
   png_colorp palette=NULL;

   int width = wid;
   int height = hit;
   int bit_depth = 8;
   int bytes_per_pixel = Bpp;

   int png_interlace = PNG_INTERLACE_NONE; //  : PNG_INTERLACE_ADAM7

   int color_type = 0;
   if (Bpp==1) {
	   color_type = PNG_COLOR_TYPE_GRAY;
   }	
   else if (Bpp==2) {
	   color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
   }	
   else if (Bpp==3) {
	   color_type = PNG_COLOR_TYPE_RGB;
   }	
   else if (Bpp==4) {
	  color_type = PNG_COLOR_TYPE_RGB_ALPHA;
   }	


   /* open the file */
   fp = fopen(file_name, "wb");
   if (fp == NULL)
      return (ERROR);

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   //png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
   //   png_voidp user_error_ptr, user_error_fn, user_warning_fn);

   png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
      return (ERROR);
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
      return (ERROR);
   }

   /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* If we get here, we had a problem reading the file */
      fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return (ERROR);
   }

   /* set up the output control if you are using standard C streams */
   png_init_io(png_ptr, fp);

#ifdef hilevel
   /* This is the easy way.  Use it if you already have all the
    * image info living info in the structure.  You could "|" many
    * PNG_TRANSFORM flags into the png_transforms integer here.
    */
   png_write_png(png_ptr, info_ptr, png_transforms, NULL);
#else
   /* This is the hard way */

   /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */
   png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
      png_interlace, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

#if 0
   if (palette) {
   /* set the palette if there is one.  REQUIRED for indexed-color images */
   palette = (png_colorp)png_malloc(png_ptr, 256 * sizeof (png_color));
   /* ... set palette colors ... */
   png_set_PLTE(png_ptr, info_ptr, palette, 256);
   /* You must not free palette here, because png_set_PLTE only makes a link to
      the palette that you malloced.  Wait until you are about to destroy
      the png structure. */
   }

   /* optional significant bit chunk */
   /* if we are dealing with a grayscale image then */
   sig_bit.gray = true_bit_depth;
   /* otherwise, if we are dealing with a color image then */
   sig_bit.red = true_red_bit_depth;
   sig_bit.green = true_green_bit_depth;
   sig_bit.blue = true_blue_bit_depth;
   /* if the image has an alpha channel then */
   sig_bit.alpha = true_alpha_bit_depth;
   png_set_sBIT(png_ptr, info_ptr, sig_bit);


   /* Optional gamma chunk is strongly suggested if you have any guess
    * as to the correct gamma of the image.
    */
   png_set_gAMA(png_ptr, info_ptr, gamma);

   /* Optionally write comments into the image */
   text_ptr[0].key = "Title";
   text_ptr[0].text = "Mona Lisa";
   text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[1].key = "Author";
   text_ptr[1].text = "Leonardo DaVinci";
   text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[2].key = "Description";
   text_ptr[2].text = "<long text>";
   text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
#ifdef PNG_iTXt_SUPPORTED
   text_ptr[0].lang = NULL;
   text_ptr[1].lang = NULL;
   text_ptr[2].lang = NULL;
#endif
   png_set_text(png_ptr, info_ptr, text_ptr, 3);

#endif


   /* other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs, */
   /* note that if sRGB is present the gAMA and cHRM chunks must be ignored
    * on read and must be written in accordance with the sRGB profile */

   /* Write the file header information.  REQUIRED */
   png_write_info(png_ptr, info_ptr);

   /* If you want, you can write the info in two steps, in case you need to
    * write your private chunk ahead of PLTE:
    *
    *   png_write_info_before_PLTE(write_ptr, write_info_ptr);
    *   write_my_chunk();
    *   png_write_info(png_ptr, info_ptr);
    *
    * However, given the level of known- and unknown-chunk support in 1.1.0
    * and up, this should no longer be necessary.
    */

   /* Once we write out the header, the compression type on the text
    * chunks gets changed to PNG_TEXT_COMPRESSION_NONE_WR or
    * PNG_TEXT_COMPRESSION_zTXt_WR, so it doesn't get written out again
    * at the end.
    */

   /* set up the transformations you want.  Note that these are
    * all optional.  Only call them if you want them.
    */
#if 0
   /* invert monochrome pixels */
   png_set_invert_mono(png_ptr);

   /* Shift the pixels up to a legal bit depth and fill in
    * as appropriate to correctly scale the image.
    */
   png_set_shift(png_ptr, &sig_bit);

   /* pack pixels into bytes */
   png_set_packing(png_ptr);

   /* swap location of alpha bytes from ARGB to RGBA */
   png_set_swap_alpha(png_ptr);

   /* Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
    * RGB (4 channels -> 3 channels). The second parameter is not used.
    */
   png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE);

   /* flip BGR pixels to RGB */
   png_set_bgr(png_ptr);

   /* swap bytes of 16-bit files to most significant byte first */
   png_set_swap(png_ptr);

   /* swap bits of 1, 2, 4 bit packed pixel formats */
   png_set_packswap(png_ptr);

   /* turn on interlace handling if you are not using png_write_image() */
   if (interlacing)
      number_passes = png_set_interlace_handling(png_ptr);
   else
      number_passes = 1;

#endif 


   /* The easiest way to write the image (you may have a different memory
    * layout, however, so choose what fits your needs best).  You need to
    * use the first method if you aren't handling interlacing yourself.
    */
   png_uint_32 k;
   // png_byte image[height][width*bytes_per_pixel];
   //png_bytep row_pointers[height];
   png_bytep *row_pointers = new png_bytep[height];

   for (k = 0; k < height; k++)
     row_pointers[k] = ((png_bytep)inputdata) + k*width*bytes_per_pixel;

   /* One of the following output methods is REQUIRED */
#if 1 // def entire /* write out the entire image data in one call */
   png_write_image(png_ptr, row_pointers);

   delete row_pointers;

   /* the other way to write the image - deal with interlacing */

#else no_entire /* write out the image data by one or more scanlines */
   /* The number of passes is either 1 for non-interlaced images,
    * or 7 for interlaced images.
    */
   for (pass = 0; pass < number_passes; pass++)
   {
      /* Write a few rows at a time. */
      png_write_rows(png_ptr, &row_pointers[first_row], number_of_rows);

      /* If you are only writing one row at a time, this works */
      for (y = 0; y < height; y++)
      {
         png_write_rows(png_ptr, &row_pointers[y], 1);
      }
   }
#endif no_entire /* use only one output method */

   /* You can write optional chunks like tEXt, zTXt, and tIME at the end
    * as well.  Shouldn't be necessary in 1.1.0 and up as all the public
    * chunks are supported and you can use png_set_unknown_chunks() to
    * register unknown chunks into the info structure to be written out.
    */

   /* It is REQUIRED to call this to finish writing the rest of the file */
   png_write_end(png_ptr, info_ptr);

#endif hilevel

   /* If you png_malloced a palette, free it here (don't free info_ptr->palette,
      as recommended in versions 1.0.5m and earlier of this example; if
      libpng mallocs info_ptr->palette, libpng will free it).  If you
      allocated it with malloc() instead of png_malloc(), use free() instead
      of png_free(). */
   if (palette) 
	   png_free(png_ptr, palette);
   palette=NULL;

   /* Similarly, if you png_malloced any data that you passed in with
      png_set_something(), such as a hist or trans array, free it here,
      when you can be sure that libpng is through with it. */
   //png_free(png_ptr, trans);
   //trans=NULL;

   /* clean up after the write, and free any memory allocated */
   png_destroy_write_struct(&png_ptr, &info_ptr);

   /* close the file */
   fclose(fp);

   /* that's it */
   return (1);
}



#include "qdefs.h"

// following is from q3 tools source imagelib.c


/*
============================================================================

TARGA IMAGE

============================================================================
*/

typedef struct _TargaHeader {
	unsigned char 	id_length, colormap_type, image_type;
	unsigned short	colormap_index, colormap_length;
	unsigned char	colormap_size;
	unsigned short	x_origin, y_origin, width, height;
	unsigned char	pixel_size, attributes;
} TargaHeader;


/*
=============
LoadTGABuffer
=============
*/
void LoadTGABuffer ( byte *buffer, byte **pic, int *width, int *height, int *bpp)
{
	int		columns, rows, numPixels;
	byte	*pixbuf;
	int		row, column;
	byte	*buf_p;
	TargaHeader	targa_header;
	byte		*targa_rgba;

	*pic = NULL;

	buf_p = buffer;

	targa_header.id_length = *buf_p++;
	targa_header.colormap_type = *buf_p++;
	targa_header.image_type = *buf_p++;
	
	targa_header.colormap_index = LittleShort ( *(short *)buf_p );
	buf_p += 2;
	targa_header.colormap_length = LittleShort ( *(short *)buf_p );
	buf_p += 2;
	targa_header.colormap_size = *buf_p++;
	targa_header.x_origin = LittleShort ( *(short *)buf_p );
	buf_p += 2;
	targa_header.y_origin = LittleShort ( *(short *)buf_p );
	buf_p += 2;
	targa_header.width = LittleShort ( *(short *)buf_p );
	buf_p += 2;
	targa_header.height = LittleShort ( *(short *)buf_p );
	buf_p += 2;
	targa_header.pixel_size = *buf_p++;
	targa_header.attributes = *buf_p++;

	if (targa_header.image_type!=2 
		&& targa_header.image_type!=10
		&& targa_header.image_type != 3 ) 
	{
		Error("LoadTGA: Only type 2 (RGB), 3 (gray), and 10 (RGB) TGA images supported\n");
	}

	if ( targa_header.colormap_type != 0 )
	{
		Error("LoadTGA: colormaps not supported\n" );
	}

	if ( ( targa_header.pixel_size != 32 && targa_header.pixel_size != 24 ) && targa_header.image_type != 3 )
	{
		Error("LoadTGA: Only 32 or 24 bit images supported (no colormaps)\n");
	}

	columns = targa_header.width;
	rows = targa_header.height;
	numPixels = columns * rows;

	if (width)
		*width = columns;
	if (height)
		*height = rows;
	if (bpp)
		*bpp = targa_header.pixel_size;

	int bytesPerPixel = targa_header.pixel_size/8;

	targa_rgba = (unsigned char *) malloc (numPixels*bytesPerPixel);
	*pic = targa_rgba;

	if (targa_header.id_length != 0)
		buf_p += targa_header.id_length;  // skip TARGA image comment
	
	if ( targa_header.image_type==2 || targa_header.image_type == 3 )
	{ 
		// Uncompressed RGB or gray scale image
		for(row=rows-1; row>=0; row--) 
		{
			pixbuf = targa_rgba + row*columns*bytesPerPixel;
			for(column=0; column<columns; column++) 
			{
				unsigned char red,green,blue,alphabyte;
				switch (targa_header.pixel_size) 
				{
					
				case 8:
					blue = *buf_p++;
					green = blue;
					red = blue;
					//*pixbuf++ = red;
					//*pixbuf++ = green;
					*pixbuf++ = blue;
					//*pixbuf++ = 255;
					break;

				case 24:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					//*pixbuf++ = 255;
					break;
				case 32:
					blue = *buf_p++;
					green = *buf_p++;
					red = *buf_p++;
					alphabyte = *buf_p++;
					*pixbuf++ = red;
					*pixbuf++ = green;
					*pixbuf++ = blue;
					*pixbuf++ = alphabyte;
					break;
				default:
					//Error("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
					break;
				}
			}
		}
	}
	else if (targa_header.image_type==10) {   // Runlength encoded RGB images
		unsigned char red,green,blue,alphabyte,packetHeader,packetSize,j;

		red = 0;
		green = 0;
		blue = 0;
		alphabyte = 0xff;

		for(row=rows-1; row>=0; row--) {
			pixbuf = targa_rgba + row*columns*bytesPerPixel;
			for(column=0; column<columns; ) {
				packetHeader= *buf_p++;
				packetSize = 1 + (packetHeader & 0x7f);
				if (packetHeader & 0x80) {        // run-length packet
					switch (targa_header.pixel_size) {
						case 24:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = 255;
								for(j=0;j<packetSize;j++) {
									*pixbuf++=red;
									*pixbuf++=green;
									*pixbuf++=blue;
									//*pixbuf++=alphabyte;
									column++;
									if (column==columns) { // run spans across rows
										column=0;
										if (row>0)
											row--;
										else
											goto breakOut;
										pixbuf = targa_rgba + row*columns*bytesPerPixel;
									}
								}

								break;
						case 32:
								blue = *buf_p++;
								green = *buf_p++;
								red = *buf_p++;
								alphabyte = *buf_p++;
								for(j=0;j<packetSize;j++) {
									*pixbuf++=red;
									*pixbuf++=green;
									*pixbuf++=blue;
									*pixbuf++=alphabyte;
									column++;
									if (column==columns) { // run spans across rows
										column=0;
										if (row>0)
											row--;
										else
											goto breakOut;
										pixbuf = targa_rgba + row*columns*bytesPerPixel;
									}
								}

								break;
						default:
							//Error("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
							break;
					}
	
				}
				else {                            // non run-length packet
					for(j=0;j<packetSize;j++) {
						switch (targa_header.pixel_size) {
							case 24:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									//*pixbuf++ = 255;
									break;
							case 32:
									blue = *buf_p++;
									green = *buf_p++;
									red = *buf_p++;
									alphabyte = *buf_p++;
									*pixbuf++ = red;
									*pixbuf++ = green;
									*pixbuf++ = blue;
									*pixbuf++ = alphabyte;
									break;
							default:
								//Sys_Printf("LoadTGA: illegal pixel_size '%d' in file '%s'\n", targa_header.pixel_size, name );
								break;
						}
						column++;
						if (column==columns) { // pixel packet run spans across rows
							column=0;
							if (row>0)
								row--;
							else
								goto breakOut;
							pixbuf = targa_rgba + row*columns*bytesPerPixel;
						}						
					}
				}
			}
			breakOut:;
		}
	}

	//free(buffer);
}




/* convert tga fille to  png file */
int Bmp::TGAToPNG(const char *tga_file_name,const char *file_name)

{
   
	// load tga file in memory
	Fload tga(tga_file_name);

	// file not found 
	if (tga.GetLen() <=16)
		return -1;

	unsigned char *inputdata = NULL; 
   

   FILE *fp=NULL;
   png_structp png_ptr=NULL;
   png_infop info_ptr=NULL;
   png_colorp palette=NULL;

   int width = 0;
   int height = 0;
   int bit_depth = 8;
   int bits_per_pixel = 32;


   int png_interlace = PNG_INTERLACE_NONE; //  : PNG_INTERLACE_ADAM7

   // load tga data in buffer 
   LoadTGABuffer((byte *) tga.GetData(), &inputdata, &width, &height,&bits_per_pixel);


   int color_type = 0;
   int bytes_per_pixel = bits_per_pixel / 8;

   if (bits_per_pixel==1*8) {
	   color_type = PNG_COLOR_TYPE_GRAY;
   }	
   else if (bits_per_pixel==2*8) {
	   color_type = PNG_COLOR_TYPE_GRAY_ALPHA;
   }	
   else if (bits_per_pixel==3*8) {
	   color_type = PNG_COLOR_TYPE_RGB;
   }	
   else if (bits_per_pixel==4*8) {
	  color_type = PNG_COLOR_TYPE_RGB_ALPHA;
   }	


   /* open the file */
   fp = fopen(file_name, "wb");
   if (fp == NULL)
      return (ERROR);

   /* Create and initialize the png_struct with the desired error handler
    * functions.  If you want to use the default stderr and longjump method,
    * you can supply NULL for the last three parameters.  We also check that
    * the library version is compatible with the one used at compile time,
    * in case we are using dynamically linked libraries.  REQUIRED.
    */
   //png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
   //   png_voidp user_error_ptr, user_error_fn, user_warning_fn);

   png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

   if (png_ptr == NULL)
   {
      fclose(fp);
      return (ERROR);
   }

   /* Allocate/initialize the image information data.  REQUIRED */
   info_ptr = png_create_info_struct(png_ptr);
   if (info_ptr == NULL)
   {
      fclose(fp);
      png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
      return (ERROR);
   }

   /* Set error handling.  REQUIRED if you aren't supplying your own
    * error handling functions in the png_create_write_struct() call.
    */
   if (setjmp(png_jmpbuf(png_ptr)))
   {
      /* If we get here, we had a problem reading the file */
      fclose(fp);
      png_destroy_write_struct(&png_ptr, &info_ptr);
      return (ERROR);
   }

   /* set up the output control if you are using standard C streams */
   png_init_io(png_ptr, fp);

#ifdef hilevel
   /* This is the easy way.  Use it if you already have all the
    * image info living info in the structure.  You could "|" many
    * PNG_TRANSFORM flags into the png_transforms integer here.
    */
   png_write_png(png_ptr, info_ptr, png_transforms, NULL);
#else
   /* This is the hard way */

   /* Set the image information here.  Width and height are up to 2^31,
    * bit_depth is one of 1, 2, 4, 8, or 16, but valid values also depend on
    * the color_type selected. color_type is one of PNG_COLOR_TYPE_GRAY,
    * PNG_COLOR_TYPE_GRAY_ALPHA, PNG_COLOR_TYPE_PALETTE, PNG_COLOR_TYPE_RGB,
    * or PNG_COLOR_TYPE_RGB_ALPHA.  interlace is either PNG_INTERLACE_NONE or
    * PNG_INTERLACE_ADAM7, and the compression_type and filter_type MUST
    * currently be PNG_COMPRESSION_TYPE_BASE and PNG_FILTER_TYPE_BASE. REQUIRED
    */
   png_set_IHDR(png_ptr, info_ptr, width, height, bit_depth, color_type,
      png_interlace, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);

#if 0
   if (palette) {
   /* set the palette if there is one.  REQUIRED for indexed-color images */
   palette = (png_colorp)png_malloc(png_ptr, 256 * sizeof (png_color));
   /* ... set palette colors ... */
   png_set_PLTE(png_ptr, info_ptr, palette, 256);
   /* You must not free palette here, because png_set_PLTE only makes a link to
      the palette that you malloced.  Wait until you are about to destroy
      the png structure. */
   }

   /* optional significant bit chunk */
   /* if we are dealing with a grayscale image then */
   sig_bit.gray = true_bit_depth;
   /* otherwise, if we are dealing with a color image then */
   sig_bit.red = true_red_bit_depth;
   sig_bit.green = true_green_bit_depth;
   sig_bit.blue = true_blue_bit_depth;
   /* if the image has an alpha channel then */
   sig_bit.alpha = true_alpha_bit_depth;
   png_set_sBIT(png_ptr, info_ptr, sig_bit);


   /* Optional gamma chunk is strongly suggested if you have any guess
    * as to the correct gamma of the image.
    */
   png_set_gAMA(png_ptr, info_ptr, gamma);

   /* Optionally write comments into the image */
   text_ptr[0].key = "Title";
   text_ptr[0].text = "Mona Lisa";
   text_ptr[0].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[1].key = "Author";
   text_ptr[1].text = "Leonardo DaVinci";
   text_ptr[1].compression = PNG_TEXT_COMPRESSION_NONE;
   text_ptr[2].key = "Description";
   text_ptr[2].text = "<long text>";
   text_ptr[2].compression = PNG_TEXT_COMPRESSION_zTXt;
#ifdef PNG_iTXt_SUPPORTED
   text_ptr[0].lang = NULL;
   text_ptr[1].lang = NULL;
   text_ptr[2].lang = NULL;
#endif
   png_set_text(png_ptr, info_ptr, text_ptr, 3);

#endif


   /* other optional chunks like cHRM, bKGD, tRNS, tIME, oFFs, pHYs, */
   /* note that if sRGB is present the gAMA and cHRM chunks must be ignored
    * on read and must be written in accordance with the sRGB profile */

   /* Write the file header information.  REQUIRED */
   png_write_info(png_ptr, info_ptr);

   /* If you want, you can write the info in two steps, in case you need to
    * write your private chunk ahead of PLTE:
    *
    *   png_write_info_before_PLTE(write_ptr, write_info_ptr);
    *   write_my_chunk();
    *   png_write_info(png_ptr, info_ptr);
    *
    * However, given the level of known- and unknown-chunk support in 1.1.0
    * and up, this should no longer be necessary.
    */

   /* Once we write out the header, the compression type on the text
    * chunks gets changed to PNG_TEXT_COMPRESSION_NONE_WR or
    * PNG_TEXT_COMPRESSION_zTXt_WR, so it doesn't get written out again
    * at the end.
    */

   /* set up the transformations you want.  Note that these are
    * all optional.  Only call them if you want them.
    */
#if 0
   /* invert monochrome pixels */
   png_set_invert_mono(png_ptr);

   /* Shift the pixels up to a legal bit depth and fill in
    * as appropriate to correctly scale the image.
    */
   png_set_shift(png_ptr, &sig_bit);

   /* pack pixels into bytes */
   png_set_packing(png_ptr);

   /* swap location of alpha bytes from ARGB to RGBA */
   png_set_swap_alpha(png_ptr);

   /* Get rid of filler (OR ALPHA) bytes, pack XRGB/RGBX/ARGB/RGBA into
    * RGB (4 channels -> 3 channels). The second parameter is not used.
    */
   png_set_filler(png_ptr, 0, PNG_FILLER_BEFORE);

   /* flip BGR pixels to RGB */
   png_set_bgr(png_ptr);

   /* swap bytes of 16-bit files to most significant byte first */
   png_set_swap(png_ptr);

   /* swap bits of 1, 2, 4 bit packed pixel formats */
   png_set_packswap(png_ptr);

   /* turn on interlace handling if you are not using png_write_image() */
   if (interlacing)
      number_passes = png_set_interlace_handling(png_ptr);
   else
      number_passes = 1;

#endif 


   /* The easiest way to write the image (you may have a different memory
    * layout, however, so choose what fits your needs best).  You need to
    * use the first method if you aren't handling interlacing yourself.
    */
   png_uint_32 k;
   // png_byte image[height][width*bytes_per_pixel];
   //png_bytep row_pointers[height];
   png_bytep *row_pointers = new png_bytep[height];

   for (k = 0; k < height; k++)
     row_pointers[k] = ((png_bytep)inputdata) + k*width*bytes_per_pixel;

   /* One of the following output methods is REQUIRED */
#if 1 // def entire /* write out the entire image data in one call */
   png_write_image(png_ptr, row_pointers);

   delete row_pointers;

   /* the other way to write the image - deal with interlacing */

#else no_entire /* write out the image data by one or more scanlines */
   /* The number of passes is either 1 for non-interlaced images,
    * or 7 for interlaced images.
    */
   for (pass = 0; pass < number_passes; pass++)
   {
      /* Write a few rows at a time. */
      png_write_rows(png_ptr, &row_pointers[first_row], number_of_rows);

      /* If you are only writing one row at a time, this works */
      for (y = 0; y < height; y++)
      {
         png_write_rows(png_ptr, &row_pointers[y], 1);
      }
   }
#endif no_entire /* use only one output method */

   /* You can write optional chunks like tEXt, zTXt, and tIME at the end
    * as well.  Shouldn't be necessary in 1.1.0 and up as all the public
    * chunks are supported and you can use png_set_unknown_chunks() to
    * register unknown chunks into the info structure to be written out.
    */

   /* It is REQUIRED to call this to finish writing the rest of the file */
   png_write_end(png_ptr, info_ptr);

#endif hilevel

   /* If you png_malloced a palette, free it here (don't free info_ptr->palette,
      as recommended in versions 1.0.5m and earlier of this example; if
      libpng mallocs info_ptr->palette, libpng will free it).  If you
      allocated it with malloc() instead of png_malloc(), use free() instead
      of png_free(). */
   if (palette) 
	   png_free(png_ptr, palette);
   palette=NULL;

   /* Similarly, if you png_malloced any data that you passed in with
      png_set_something(), such as a hist or trans array, free it here,
      when you can be sure that libpng is through with it. */
   //png_free(png_ptr, trans);
   //trans=NULL;

   /* clean up after the write, and free any memory allocated */
   png_destroy_write_struct(&png_ptr, &info_ptr);

   /* close the file */
   fclose(fp);
   
   free(inputdata);

   /* that's it */
   return (1);
}
