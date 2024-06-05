#ifndef __GLTextureWriter__
#define __GLTextureWriter__

#include <string>
#include <memory>
#include "Texture.h"
/**
 * GLTextureWriter outputs a three channel (GL_RGB)
 * image to a png file given by file name.
 *
 * Contact kpidding@calpoly.edu for any support questions!
 */
namespace GLTextureWriter
{
	bool WriteImage(std::shared_ptr<Texture> texture, std::string fileName);
	bool WriteImage(const Texture & texture, std::string fileName);
	bool WriteImage(GLint textureHandle, std::string fileName);
}

/**
 * Retrieve data from a bound buffer
 */
void getData(void * dataBuffer, GLenum format, GLenum type);
/**
 * Main logic of this code taken from stb_image.h
 * @param imgData Image data to flip
 * @param width   width of the image
 * @param height  height of the image
 */
void flip_buffer(char * imgData, int width, int height );
#endif