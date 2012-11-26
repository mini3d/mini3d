
// Copyright (c) <2011> Daniel Peterson
// This file is part of Mini3D <www.mini3d.org>
// It is distributed under the MIT Software License <www.mini3d.org/license.php>

namespace mini3d
{

template <typename T1, typename T2> T1 mini3d_swuzzle_bits(T1 &data, unsigned int u, unsigned int v) { return ((T1)((T2)(data >> u*sizeof(T2)*8))) << v*sizeof(T2)*8; };

unsigned char* mini3d_GenerateMipMapBoxFilter(unsigned char* pBuffer, unsigned int width, unsigned int height)
{
	if (width <= 1 && height <= 1) return 0;

	unsigned int mipMapWidth = (width >> 1) > 0 ? (width >> 1) : 1;
	unsigned int mipMapHeight = (height >> 1) > 0 ? (height >> 1) : 1;

	unsigned char* pMipMap = new unsigned char[mipMapWidth * mipMapHeight * 4];

	unsigned int rowSizeInBytes = width * 4;
	unsigned int mipMapRowSizeInBytes = mipMapWidth * 4;

	for(unsigned int y=0; y < height; y += 2)
	{
		for(unsigned int x=0; x < rowSizeInBytes; x += 8)
		{
			if (width == 1)
			{
				pMipMap[0 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[0 + x + y * rowSizeInBytes] + pBuffer[0 + x + (y + 1) * rowSizeInBytes]) >> 1;
				pMipMap[1 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[1 + x + y * rowSizeInBytes] + pBuffer[1 + x + (y + 1) * rowSizeInBytes]) >> 1;
				pMipMap[2 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[2 + x + y * rowSizeInBytes] + pBuffer[2 + x + (y + 1) * rowSizeInBytes]) >> 1;
				pMipMap[3 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[3 + x + y * rowSizeInBytes] + pBuffer[3 + x + (y + 1) * rowSizeInBytes]) >> 1;
			}
			else if (height == 1)
			{
				pMipMap[0 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[0 + x + y * rowSizeInBytes] + pBuffer[4 + x + y * rowSizeInBytes]) >> 1;
				pMipMap[1 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[1 + x + y * rowSizeInBytes] + pBuffer[5 + x + y * rowSizeInBytes]) >> 1;
				pMipMap[2 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[2 + x + y * rowSizeInBytes] + pBuffer[6 + x + y * rowSizeInBytes]) >> 1;
				pMipMap[3 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[3 + x + y * rowSizeInBytes] + pBuffer[7 + x + y * rowSizeInBytes]) >> 1;
			}
			else
			{
				pMipMap[0 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[0 + x + y * rowSizeInBytes] + pBuffer[4 + x + y * rowSizeInBytes] + pBuffer[0 + x + (y + 1) * rowSizeInBytes] + pBuffer[4 + x + (y + 1) * rowSizeInBytes]) >> 2;
				pMipMap[1 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[1 + x + y * rowSizeInBytes] + pBuffer[5 + x + y * rowSizeInBytes] + pBuffer[1 + x + (y + 1) * rowSizeInBytes] + pBuffer[5 + x + (y + 1) * rowSizeInBytes]) >> 2;
				pMipMap[2 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[2 + x + y * rowSizeInBytes] + pBuffer[6 + x + y * rowSizeInBytes] + pBuffer[2 + x + (y + 1) * rowSizeInBytes] + pBuffer[6 + x + (y + 1) * rowSizeInBytes]) >> 2;
				pMipMap[3 + (x >> 1) + (y >> 1) * mipMapRowSizeInBytes] = (pBuffer[3 + x + y * rowSizeInBytes] + pBuffer[7 + x + y * rowSizeInBytes] + pBuffer[3 + x + (y + 1) * rowSizeInBytes] + pBuffer[7 + x + (y + 1) * rowSizeInBytes]) >> 2;
			}
		}
	}	
	
	return pMipMap;
}
}

