//----------------------------------------------------------------------------------------------------------------------
//	CData_ZIPExtensions.cpp			©2013 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CData_ZIPExtensions.h"

#include <zlib.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CData_ZIPExtensions

//----------------------------------------------------------------------------------------------------------------------
CData CData_ZIPExtensions::uncompressDataAsZIP(const CData& data, CDataSize uncompressedDataSize)
//----------------------------------------------------------------------------------------------------------------------
{
	UInt32	sourceSize = data.getSize();
	CData	decompressedData(
					(uncompressedDataSize != kCDataSizeUnknown) ? uncompressedDataSize : sourceSize + sourceSize / 2);

	z_stream	strm;
	strm.next_in = (Bytef*) data.getBytePtr();
	strm.avail_in = sourceSize;
	strm.total_out = 0;
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;

	if (inflateInit(&strm) != Z_OK)
		return CData::mEmpty;

	int	zLibStatus = Z_OK;
	while (zLibStatus == Z_OK) {
		strm.next_out = (Bytef*) decompressedData.getMutableBytePtr() + strm.total_out;
		strm.avail_out = (uInt) (decompressedData.getSize() - strm.total_out);

		// Inflate another chunk.
		zLibStatus = inflate(&strm, Z_SYNC_FLUSH);
		
		// We need more space?
		if (strm.avail_out == 0)
			decompressedData.increaseSizeBy(sourceSize / 2);
	}
	
	if (zLibStatus == Z_STREAM_END)
		zLibStatus = inflateEnd(&strm);
	else
		inflateEnd(&strm);
	
	if (zLibStatus != Z_OK)
		return CData::mEmpty;

	decompressedData.setSize((CDataSize) strm.total_out);
	
	return decompressedData;
}
