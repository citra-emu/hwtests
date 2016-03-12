#include <memory>
#include <cstring>
#include <3ds.h>

#include "common/scope_exit.h"
#include "tests/test.h"
#include "tests/fs/fs_sdmc.h"

namespace FS {
namespace SDMC {

static bool TestFileCreateDelete(FS_Archive sdmcArchive)
{
    Handle fileHandle, fileHandle2;
    const static FS_Path filePath = fsMakePath(PATH_ASCII, "/test_file_create_delete.txt");
    const static FS_Path filePath2 = fsMakePath(PATH_ASCII, "/test_file_create_2.txt");

    // Create file with OpenFile (not interested in opening the handle)
    SoftAssert(FSUSER_OpenFile(&fileHandle, sdmcArchive, filePath, FS_OPEN_CREATE | FS_OPEN_WRITE, 0) == 0);
    FSFILE_Close(fileHandle);

    // Make sure the new file exists
    SoftAssert(FSUSER_OpenFile(&fileHandle, sdmcArchive, filePath, FS_OPEN_READ, 0) == 0);
    FSFILE_Close(fileHandle);

    SoftAssert(FSUSER_DeleteFile(sdmcArchive, filePath) == 0);

    // Should fail to make sure the file no longer exists
    SoftAssert(FSUSER_OpenFile(&fileHandle, sdmcArchive, filePath, FS_OPEN_READ, 0) != 0);
    FSFILE_Close(fileHandle);

    // Create file with CreateFile
    SoftAssert(FSUSER_CreateFile(sdmcArchive, filePath2, 0, 0) == 0);
    SCOPE_EXIT({
        FSFILE_Close(fileHandle2);
        FSUSER_DeleteFile(sdmcArchive, filePath2);
    });

    // Make sure the new file exists
    SoftAssert(FSUSER_OpenFile(&fileHandle2, sdmcArchive, filePath2, FS_OPEN_READ, 0) == 0);

    // Try and create a file over an already-existing file (Should fail)
    SoftAssert(FSUSER_CreateFile(sdmcArchive, filePath2, 0, 0) != 0);

    return true;
}

static bool TestFileRename(FS_Archive sdmcArchive)
{
    Handle fileHandle;
    const static FS_Path filePath = fsMakePath(PATH_ASCII, "/test_file_rename.txt");
    const static FS_Path newFilePath = fsMakePath(PATH_ASCII, "/test_file_rename_new.txt");

    // Create file
    FSUSER_CreateFile(sdmcArchive, filePath, 0, 0);

    SoftAssert(FSUSER_RenameFile(sdmcArchive, filePath, sdmcArchive, newFilePath) == 0);

    // Should fail to make sure the old file no longer exists
    if (FSUSER_OpenFile(&fileHandle, sdmcArchive, filePath, FS_OPEN_READ, 0) == 0) {
        FSUSER_DeleteFile(sdmcArchive, filePath);
        return false;
    }
    FSFILE_Close(fileHandle);

    // Make sure the new file exists
    SoftAssert(FSUSER_OpenFile(&fileHandle, sdmcArchive, newFilePath, FS_OPEN_READ, 0) == 0);
    FSFILE_Close(fileHandle);

    SoftAssert(FSUSER_DeleteFile(sdmcArchive, newFilePath) == 0);

    return true;
}

static bool TestFileWriteRead(FS_Archive sdmcArchive)
{
    Handle fileHandle;
    u32 bytesWritten;
    u32 bytesRead;
    u64 fileSize;

    const static FS_Path filePath = fsMakePath(PATH_ASCII, "/test_file_write_read.txt");
    const static char* stringWritten = "A string\n";

    // Create file
    FSUSER_OpenFile(&fileHandle, sdmcArchive, filePath, FS_OPEN_CREATE | FS_OPEN_WRITE, 0);
    SCOPE_EXIT({ // Close and delete file no matter what happens
        FSFILE_Close(fileHandle);
        FSUSER_DeleteFile(sdmcArchive, filePath);
    });

    // Write to file
    SoftAssert(FSFILE_Write(fileHandle, &bytesWritten, 0, stringWritten, strlen(stringWritten)+1, FS_WRITE_FLUSH) == 0);
    // Verify string size
    SoftAssert(strlen(stringWritten)+1 == bytesWritten);

    // Check file size
    SoftAssert(FSFILE_GetSize(fileHandle, &fileSize) == 0);
    // Verify file size
    SoftAssert(fileSize == bytesWritten);

    std::unique_ptr<char> stringRead(new char[fileSize]);
    // Read from file
    SoftAssert(FSFILE_Read(fileHandle, &bytesRead, 0, stringRead.get(), fileSize) == 0);
    // Verify string contents
    SoftAssert(strcmp(stringRead.get(), stringWritten) == 0);

    return true;
}

static bool TestDirCreateDelete(FS_Archive sdmcArchive)
{
    Handle dirHandle;
    const static FS_Path dirPath = fsMakePath(PATH_ASCII, "/test_dir_create_delete");

    // Create directory
    SoftAssert(FSUSER_CreateDirectory(sdmcArchive, dirPath, 0) == 0);

    // Make sure the new dir exists
    SoftAssert(FSUSER_OpenDirectory(&dirHandle, sdmcArchive, dirPath) == 0);
    FSDIR_Close(dirHandle);

    SoftAssert(FSUSER_DeleteDirectory(sdmcArchive, dirPath) == 0);

    // Should fail to make sure the dir no longer exists
    SoftAssert(FSUSER_OpenDirectory(&dirHandle, sdmcArchive, dirPath) != 0);
    FSDIR_Close(dirHandle);

    return true;
}

static bool TestDirRename(FS_Archive sdmcArchive)
{
    Handle dirHandle;
    const static FS_Path dirPath = fsMakePath(PATH_ASCII, "/test_dir_rename");
    const static FS_Path newDirPath = fsMakePath(PATH_ASCII, "/test_dir_rename_new");

    // Create dir
    FSUSER_CreateDirectory(sdmcArchive, dirPath, 0);

    SoftAssert(FSUSER_RenameDirectory(sdmcArchive, dirPath, sdmcArchive, newDirPath) == 0);

    // Should fail to make sure the old dir no longer exists
    if (FSUSER_OpenDirectory(&dirHandle, sdmcArchive, dirPath) == 0) {
        FSUSER_DeleteDirectory(sdmcArchive, dirPath);
        return false;
    }
    FSDIR_Close(dirHandle);

    // Make sure the new dir exists
    SoftAssert(FSUSER_OpenDirectory(&dirHandle, sdmcArchive, newDirPath) == 0);
    FSDIR_Close(dirHandle);

    SoftAssert(FSUSER_DeleteDirectory(sdmcArchive, newDirPath) == 0);

    return true;
}

void TestAll()
{
    FS_Archive sdmcArchive = { 0x00000009, { PATH_EMPTY, 1, (u8*) "" } };

    Test("SDMC", "Opening archive", FSUSER_OpenArchive(&sdmcArchive), 0L);
    Test("SDMC", "Creating and deleting file", TestFileCreateDelete(sdmcArchive), true);
    Test("SDMC", "Renaming file", TestFileRename(sdmcArchive), true);
    Test("SDMC", "Writing and reading file", TestFileWriteRead(sdmcArchive), true);
    Test("SDMC", "Creating and deleting directory", TestDirCreateDelete(sdmcArchive), true);
    Test("SDMC", "Renaming directory", TestDirRename(sdmcArchive), true);
    Test("SDMC", "Closing archive", FSUSER_CloseArchive(&sdmcArchive), 0L);
}

} // namespace
} // namespace
