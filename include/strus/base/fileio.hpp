/*
 * Copyright (c) 2014 Patrick P. Frey
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/// \brief Basic portable file read/write and related filesystem functions
#ifndef _STRUS_BASE_FILE_IO_HPP_INCLUDED
#define _STRUS_BASE_FILE_IO_HPP_INCLUDED
#include <vector>
#include <string>
#include <cstdio>

namespace strus
{

/// \brief Evaluate the size of a file in bytes
/// \param[in] filename the name of file to get the filesize of
/// \param[out] size the returned size
/// \return 0 on success, errno on failure
int readFileSize( const std::string& filename, std::size_t& size);

/// \brief Reads the complete content of a file to a string
/// \param[in] filename the name of file to read
/// \param[out] res the returned content of the file read
/// \return 0 on success, errno on failure
int readFile( const std::string& filename, std::string& res);

/// \brief Reads the complete input of stdin to a string
/// \param[out] res the returned content of the file read
/// \return 0 on success, errno on failure
int readStdin( std::string& res);

/// \brief Writes a string to a file creating the file if it does not exist
/// \param[in] filename the name of the file to write
/// \param[in] content content of the string to write
/// \return 0 on success, errno on failure
int writeFile( const std::string& filename, const std::string& content);

/// \brief Writes a blob to a file creating the file if it does not exist
/// \param[in] filename the name of the file to write
/// \param[in] content content to write
/// \param[in] contentsize size of content to write in bytes
/// \return 0 on success, errno on failure
int writeFile( const std::string& filename, const void* content, std::size_t contentsize);

/// \brief Appends a string to a file creating the file if it does not exist
/// \param[in] filename the name of the file to append to
/// \param[out] content content of the string to write
/// \return 0 on success, errno on failure
int appendFile( const std::string& filename, const std::string& content);

/// \brief Removes a file
/// \param[in] filename the name of the file to remove
/// \param[in] fail_ifnotexist return an error if the directory does not exist
/// \return 0 on success, errno on failure
int removeFile( const std::string& filename, bool fail_ifnotexist=false);

/// \brief Removes an empty directory
/// \param[in] dirname the name of the directory to remove
/// \param[in] fail_ifnotexist return an error if the directory does not exist
/// \return 0 on success, errno on failure
int removeDir( const std::string& dirname, bool fail_ifnotexist=false);

/// \brief Removes a directory with all its contents
/// \param[in] dirname the name of the directory to remove
/// \param[in] fail_ifnotexist return an error if the directory does not exist
/// \return 0 on success, errno on failure
int removeDirRecursive( const std::string& dirname, bool fail_ifnotexist=false);

/// \brief Moves (renames) a file
/// \param[in] old_filename the name of the file to rename
/// \param[in] new_filename the new name of the file
/// \return 0 on success, errno on failure
int renameFile( const std::string& old_filename, const std::string& new_filename);

/// \brief Creates a directory if it does not exist yet
/// \param[in] dirname path of directory to create
/// \param[in] fail_ifexist return an error if the directory already exists
/// \return 0 on success, errno on failure
int createDir( const std::string& dirname, bool fail_ifexist=true);

/// \brief Ensures the existance of a directory path by creating it if it does not exist (mkdir -p)
/// \param[in] dirname path of directory to create
/// \return 0 on success, errno on failure
int mkdirp( const std::string& dirname);

/// \brief Same as mkdirp, but returns also the directory created
/// \param[in] dirname path of directory to create
/// \param[out] firstDirectoryCreated path of the first directory of the directory path created
/// \return 0 on success, errno on failure
int mkdirp( const std::string& dirname, std::string& firstDirectoryCreated);

/// \brief Change current directory of the process
/// \param[in] dirname path of directory to create
/// \return 0 on success, errno on failure
int changeDir( const std::string& dirname);

/// \brief Reads all file names matching to the extension ext of a directory to an array of strings
/// \param[in] path the name of director to read
/// \param[in] ext the file extension the accepted files must have (empty if all extensions are accepted)
/// \param[out] res the returned matching file names of the directory read
/// \return 0 on success, errno on failure
int readDirFiles( const std::string& path, const std::string& ext, std::vector<std::string>& res);

/// \brief Reads all subdirectories of a directory to an array of strings
/// \param[in] path the name of director to read
/// \param[out] res the returned matching sub directories of the directory read
/// \return 0 on success, errno on failure
int readDirSubDirs( const std::string& path, std::vector<std::string>& res);

/// \brief Reads all items (directory or file) of a directory to an array of strings
/// \param[in] path the name of director to read
/// \param[out] res the returned matching items of the directory read
/// \return 0 on success, errno on failure
int readDirItems( const std::string& path, std::vector<std::string>& res);

/// \brief Get all items matching a file path pattern (with '?' and '*' as substitutes)
/// \param[in] pathPattern file path pattern
/// \param[out] res the returned matching paths
/// \return 0 on success, errno on failure
int expandFilePattern( const std::string& pathPattern, std::vector<std::string>& res);

/// \brief Check if a file system path points to a file
/// \return true, if yes
/// \param[in] path file system path to check
bool isFile( const std::string& path);

/// \brief Check if a file system path points to a directory
/// \param[in] path file system path to check
/// \return true, if yes
bool isDir( const std::string& path);

/// \brief Check if a file system path is a path starting with a name and not with a directive referencing the current path './' or the file system root '/'.
/// \return true, if yes
/// \param[in] path file system path to check
bool isRelativePath( const std::string& path);

/// \brief Check if a file system path is a path starting the file system root '/'.
/// \return true, if yes
/// \param[in] path file system path to check
bool isAbsolutePath( const std::string& path);

/// \brief Check if a file path is explicit (relative path or absolute path)
/// \return true, if yes
/// \param[in] path file system path to check
bool isExplicitPath( const std::string& path);

/// \brief Check if a file system path points to a file with text content (not binary)
/// \param[in] path file system path to check
/// \remark Implementation uses heuristics of character frequencies for the check. Currently fails on UTF-8 text files of non latin alphabet based languages.
/// \return true, if yes
bool isTextFile( const std::string& path);

/// \brief Get the parent (containing) path of a file or directory
/// \param[in] path file system path to get the containing path
/// \param[out] dest the containing path
/// \return 0 on success, errno on failure
int getParentPath( const std::string& path, std::string& dest);

/// \brief Get an ancestor (containing) path of a file or directory
/// \param[in] path file system path to get the containing path
/// \param[in] level how many levels to step back (level == 1 => getParentPath)
/// \param[out] dest the containing path
/// \return 0 on success, errno on failure
int getAncestorPath( const std::string& path, int level, std::string& dest);

/// \brief Get the file name without parent path of a file or directory
/// \param[in] path file system path to get the file name from
/// \param[out] dest the file name
/// \return 0 on success, errno on failure
int getFileName( const std::string& path, std::string& dest, bool withExtension=true);

/// \brief Get the extension (including '.') of a file or directory
/// \param[in] path file system path to get the extension from
/// \param[out] ext the extension
/// \return 0 on success, errno on failure
int getFileExtension( const std::string& path, std::string& ext);

/// \brief Replace the file extension (including '.') of a filename by a diven new one
/// \param[in] path file system path to replace the extension
/// \param[in] ext the new extension to substiture with
/// \return the new path or an empty string on a memory allocation error
std::string replaceFileExtension( const std::string& path, const std::string& ext);

/// \brief Get the OS path element separator
/// \return the separator
char dirSeparator();

/// \brief Return true, if the path contains an upper directory reference '..'
/// \param[in] path file system path
/// \return true if yes
bool hasUpdirReference( const std::string& path);

/// \brief Join two path with directory separator
/// \param[in] parentpath left part of the resulting file system path
/// \param[in] childpath right part of the resulting file system path
/// \return the joined path or empty in case of a memory allocation error
std::string joinFilePath( const std::string& parentpath, const std::string& childpath);

/// \brief Resolve upper directory references in a path, e.g. replace "/home/john/../jack" by "/home/jack"
/// \param[in,out] path path to resolve updir references in
/// \note Does not care about permissions, it is just mapping a string to another
/// \note Does not care about symbolic links and may do an incorrect transformation when symbolic links are in the path
/// \remark Result gets incorrect when symbolic links are used
/// \return 0 on success, errno on failure (ENOMEM,EINVAL)
int resolveUpdirReferences( std::string& path);

}//namespace
#endif

