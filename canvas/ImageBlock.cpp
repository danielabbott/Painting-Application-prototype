#include <ImageBlock.h>
#include <Layer.h>
#include <cassert>
#include <UI.h>
#include <Canvas.h>
#include <Shader.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;


unsigned int image_block_size() { return 1024; }

void ImageBlock::create()
{
	assert(!layersRGBA.size());
	assert(!layersRG.size());
	assert(!layersR.size());

	

	unsigned int numRGBA = 0;
	unsigned int numRG = 0;
	unsigned int numR = 0;

	Layer * layer = get_first_layer();

	if(!layer) {
		// No layers, use default state
		return;
	}

	while(1) {
		if(layer->type == Layer::Type::LAYER) {
			if(layer->imageFormat == ImageFormat::FMT_RGBA) {
				layer->imageFormatSpecificIndex = numRGBA;
				numRGBA++;
			}
			else if(layer->imageFormat == ImageFormat::FMT_RG) {
				layer->imageFormatSpecificIndex = numRG;
				numRG++;
			}
			else if(layer->imageFormat == ImageFormat::FMT_R) {
				layer->imageFormatSpecificIndex = numR;
				numR++;
			}
		}


		if(layer->firstChild) {
			assert(layer->type == Layer::Type::LAYER);
			layer = layer->firstChild;
		}
		else {
			if(layer->next) {
				layer = layer->next;
			}
			else if(layer->parent && layer->parent->next) {
				layer = layer->parent->next;
			}
			else {
				break;
			}
		}
	}

	layersRGBA = vector<LayerData>(numRGBA);
	layersRG = vector<LayerData>(numRG);
	layersR = vector<LayerData>(numR);

	if(numRGBA) {
		arrayTextureRGBA.create(ImageFormat::FMT_RGBA, image_block_size(), numRGBA);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRGBA.clear(0x00ffffff);
		}

		for(unsigned int i = 0; i < numRGBA; i++) {
			layersRGBA[i].colour = 0x00ffffff;
			layersRGBA[i].frameBuffer.create(arrayTextureRGBA, i);

			if(!GLAD_GL_ARB_clear_texture) {
				layersRGBA[i].frameBuffer.bindFrameBuffer();
				glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}

	}
	if(numRG) {
		arrayTextureRG.create(ImageFormat::FMT_RG, image_block_size(), numRG);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRG.clear(0x00ff);
		}

		for(unsigned int i = 0; i < numRG; i++) {
			layersRG[i].colour = 0x00ffffff;
			layersRG[i].frameBuffer.create(arrayTextureRG, i);

			if(!GLAD_GL_ARB_clear_texture) {
				layersRG[i].frameBuffer.bindFrameBuffer();
				glClearColor(1.0f, 0.0f, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}
	}
	if(numR) {
		arrayTextureR.create(ImageFormat::FMT_R, image_block_size(), numR);

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureR.clear();
		}

		for(unsigned int i = 0; i < numR; i++) {
			layersR[i].colour = 0x00;
			layersR[i].frameBuffer.create(arrayTextureR, i);

			if(!GLAD_GL_ARB_clear_texture) {
				layersR[i].frameBuffer.bindFrameBuffer();
				glClearColor(0, 0, 0, 0);
				glClear(GL_COLOR_BUFFER_BIT);
			}
		}
	}

}

void ImageBlock::destroy()
{
	for(LayerData & l : layersRGBA) {
		if(l.frameBuffer.isCreated()){
			l.frameBuffer.destroy();
		}
	}
	for(LayerData & l : layersRG) {
		if(l.frameBuffer.isCreated()){
			l.frameBuffer.destroy();
		}
	}
	for(LayerData & l : layersR) {
		if(l.frameBuffer.isCreated()){
			l.frameBuffer.destroy();
		}
	}


	if(arrayTextureRGBA.isCreated()) {
		arrayTextureRGBA.destroy();
	}

	if(arrayTextureRG.isCreated()) {
		arrayTextureRG.destroy();
	}

	if(arrayTextureR.isCreated()) {
		arrayTextureR.destroy();
	}
}

int ImageBlock::indexOf(Layer * layer)
{
	assert(layer);
	return layer->imageFormatSpecificIndex;
}

