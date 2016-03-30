// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2013 OPeNDAP, Inc.
// Authors:  James Gallagher <jgallagher@opendap.org>
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

#include <cassert>

#include <sstream>
#include <vector>

#include <BaseType.h>
#include <Array.h>
#include <Str.h>

#include <Error.h>
#include <DDS.h>
#include <DMR.h>
#include <D4RValue.h>
#include <D4Dimensions.h>
#include <debug.h>
#include <util.h>

#include <BESDebug.h>

#include "BindNameFunction.h"

#define DEBUG_KEY "functions"

using namespace std;
using namespace libdap;

namespace functions {

string bind_shape_info =
        string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
                + "<function name=\"make_array\" version=\"1.0\" href=\"http://docs.opendap.org/index.php/Server_Side_Processing_Functions#bind_shape\">\n"
                + "</function>";

vector<int> parse_dims(const string &shape); // defined in MakeArrayFunction.cc

BaseType *bind_shape_worker(vector<BaseType*> argv)
{
    int argc = argv.size();

    if (argc == 0) {
        Str *response = new Str("info");
        response->set_value(bind_shape_info);
        return response;
    }

    // Check for two args
    if (argc != 2)
        throw Error(malformed_expr, "bind_shape(shape,variable) requires two arguments.");

    string shape = extract_string_argument(argv[0]);

    BaseType *btp = argv[1];

    // string shape = extract_string_argument(argv[0]);
    vector<int> dims = parse_dims(shape);

    Array *array = dynamic_cast<Array*>(btp);
    if (!array) throw Error(malformed_expr, "bind_shape() requires an Array as its second argument.");

    unsigned long vector_size = array->length();
    DBG(cerr << "bind_shape_worker() - vector_size: " << long_to_string(vector_size) << endl);

    array->clear_all_dims();

    unsigned long number_of_elements = 1;
    vector<int>::iterator i = dims.begin();
    while (i != dims.end()) {
        int dimSize = *i;
        number_of_elements *= dimSize;
        if (array->is_dap4()) {
            DBG(cerr << "bind_shape_worker() - Adding DAP4 dimension." << endl);

            // FIXME - I think this creates a memory leak because
            // the D4Dimension will never be deleted by the
            // current implementation of Array which only has a
            // weak pointer to the D4Dimension.
            //
            // NB: The likely fix is to find the Group that holds
            // this variable and add the new D4Dimension to its
            // D4Dimensions object. That will enure it is
            // deleted. jhrg 8/26/14
            D4Dimension *this_dim = new D4Dimension("", dimSize);
            array->append_dim(this_dim);
        }
        else {
            DBG(cerr << "bind_shape_worker() - Adding DAP2 dimension." << endl);
            array->append_dim(dimSize);
        }
        i++;
    } DBG(cerr << "bind_shape_worker() - number_of_elements: " << long_to_string(number_of_elements) << endl);

    if (number_of_elements != vector_size)
        throw Error(malformed_expr,
                "bind_shape(): The product of the new dimensions must match the size of the Array's internal storage vector.");

    return array;
}

/** Bind a shape to a DAP2 Array that is a vector. The product of the dimension
 * sizes must match the number of elements in the vector. This function takes
 * two arguments: A shape expression and a BaseType* to the DAP2 Array that holds
 * the data. In practice, the Array can already have a shape (it's a vector, so
 * that is a shape, e.g.) and this function simply changes that shape. The shape
 * expression is the C bracket notation for array size and is parsed by this
 * function.
 *
 * @param argc A count of the arguments
 * @param argv An array of pointers to each argument, wrapped in a child of BaseType
 * @param btpp A pointer to the return value; caller must delete.
 * @return The newly (re)named variable.
 * @exception Error Thrown for a variety of errors.
 */
void function_dap2_bind_shape(int argc, BaseType * argv[], DDS &, BaseType **btpp)
{
    BESDEBUG(DEBUG_KEY, "function_dap2_bind_shape() - BEGIN" << endl);

    BESDEBUG(DEBUG_KEY, "function_dap2_bind_shape() - Building argument vector for geogrid_worker()" << endl);
    vector<BaseType*> args;
    for(int i=0; i< argc; i++){
        BaseType * bt = argv[i];
        BESDEBUG(DEBUG_KEY, "function_dap2_bind_shape() - Adding argument: "<< bt->name() << endl);
        args.push_back(bt);
    }

    *btpp = bind_shape_worker(args);

    BESDEBUG(DEBUG_KEY, "function_dap2_bind_shape() - END (result: "<< (*btpp)->name() << ")" << endl);
    return;
}

/** Bind a shape to a DAP4 Array that is a vector. The product of the dimension
 * sizes must match the number of elements in the vector. This function takes
 * two arguments: A shape expression and a BaseType* to the DAP2 Array that holds
 * the data. In practice, the Array can already have a shape (it's a vector, so
 * that is a shape, e.g.) and this function simply changes that shape. The shape
 * expression is the C bracket notation for array size and is parsed by this
 * function.
 *
 * @param args The DAP4 function arguments list
 * @param dmr The DMR for the dataset in question
 * @return A pointer to the return value; caller must delete.
 * @exception Error Thrown for a variety of errors.
 */

BaseType *function_dap4_bind_shape(D4RValueList *dvl_args, DMR &dmr)
{
    BESDEBUG(DEBUG_KEY, "function_dap4_bind_shape() - Building argument vector for bbox_union_worker()" << endl);
    vector<BaseType*> args;
    for(unsigned int i=0; i< dvl_args->size(); i++){
        BaseType * bt = dvl_args->get_rvalue(i)->value(dmr);
        BESDEBUG(DEBUG_KEY, "function_dap4_bind_shape() - Adding argument: "<< bt->name() << endl);
        args.push_back(bt);
    }

    BaseType *result = bind_shape_worker(args);

    BESDEBUG(DEBUG_KEY, "function_dap4_bind_shape() - END (result: "<< result->name() << ")" << endl);
    return result; //response.release();
}

} // namesspace functions
