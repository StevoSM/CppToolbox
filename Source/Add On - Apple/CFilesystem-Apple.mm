//----------------------------------------------------------------------------------------------------------------------
//	CFilesystem-Apple.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFilesystem.h"

#include "CCoreFoundation.h"

#include <Foundation/Foundation.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFilesystemReportErrorFileFolderX1(error, message, fileFolder)								\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder.logAsError(CString::mSpaceX4);										\
				}
#define	CFilesystemReportErrorFileFolderX1AndReturnError(error, message, fileFolder)				\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder.logAsError(CString::mSpaceX4);										\
																									\
					return error;																	\
				}
#define	CFilesystemReportErrorFileFolderX2AndReturnError(error, message, fileFolder1, fileFolder2)	\
				{																					\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);			\
					fileFolder1.logAsError(CString::mSpaceX4);										\
					fileFolder2.logAsError(CString::mSpaceX4);										\
																									\
					return error;																	\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFilesystem

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::getFolders(const CFolder& folder, TArray<CFolder>& outFolders)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*				url =
								(NSURL*) CFBridgingRelease(
										CCoreFoundation::createURLRefFrom(folder.getFilesystemPath(), false));

	NSError*			error;
	NSFileManager*		fileManager = [NSFileManager defaultManager];
	NSArray<NSURL*>*	URLs =
								[fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:nil options:0
										error:&error];
	if (URLs != nil) {
		// Iterate URLs
		for (url in URLs) {
			// Determine if file or folder
			NSNumber*	number;
			if ([url getResourceValue:&number forKey:NSURLIsDirectoryKey error:nil] && number.boolValue)
				// Folder
				outFolders += CFolder(CFilesystemPath(CString((__bridge CFStringRef) url.path)));
		}

		return kNoError;
	} else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(MAKE_UErrorFromNSError(error), "getting folders", folder);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::getFiles(const CFolder& folder, TArray<CFile>& outFiles)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*				url =
								(NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(folder.getFilesystemPath(),
										false));

	NSError*			error;
	NSFileManager*		fileManager = [NSFileManager defaultManager];
	NSArray<NSURL*>*	URLs =
								[fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:nil options:0
										error:&error];
	if (URLs != nil) {
		// Iterate URLs
		for (url in URLs) {
			// Determine if file or folder
			NSNumber*	number;
			if ([url getResourceValue:&number forKey:NSURLIsDirectoryKey error:nil] && !number.boolValue)
				// File
				outFiles += CFile(CFilesystemPath(CString((__bridge CFStringRef) url.path)));
		}

		return kNoError;
	} else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(MAKE_UErrorFromNSError(error), "getting files", folder);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::getFoldersFiles(const CFolder& folder, TArray<CFolder>& outFolders, TArray<CFile>& outFiles)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get URL
	NSURL*				url =
								(NSURL*) CFBridgingRelease(CCoreFoundation::createURLRefFrom(folder.getFilesystemPath(),
										false));

	NSError*			error;
	NSFileManager*		fileManager = [NSFileManager defaultManager];
	NSArray<NSURL*>*	URLs =
								[fileManager contentsOfDirectoryAtURL:url includingPropertiesForKeys:nil options:0
										error:&error];
	if (URLs != nil) {
		// Iterate URLs
		for (url in URLs) {
			// Determine if file or folder
			NSNumber*	number;
			if ([url getResourceValue:&number forKey:NSURLIsDirectoryKey error:nil] && number.boolValue)
				// Folder
				outFolders += CFolder(CFilesystemPath(CString((__bridge CFStringRef) url.path)));
			else
				// File
				outFiles += CFile(CFilesystemPath(CString((__bridge CFStringRef) url.path)));
		}

		return kNoError;
	} else
		// Error
		CFilesystemReportErrorFileFolderX1AndReturnError(MAKE_UErrorFromNSError(error), "getting folders and files",
				folder);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFilesystem::replace(const CFile& sourceFile, const CFile& destinationFile)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	NSURL*	sourceURL =
					(NSURL*) CFBridgingRelease(
							CCoreFoundation::createURLRefFrom(sourceFile.getFilesystemPath(), false));
	NSURL*	destinationURL =
					(NSURL*) CFBridgingRelease(
							CCoreFoundation::createURLRefFrom(destinationFile.getFilesystemPath(), false));

	// Replace contents
	NSError*	error;
	if ([[NSFileManager defaultManager] replaceItemAtURL:destinationURL withItemAtURL:sourceURL backupItemName:nil
			options:0 resultingItemURL:nil error:&error])
		// Success
		return kNoError;
	else
		// Error
		CFilesystemReportErrorFileFolderX2AndReturnError(MAKE_UErrorFromNSError(error), "replacing file", sourceFile,
				destinationFile);
}