void ImageBlock::bindFrameBuffer(Layer * layer)
{
	assert(layer);

	if(layer->imageFormat == ImageFormat::FMT_RGBA) {
		layersRGBA[layer->imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
		layersRGBA[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
	}
	else if(layer->imageFormat == ImageFormat::FMT_RG) {
		layersRG[layer->imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
		layersRG[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
	}
	else if(layer->imageFormat == ImageFormat::FMT_R) {
		layersR[layer->imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
		layersR[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
	}
}

void ImageBlock::bindTexture(Layer * layer)
{
	assert(layer);

	if(layer->imageFormat == ImageFormat::FMT_RGBA) {
		arrayTextureRGBA.bind();
	}
	else if(layer->imageFormat == ImageFormat::FMT_RG) {
		arrayTextureRG.bind();
	}
	else if(layer->imageFormat == ImageFormat::FMT_R) {
		arrayTextureR.bind();
	}
}


void ImageBlock::copyTo(Layer * layer)
{
	assert(layer);

	if(layer->imageFormat == ImageFormat::FMT_RGBA) {
		layersRGBA[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
		arrayTextureRGBA.copy(layer->imageFormatSpecificIndex);
	}
	else if(layer->imageFormat == ImageFormat::FMT_RG) {
		layersRG[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
		arrayTextureRG.copy(layer->imageFormatSpecificIndex);
	}
	else if(layer->imageFormat == ImageFormat::FMT_R) {
		layersR[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
		arrayTextureR.copy(layer->imageFormatSpecificIndex);
	}
	else {
		assert(0);
	}
}

void ImageBlock::uploadImage(Layer * layer, unsigned int x, unsigned int y, unsigned int width, unsigned int height, void * data, unsigned int stride, ImageFormat sourceType)
{
	assert(layer);

	if(layer->imageFormat == ImageFormat::FMT_RGBA) {
		layersRGBA[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
		arrayTextureRGBA.uploadImage(layer->imageFormatSpecificIndex, x, y, width, height, data, stride, sourceType);
	}
	else if(layer->imageFormat == ImageFormat::FMT_RG) {
		layersRG[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
		arrayTextureRG.uploadImage(layer->imageFormatSpecificIndex, x, y, width, height, data, stride, sourceType);
	}
	else if(layer->imageFormat == ImageFormat::FMT_R) {
		layersR[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::ACTUAL_DATA;
		arrayTextureR.uploadImage(layer->imageFormatSpecificIndex, x, y, width, height, data, stride, sourceType);
	}
}

void ImageBlock::fillLayer(Layer * layer, uint32_t colour)
{
	assert(layer);

	if(layer->imageFormat == ImageFormat::FMT_RGBA) {
		layersRGBA[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::SOLID_COLOUR;
		layersRGBA[layer->imageFormatSpecificIndex].colour = colour;

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRGBA.clear(layer->imageFormatSpecificIndex, 1, colour);
		}
		else {
			layersRGBA[layer->imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, ((colour >> 8) & 0xff) / 255.0f, ((colour >> 16) & 0xff) / 255.0f, ((colour >> 24) & 0xff) / 255.0f);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	else if(layer->imageFormat == ImageFormat::FMT_RG) {
		layersRG[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::SOLID_COLOUR;
		layersRG[layer->imageFormatSpecificIndex].colour = colour;

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureRG.clear(layer->imageFormatSpecificIndex, 1, colour);
		}
		else {
			layersRG[layer->imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, ((colour >> 8) & 0xff) / 255.0f, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
	else if(layer->imageFormat == ImageFormat::FMT_R) {
		layersR[layer->imageFormatSpecificIndex].dataType = LayerData::DataType::SOLID_COLOUR;
		layersR[layer->imageFormatSpecificIndex].colour = colour;

		if(GLAD_GL_ARB_clear_texture) {
			arrayTextureR.clear(layer->imageFormatSpecificIndex, 1, colour);
		}
		else {
			layersR[layer->imageFormatSpecificIndex].frameBuffer.bindFrameBuffer();
			glClearColor((colour & 0xff) / 255.0f, 0, 0, 0);
			glClear(GL_COLOR_BUFFER_BIT);
		}
	}
}

bool ImageBlock::dirtyRegion(int x_, int y_, unsigned int width, unsigned int height)
{
	x_ -= (int)x;
	y_ -= (int)y;

	if(x_ < (int)image_block_size() && y_ < (int)image_block_size() &&
		x_ + (int)width >= 0 && y_ + (int)height >= 0) {


		if(x_ < 0) {
			width += x_;
			x_ = 0;
		}

		if(y_ < 0) {
			height += y_;
			y_ = 0;
		}

		if((unsigned)x_ + width > image_block_size()) {
			width = image_block_size() - (unsigned)x_;
		}

		if((unsigned)y_ + height > image_block_size()) {
			height = image_block_size() - (unsigned)y_;
		}

		if(dirty) {
			if(x_ < (int)dirtyMinX) {
				dirtyWidth += dirtyMinX-x_;
				dirtyMinX = x_;
			}
			if(y_ < (int)dirtyMinY) {
				dirtyHeight += dirtyMinY-y_;
				dirtyMinY = y_;
			}
			if(x_+width > dirtyMinX+dirtyWidth){
				dirtyWidth = x_+width - dirtyMinX;
			}
			if(y_+height > dirtyMinY+dirtyHeight){
				dirtyHeight = y_+height - dirtyMinY;
			}

		}
		else {
			dirtyMinX = x_;
			dirtyMinY = y_;
			dirtyWidth = width;
			dirtyHeight = height;
			dirty = true;
		}

		return true;
	}
	return false;
}
