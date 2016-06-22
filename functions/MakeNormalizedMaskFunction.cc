// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2015 OPeNDAP, Inc.
// Authors: Dan Holloway <dholloway@opendap.org>
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

//#define DODS_DEBUG 1

#include <cassert>

#include <sstream>
#include <vector>
#include <algorithm>
#include <functional>

#include <Type.h>
#include <BaseType.h>
#include <Byte.h>
#include <Int16.h>
#include <UInt16.h>
#include <Int32.h>
#include <UInt32.h>
#include <Float32.h>
#include <Float64.h>
#include <Str.h>
#include <Url.h>
#include <Array.h>
#include <Grid.h>
#include <Error.h>
#include <DDS.h>

#include <DMR.h>
#include <D4Group.h>
#include <D4RValue.h>

#include <debug.h>
#include <util.h>
#include <functions_util.h>

#include <BaseTypeFactory.h>

#include <BESDebug.h>

#include "MakeNormalizedMaskFunction.h"
#include "Odometer.h"
#include "functions_util.h"

using namespace libdap;
using namespace std;

namespace functions {

string make_normalized_mask_info =
        string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
                + "<function name=\"make_normalized_mask\" version=\"1.0\" href=\"http://docs.opendap.org/index.php/Server_Side_Processing_Functions#make_normalized_mask\">\n"
                + "</function>";


// Dan: In this function I made the vector<dods_byte> a reference so that changes to
// it will be accessible to the caller without having to return the vector<> mask
// (it could be large). This also means that it won't be passed on the stack
/**
 * Given a vector of Grid maps (effectively domain variables) and a 'list'
 * of tuples where N tuples and M maps means N x M values n the list, build
 * a mask that can be used to filter an array of N dimensions selecting only
 * locations (range values) that match those domain values. The list of M tuples
 * is organized so that the first 0, ..., N-1 (the first N values) are M0, the
 * next N values are M1, and so on.
 *
 * @todo Once we're done testing, replace .at() with []
 *
 * @param dims Maps from a Grid that supply the domain values' indices
 * @param tuples The domain values
 * @param mask The resulting mask for an N dimensional array, where N is the number
 * of arrays in the Vector<> dims
 */

template<typename T>
vector< vector<int> > make_normalized_mask_helper(const vector<Array*> dims, Array *tuples)
{

    vector< vector<double> > dim_value_vecs(dims.size());
    int i = 0;  // index the dim_value_vecs vector of vectors;
    for (vector<Array*>::const_iterator d = dims.begin(), e = dims.end(); d != e; ++d) {
        // This version of extract...() takes the vector<double> by reference:
        // In util.cc/h: void extract_double_array(Array *a, vector<double> &dest)
        extract_double_array(*d, dim_value_vecs.at(i++));
    }

    // Construct and Odometer used to calculate offsets
    Odometer::shape shape(dims.size());

    int j = 0;  // index the shape vector for an Odometer;
    for (vector<Array*>::const_iterator d = dims.begin(), e = dims.end(); d != e; ++d) {
	shape[j++] = (*d)->length();
    }

    Odometer odometer(shape);

    // Copy the 'tuple' data to a simple vector<T>
    vector<T> data(tuples->length());
    tuples->value(&data[0]);

    // Iterate over the tuples, searching the dimensions for their values
    int nDims = dims.size();
    int nTuples = data.size() / nDims;

    vector<int> xLoc(nTuples,0);  // Create 'xLoc' array, initialized with zero's
    vector<int> yLoc(nTuples,0);  // Create 'yLoc' array, initialized with zero's

    // NB: 'data' holds the tuple values

    // unsigned int tuple_offset = 0;       // an optimization...
    for (int n = 0; n < nTuples; ++n) {
        vector<double> tuple(nDims);
        // Build the next tuple
        for (int dim = 0; dim < nDims; ++dim) {
            // could replace 'tuple * nDims' with 'tuple_offset'
            tuple[dim] = data[n * nDims + dim];
        }

        DBG(cerr << "tuple: ");
        DBGN(copy(tuple.begin(), tuple.end(), ostream_iterator<int>(std::cerr, " ")));
        DBGN(cerr << endl);

        // find indices for tuple-values in the specified
	// target-grid dimensions
	vector<int> indices = find_value_indices(tuple, dim_value_vecs);
	DBG(cerr << "indices: ");
        DBGN(copy(indices.begin(), indices.end(), ostream_iterator<int>(std::cerr, " ")));
        DBGN(cerr << endl);

        // if all of the indices are >= 0, then add this point to the mask
        if (all_indices_valid(indices)) {

	    // Pass identified indices to Odometer, it will automatically
	    // calculate offset within defined 'shape' using those index values.
	    // Result of set_indices() will update d_offset value, accessible
	    // using the Odometer::offset() accessor.
	    for (std::vector<int>::const_iterator i=indices.begin(); i != indices.end(); i+=2) {
		xLoc.push_back( *i );
		yLoc.push_back( *(i+1) );
	    }
        }
    }

    int cfPixels = 7;
    int deltaX, deltaY;
    int mp1, np1;
    float factorX, factorY, d;

    std::vector<int>::iterator xDx, xEnd, xStart;
    xEnd = xLoc.end(); 
    xStart = xLoc.begin();

    std::vector<int>::iterator yDx, yEnd, yStart;
    yEnd = yLoc.end(); 
    yStart = yLoc.begin();

    vector< vector<int> > result;

    for (xDx = xLoc.begin(),yDx = yLoc.begin(); xDx != xLoc.end() && yDx != yLoc.end(); ++xDx, ++yDx) {
    //for (int i = segStart; i < segEnd; i++ ) {

	if ( xLoc.size() < 5 ) {    // Granted this is loop-invariant, but easier to read if inside loop and it's only 5 iterations.
	    deltaX = *xEnd - *xStart;	    //deltaX = xLocation[segEnd] - xLocation[segStart];
	    deltaY = *yEnd - *yStart;	    //deltaY = yLocation[segEnd] - yLocation[segStart];
	}
	else if ( xDx < xStart+2 ) {	            //else if ( i < (segStart + 2) ) {
	    deltaX = *(xStart+3) - *xStart;	    //deltaY = yLocation[segStart + 3] - yLocation[segStart];
	    deltaY = *(yStart+3) - *yStart;	    //deltaX = xLocation[segStart + 3] - xLocation[segStart];
	}
	else if ( xDx > xEnd+2 ) {	            //else if ( i > (segEnd + 2) ) {
	    deltaX = *xEnd - *(xEnd - 3);	    //deltaX = xLocation[segEnd] - xLocation[segEnd -3];
	    deltaY = *yEnd - *(yEnd - 3);	    //deltaY = yLocation[segEnd] - yLocation[segEnd -3];
	}
	else {
	    deltaX = *(xDx+2) - *(xDx-2); 	    //deltaX = xLocation[i+2] - xLocation[i-2];
	    deltaY = *(yDx+2) - *(yDx-2); 	    //deltaY = yLocation[i+2] - yLocation[i-2];
	}

	d = sqrt(deltaX * deltaX + deltaY * deltaY);
	
	factorX = deltaX / d;
	factorY = deltaY / d;

	vector<int> CF_offsets;

	for (int ip = -cfPixels; ip < cfPixels; ip++) {   // cfPixels ranges from -7 to +7
	    mp1 = min((int)xLoc.size(), max(1,((int)(*xDx - round(ip * factorX)))));
	    np1 = min((int)yLoc.size(), max(1,((int)(*yDx - round(ip * factorY))))); //np1 = min(lenY, max(1,(yLocation[i]- round(ip * factorY)))); 
	    //jp = ip + cfPixels + 1;  // jp index ranges from 0 ==> 15;

	    vector<int> CF_Locs;

	    // Find normalized X/Y's to tuple location 
	    
	    CF_Locs.push_back( mp1 - *xDx );  //CFmLocation[jp] = mp1 - xLocation[i];
	    CF_Locs.push_back( np1 - *yDx );  //CFnLocation[jp] = np1 = yLocation[i];

	    odometer.set_indices(CF_Locs);    // Find offset of normalized point.
	    DBG(cerr << "odometer.offset(): " << odometer.offset() << endl);
	    
	    CF_offsets.push_back(odometer.offset());  // Store target array offset for normalized point
	}

	result.push_back(CF_offsets);  // Store all 15 normalized points in result
    }

    return result;
}


/** Given a ...

 @param argc A count of the arguments
 @param argv An array of pointers to each argument, wrapped in a child of BaseType
 @param btpp A pointer to the return value; caller must delete.

 @return The mask variable, represented using Byte
 @exception Error Thrown if target variable is not a DAP2 Grid
 **/
void function_dap2_make_normalized_mask(int argc, BaseType * argv[], DDS &, BaseType **btpp)
{
    if (argc == 0) {
        Str *response = new Str("info");
        response->set_value(make_normalized_mask_info);
        *btpp = response;
        return;
    }

    // Check for three args or more. The first two must be strings.                                                                                           
    DBG(cerr << "argc = " << argc << endl);
    if (argc < 3)
        throw Error(malformed_expr,
<<<<<<< HEAD
		    "make_normalized_mask(shape_string,[dim1,...],$TYPE(dim1_value0,dim2_value0,...)) requires at least three arguments.");

    if (argv[0]->type() != dods_str_c)
        throw Error(malformed_expr, "make_normalized_mask(): first argument must point to a string variable.");

    string shape_str = extract_string_argument(argv[0]);
    vector<int> shape = parse_dims(shape_str);

    unsigned int nDims = shape.size();

    // read argv[1] -> argv[1+numberOfDims]; the grid dimensions where we will find the values                                                                
    // of the mask tuples. Also note that the number of dims (shape.size() above) should be the                                                               
    // same as argc-2.                                                                                                                                        
    assert(nDims == (unsigned int)argc-2);
=======
                "make_normalized_mask(target,nDims,dim1[,dim2...],$TYPE(dim1_value0,[dim2_value0,]...)) requires at least four arguments.");
>>>>>>> ff2ba5b7db518dd34d3cd1d63a36c4689a5791b6

