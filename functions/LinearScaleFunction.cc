// -*- mode: c++; c-basic-offset:4 -*-

// This file is part of libdap, A C++ implementation of the OPeNDAP Data
// Access Protocol.

// Copyright (c) 2013 OPeNDAP, Inc.
// Authors: Nathan Potter <npotter@opendap.org>
//         James Gallagher <jgallagher@opendap.org>
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

#include <sstream>

#include <BaseType.h>
#include <DMR.h>
#include <D4Group.h>
#include <Float64.h>
#include <Str.h>
#include <Array.h>
#include <Grid.h>
#include "D4RValue.h"

#include <Error.h>
#include <DDS.h>
#include <D4Attributes.h>

#include <debug.h>
#include <util.h>

#include "BESDebug.h"

#include "LinearScaleFunction.h"

#define DEBUG_KEY "functions"

using namespace libdap;

namespace functions {

string linear_scale_info =
        string("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n")
                + "<function name=\"linear_scale\" version=\"1.0b1\" href=\"http://docs.opendap.org/index.php/Server_Side_Processing_Functions#linear_scale\">\n"
                + "</function>";

// These static functions could be moved to a class that provides a more
// general interface for COARDS/CF someday. Assume each BaseType comes bundled
// with an attribute table.

static double string_to_double(const char *val)
{
    istringstream iss(val);
    double v;
    iss >> v;

    double abs_val = fabs(v);
    if (abs_val > DODS_DBL_MAX || (abs_val != 0.0 && abs_val < DODS_DBL_MIN))
        throw Error(malformed_expr, string("Could not convert the string '") + val + "' to a double.");

    return v;
}

/** Look for any one of a series of attribute values in the attribute table
 for \e var. This function treats the list of attributes as if they are ordered
 from most to least likely/important. It stops when the first of the vector of
 values is found. If the variable (var) is a Grid, this function also looks
 at the Grid's Array for the named attributes. In all cases it returns the
 first value found.
 @param var Look for attributes in this BaseType variable.
 @param attributes A vector of attributes; the first one found will be returned.
 @return The attribute value in a double. */
static double get_attribute_double_value(BaseType *var,
        vector<string> &attributes) {
    // This code also builds a list of the attribute values that have been
    // passed in but not found so that an informative message can be returned.
    AttrTable &attr = var->get_attr_table();
    string attribute_value = "", attribute_name="";
    string checkedAttrNames = "";

    D4Attributes *d4Attr = var->attributes();
    if(!d4Attr->empty()){
        BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - Checking DAP4 Attributes (DAP4 attribute table is not empty.)" << endl);
        vector<string>::iterator i = attributes.begin();
        while (attribute_value == "" && i != attributes.end()) {
            /// This is building an error message that may not be used.
            string aName = *i;
            checkedAttrNames += aName;
            if (!checkedAttrNames.empty())
                checkedAttrNames += ", ";

           BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - Looking for DAP4 attribute: '" << aName << "'"<< endl);
           D4Attribute *requestedAttr = d4Attr->find(aName);
           if(requestedAttr!=0){
               BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - Found for DAP4 attribute: '" << aName << "'"<< endl);
               attribute_value = requestedAttr->value(0);
               attribute_name = aName;
           }
           else {
               BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - MISSING DAP4 attribute: '" << aName << "'"<< endl);
           }
           ++i;
        }

        BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - Using DAP4 attribute: '"<< attribute_name << "' value: " << attribute_value << endl);
    }
    else {
        BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - Checking DAP2 attributes." << endl);
       vector<string>::iterator i = attributes.begin();
        while (attribute_value == "" && i != attributes.end()) {
            /// This is building an error message that may not be used.
            checkedAttrNames += *i;
            if (!checkedAttrNames.empty())
                checkedAttrNames += ", ";

            attribute_value = attr.get_attr(*i++);
        }
        BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - Using DAP2 attribute: '"<< *i << "' value: " << attribute_value << endl);
    }


    // If the value string is empty, then look at the grid's array (if it's a
    // grid) or throw an Error.
    if (attribute_value.empty()) {
        if (var->type() == dods_grid_c)
            return get_attribute_double_value( dynamic_cast<Grid&>(*var).get_array(), attributes);
        else
            throw Error(malformed_expr,
                    string("No COARDS/CF '")
                            + checkedAttrNames.substr(0, checkedAttrNames.length() - 2)
                            + "' attribute was found for the variable '"
                            + var->name() + "'.");
    }

    return string_to_double(remove_quotes(attribute_value).c_str());
}

static double get_attribute_double_value(BaseType *var, const string &attribute)
{
    string attribute_value;

    D4Attributes *d4Attr = var->attributes();
    if(!d4Attr->empty()){
        // BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - D4Attributes: "<< endl << *d4Attr << endl);
        D4Attribute *requestedAttr = d4Attr->find(attribute);
        if(requestedAttr!=0)
            attribute_value = requestedAttr->value(0);
        BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - Using DAP4 attribute: '"<< attribute << "' value: " << attribute_value << endl);
    }
    else {
        AttrTable &attr = var->get_attr_table();
        attribute_value = attr.get_attr(attribute);
        BESDEBUG(DEBUG_KEY, "get_attribute_double_value() - Using DAP2 attribute: '"<< attribute << "' value: " << attribute_value << endl);
    }

    // If the value string is empty, then look at the grid's array (if it's a
    // grid or throw an Error.
    if (attribute_value.empty()) {
        if (var->type() == dods_grid_c)
            return get_attribute_double_value(dynamic_cast<Grid&>(*var).get_array(), attribute);
        else
            throw Error(malformed_expr,
                    string("No COARDS '") + attribute + "' attribute was found for the variable '" + var->name()
                            + "'.");
    }

    return string_to_double(remove_quotes(attribute_value).c_str());
}

static double get_y_intercept(BaseType *var)
{
    vector<string> attributes;
    attributes.push_back("add_offset");
    attributes.push_back("add_off");
    double b =  get_attribute_double_value(var, attributes);
    BESDEBUG(DEBUG_KEY, "get_y_intercept() - returning: " << b << endl);
    return b;
}

static double get_slope(BaseType *var)
{
    double d =  get_attribute_double_value(var, "scale_factor");
    BESDEBUG(DEBUG_KEY, "get_slope() - returning: " << d << endl);
    return d;
}

static double get_missing_value(BaseType *var)
{
    double mv = get_attribute_double_value(var, "missing_value");
    BESDEBUG(DEBUG_KEY, "get_missing_value() - returning: " << mv << endl);
    return mv;
}

BaseType *compute_linear_scale_result(BaseType *bt, double m, double b, double missing, bool use_missing)
{
    // Read the data, scale and return the result. Must replace the new data
    // in a constructor (i.e., Array part of a Grid).
    BaseType *dest = 0;
    double *data;
    if (bt->type() == dods_grid_c) {
        // Grab the whole Grid; note that the scaling is done only on the array part
        Grid &source = dynamic_cast<Grid&>(*bt);

        BESDEBUG(DEBUG_KEY, "compute_linear_scale_result() - Grid send_p: " << source.send_p() << endl);
        BESDEBUG(DEBUG_KEY,
                "compute_linear_scale_result() - Grid Array send_p: " << source.get_array()->send_p() << endl);

        // Read the grid; set send_p since Grid is a kind of constructor and
        // read will only be called on it's fields if their send_p flag is set
        source.set_send_p(true);
        source.read();

        // Get the Array part and read the values
        Array *a = source.get_array();
        //a->read();
        data = extract_double_array(a);

        // Now scale the data.
        int length = a->length();
        for (int i = 0; i < length; ++i)
            data[i] = data[i] * m + b;

        // Copy source Grid to result Grid. Could improve on this by not using this
        // trick since it copies all of 'source' to 'dest', including the main Array.
        // The next bit of code will replace those values with the newly scaled ones.
        Grid *result = new Grid(source);

        // Now load the transferred values; use Float64 as the new type of the result
        // Grid Array.
        result->get_array()->add_var_nocopy(new Float64(source.name()));
        result->get_array()->set_value(data, length);
        delete[] data;

        // FIXME result->set_send_p(true);
        BESDEBUG(DEBUG_KEY, "compute_linear_scale_result() - Grid send_p: " << source.send_p() << endl);
        BESDEBUG(DEBUG_KEY,
                "compute_linear_scale_result() - Grid Array send_p: " << source.get_array()->send_p() << endl);

        dest = result;
    }
    else if (bt->is_vector_type()) {
        Array &source = dynamic_cast<Array&>(*bt);
        // If the array is really a map, make sure to read using the Grid
        // because of the HDF4 handler's odd behavior WRT dimensions.
        if (source.get_parent() && source.get_parent()->type() == dods_grid_c) {
            source.get_parent()->set_send_p(true);
            source.get_parent()->read();
        }
        else
            source.read();

        data = extract_double_array(&source);
        int length = source.length();
        for (int i = 0; i < length; ++i)
            data[i] = data[i] * m + b;

        Array *result = new Array(source);

        result->add_var_nocopy(new Float64(source.name()));
        result->set_value(data, length);

        delete[] data; // val2buf copies.

        dest = result;
    }
    else if (bt->is_simple_type() && !(bt->type() == dods_str_c || bt->type() == dods_url_c)) {
        double data = extract_double_value(bt);
        if (!use_missing || !double_eq(data, missing)) data = data * m + b;

        Float64 *fdest = new Float64(bt->name());

        fdest->set_value(data);
        // dest->val2buf(static_cast<void*> (&data));
        dest = fdest;
    }
    else {
        throw Error(malformed_expr, "The linear_scale() function works only for numeric Grids, Arrays and scalars.");
    }

    return dest;
}

BaseType *linear_scale_worker(vector<BaseType*> argv){
    int argc = argv.size();

    if (argc == 0) {
        Str *response = new Str("info");
        response->set_value(linear_scale_info);
        return response;
    }

    // Check for 1 or 3 arguments: 1 --> use attributes; 3 --> m & b supplied
    DBG(cerr << "argc = " << argc << endl);
    if (!(argc == 1 || argc == 3 || argc == 4))
        throw Error(malformed_expr,
                "Wrong number of arguments to linear_scale(). See linear_scale() for more information");

    // Get m & b
    bool use_missing = false;
    double m, b, missing = 0.0;
    if (argc == 4) {
        m = extract_double_value(argv[1]);
        b = extract_double_value(argv[2]);
        missing = extract_double_value(argv[3]);
        use_missing = true;
    }
    else if (argc == 3) {
        m = extract_double_value(argv[1]);
        b = extract_double_value(argv[2]);
        use_missing = false;
    }
    else {
        //BESDEBUG(DEBUG_KEY, "linear_scale_worker() - argv[0]: "<< *argv[0] << endl);

        m = get_slope(argv[0]);

        // This is really a hack; on a fair number of datasets, the y intercept
        // is not given and is assumed to be 0. Here the function looks and
        // catches the error if a y intercept is not found.
        try {
            b = get_y_intercept(argv[0]);
        }
        catch (Error &e) {
            b = 0.0;
        }

        // This is not the best plan; the get_missing_value() function should
        // do something other than throw, but to do that would require mayor
        // surgery on get_attribute_double_value().
        try {
            missing = get_missing_value(argv[0]);
            use_missing = true;
        }
        catch (Error &e) {
            use_missing = false;
        }
    }

    BESDEBUG(DEBUG_KEY,
            "linear_scale_worker() - m: " << m << ", b: " << b << ", use_missing: " << use_missing << ", missing: " << missing << endl);

    return compute_linear_scale_result(argv[0], m, b, missing, use_missing);
}




/** Given a BaseType, scale it using 'y = mx + b'. Either provide the
 constants 'm' and 'b' or the function will look for the COARDS attributes
 'scale_factor' and 'add_offset'.

 @param argc A count of the arguments
 @param argv An array of pointers to each argument, wrapped in a child of BaseType
 @param btpp A pointer to the return value; caller must delete.

 @return The scaled variable, represented using Float64
 @exception Error Thrown if scale_factor is not given and the COARDS
 attributes cannot be found OR if the source variable is not a
 numeric scalar, Array or Grid. */
void function_dap2_linear_scale(int argc, BaseType * argv[], DDS &, BaseType **btpp)
{
    BESDEBUG(DEBUG_KEY, "function_dap2_linear_scale() - BEGIN" << endl);

    BESDEBUG(DEBUG_KEY, "function_dap2_linear_scale() - Building argument vector for linear_scale_worker()" << endl);
    vector<BaseType*> args;
    for(int i=0; i< argc; i++){
        BaseType * bt = argv[i];
        BESDEBUG(DEBUG_KEY, "function_dap4_linear_scale() - Adding argument: "<< bt->name() << endl);
        //BESDEBUG(DEBUG_KEY, "function_dap2_linear_scale() - BaseType Retrieved from DDS: "<< *bt << endl);
        args.push_back(bt);
    }

    *btpp = linear_scale_worker(args);

    BESDEBUG(DEBUG_KEY, "function_dap2_linear_scale() - END (result: "<< (*btpp)->name() << ")" << endl);
    return;
}

/** Given a BaseType, scale it using 'y = mx + b'. Either provide the
 constants 'm' and 'b' or the function will look for the COARDS attributes
 'scale_factor' and 'add_offset'.


 @return The scaled variable, represented using Float64
 @exception Error Thrown if scale_factor is not given and the COARDS
 attributes cannot be found OR if the source variable is not a
 numeric scalar, Array or Grid. */
BaseType *function_dap4_linear_scale(D4RValueList *dvl_args, DMR &dmr)
{
    BESDEBUG(DEBUG_KEY, "function_dap4_linear_scale() - Building argument vector for linear_scale_worker()" << endl);
    //BESDEBUG(DEBUG_KEY, "function_dap4_linear_scale() - DMR.root(): "<< endl << *dmr.root() << endl);


    vector<BaseType*> args;
    for(unsigned int i=0; i< dvl_args->size(); i++){
        BaseType * bt = dvl_args->get_rvalue(i)->value(dmr);
        BESDEBUG(DEBUG_KEY, "function_dap4_linear_scale() - Adding argument: "<< bt->name() << endl);
        //BESDEBUG(DEBUG_KEY, "function_dap4_linear_scale() - BaseType Retrieved from DDS: "<< *bt << endl);
        args.push_back(bt);
        bt = dmr.root()->var(bt->name());
    }

    BaseType *result = linear_scale_worker(args);

    BESDEBUG(DEBUG_KEY, "function_dap4_linear_scale() - END (result: "<< result->name() << ")" << endl);
    return result;


}

} // namesspace functions
