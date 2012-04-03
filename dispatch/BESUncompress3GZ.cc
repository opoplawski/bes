// BESUncompress3GZ.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004-2009 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.edu> and Jose Garcia <jgarcia@ucar.edu>
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2.1 of the License, or (at your option) any later version.
// 
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// You can contact University Corporation for Atmospheric Research at
// 3080 Center Green Drive, Boulder, CO 80301

// (c) COPYRIGHT University Corporation for Atmospheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>
//      jgarcia     Jose Garcia <jgarcia@ucar.edu>

#include <zlib.h>

#include <cstdio>
#include <cstring>
#include <cerrno>
#include <sstream>

using std::ostringstream;

#include "BESUncompress3GZ.h"
#include "BESInternalError.h"
#include "BESDebug.h"

#define CHUNK 4096

/** @brief uncompress a file with the .gz file extension
 *
 * @param src file that will be uncompressed
 * @param target file to uncompress the src file to
 */
void BESUncompress3GZ::uncompress(const string &src, int dest_fd)
{
    // buffer to hold the uncompressed data
    char in[CHUNK];

    // open the file to be read by gzopen. If the file is not compressed
    // using gzip then all this function will do is trasnfer the data to the
    // destination file.
    gzFile gsrc = gzopen(src.c_str(), "rb");
    if (gsrc == NULL) {
        string err = "Could not open the compressed file " + src;
        throw BESInternalError(err, __FILE__, __LINE__);
    }
#if 0
    FILE *dest = fopen(target.c_str(), "wb");
    if (!dest) {
        char *serr = strerror(errno);
        string err = "Unable to create the uncompressed file " + target + ": ";
        if (serr) {
            err.append(serr);
        }
        else {
            err.append("unknown error occurred");
        }
        gzclose(gsrc);
        throw BESInternalError(err, __FILE__, __LINE__);
    }
#endif
    // gzread will read the data in uncompressed. All we have to do is write
    // it to the destination file.
    bool done = false;
    while (!done) {
        int bytes_read = gzread(gsrc, in, CHUNK);
        if (bytes_read == 0) {
            done = true;
        }
        else {
            int bytes_written = write(dest_fd, in, bytes_read); //fwrite(in, 1, bytes_read, dest);
            if (bytes_written < bytes_read) {
                ostringstream strm;
                strm << "Error writing uncompressed data for file " << gsrc << ": " << "wrote "
                        << bytes_written << " " << "instead of " << bytes_read;
                gzclose(gsrc);
#if 0
                fclose(dest);
                remove(target.c_str());
#endif
                throw BESInternalError(strm.str(), __FILE__, __LINE__);
            }
        }
    }

    gzclose(gsrc);
#if 0
    fclose(dest);
#endif
}