    // string requestedTargetName = extract_string_argument(argv[0]);
    // BESDEBUG("functions", "Requested target variable: " << requestedTargetName << endl);

    BaseType *btp = argv[0];

    // read argv[2] -> argv[2+numberOfDims]; the grid dimensions where we will find the values
    // of the mask tuples.
    vector<Array*> dims;
    for (unsigned int i = 0; i < nDims; i++) {

        btp = argv[1 + i];
        if (btp->type() != dods_array_c) {
            throw Error(malformed_expr,
                    "make_normalized_mask(): dimension-name arguments must point to Grid variable dimensions.");
        }

        Array *a = static_cast<Array*>(btp);

	// Check that each map size matches the 'shape' info passed in the first arg.                                                                         
        // This might not be the case for DAP4 (or if we change this to support level                                                                         
        // 2 swath data).                                                                                                                                     
        assert(a->dimension_size(a->dim_begin()) == shape.at(i));

        a->read();
	a->set_read_p(true);
        dims.push_back(a);
    }

    BESDEBUG("functions", "number of dimensions: " << dims.size() << endl);

    btp = argv[argc - 1];
    if (btp->type() != dods_array_c) {
        throw Error(malformed_expr, "make_normalized_mask(): last argument must be an array.");
    }

    check_number_type_array(btp);  // Throws an exception if not a numeric type.

