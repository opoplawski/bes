
// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of bes, A C++ implementation of the OPeNDAP
// Hyrax data server

// Copyright (c) 2015 OPeNDAP, Inc.
// Authors: James Gallagher <jgallagher@opendap.org>
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
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
//
// You can contact OPeNDAP, Inc. at PO Box 112, Saunderstown, RI. 02874-0112.

#include "config.h"

#include <string>
#include <sstream>
#include <vector>

#include <Array.h>
#include <Error.h>
#include <util.h>

using namespace std;
using namespace libdap;

namespace functions {

/** Parse the shape 'expression'. The form of the expression is '[' size ']'
 * @note Also used by bind_shape()
 * @return A vector of ints
 */
vector<int> parse_dims(const string &shape)
{
    vector<int> dims;
    istringstream iss(shape);
    string::size_type pos = 0;
    do {
        char brace;
        iss >> brace;
        ++pos;
        // EOF is only found by reading past the last character
        if (iss.eof()) return dims;

        if (brace != '[' || iss.fail())
            throw Error(malformed_expr,
                    "make_array(): Expected a left brace at position " + long_to_string(pos) + " in shape expression: "
                            + shape);

        int size = 0;
        iss >> size;
        ++pos;
        if (size == 0 || iss.fail())
            throw Error(malformed_expr,
                    "make_array(): Expected an integer at position " + long_to_string(pos) + " in shape expression: "
                            + shape);
        dims.push_back(size);

        iss >> brace;
        ++pos;
        if (brace != ']' || iss.fail())
            throw Error(malformed_expr,
                    "make_array(): Expected a right brace at position " + long_to_string(pos) + " in shape expression: "
                            + shape);
    } while (!iss.eof());

    return dims;
}

/**
 * Test for acceptable array types for the N-1 arguments of roi().
 * Throw Error if the array is not valid for this function
 *
 * @param btp Test this variable.
 * @param rank If given and not zero, the Array must have rank equal to
 * or at most one greater than the value of this parameter.
 * @exception Error thrown if the array is not valid
 */
void check_number_type_array(BaseType *btp, unsigned int rank /* = 0 */)
{
    if (!btp)
        throw InternalErr(__FILE__, __LINE__, "roi() function called with null variable.");

    if (btp->type() != dods_array_c)
        throw Error("In function roi(): Expected argument '" + btp->name() + "' to be an Array.");

    Array *a = static_cast<Array *>(btp);
    if (!a->var()->is_simple_type() || a->var()->type() == dods_str_c || a->var()->type() == dods_url_c)
        throw Error("In function roi(): Expected argument '" + btp->name() + "' to be an Array of numeric types.");

    if (rank && !(a->dimensions() == rank || a->dimensions() == rank+1))
        throw Error("In function roi(): Expected the array '" + a->name() +"' to be rank " + long_to_string(rank) + " or " + long_to_string(rank+1) + ".");
}

} // namespace fucntions