    Array *tuples = static_cast<Array*>(btp);

    // Create the 'normalization_mask' array using the number of input tuples times 15, to represent a 2-dimensional array size [15][n-tuples].

    vector< vector<int> > mask;

    switch (tuples->var()->type()) {
    // All mask values are stored in Byte DAP variables by the stock argument parser
    // except values too large; those are stored in a UInt32
    case dods_byte_c:
        mask = make_normalized_mask_helper<dods_byte>(dims, tuples);
        break;

    case dods_int16_c:
        mask = make_normalized_mask_helper<dods_int16>(dims, tuples);
        break;

    case dods_uint16_c:
        mask = make_normalized_mask_helper<dods_uint16>(dims, tuples);
        break;

    case dods_int32_c:
        mask = make_normalized_mask_helper<dods_int32>(dims, tuples);
        break;

    case dods_uint32_c:
        mask = make_normalized_mask_helper<dods_uint32>(dims, tuples);
        break;

    case dods_float32_c:
        mask = make_normalized_mask_helper<dods_float32>(dims, tuples);
        break;

    case dods_float64_c:
        mask  = make_normalized_mask_helper<dods_float64>(dims, tuples);
        break;

    case dods_str_c:
    case dods_url_c:
    default:
        throw InternalErr(__FILE__, __LINE__,
                "make_normalized_mask(): Expect an array of mask points (numbers) but found something else instead.");
    }

    // BESDEBUG("function", "function_dap2_make_mask() -target " << requestedTargetName << " -nDims " << nDims << endl);

    vector<int> normalizedMask;

    vector< vector<int> >::const_iterator row;

    for (row = mask.begin(); row != mask.end(); ++row) {
	std::copy(row->begin(), row->end(), std::back_inserter(normalizedMask));
    }
    
    Array *dest = new Array("mask", 0);	// The ctor for Array copies the prototype pointer...
    BaseTypeFactory btf;
    dest->add_var_nocopy(new UInt32("mask"));	// ... so use add_var_nocopy() to add it instead
    
    dest->append_dim(mask.size());              // set dimensions to 2d mask.sizes()
    dest->append_dim(mask.at(0).size());

    dest->set_value(normalizedMask, normalizedMask.size()); 
    dest->set_read_p(true);
    *btpp = dest;

}

} // namespace functions
