// DODSGlobalIQ.cc

// This file is part of bes, A C++ back-end server implementation framework
// for the OPeNDAP Data Access Protocol.

// Copyright (c) 2004,2005 University Corporation for Atmospheric Research
// Author: Patrick West <pwest@ucar.org>
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
 
// (c) COPYRIGHT University Corporation for Atmostpheric Research 2004-2005
// Please read the full copyright statement in the file COPYRIGHT_UCAR.
//
// Authors:
//      pwest       Patrick West <pwest@ucar.edu>

#include "DODSGlobalIQ.h"
#include "DODSInitList.h"
#include "DODSInitializer.h"
#include "DODSException.h"

bool DODSGlobalIQ::is_initialized = false ;

/** @brief initialize global objects in an orderly fasion.
 *
 * There are multiple levels of initialization, which allows for the
 * ordering of global objects to be created, functions to be called,
 * whatever initialization needs to take place for your application. This
 * function traverses the different levels, in order, calling the first
 * initialization function for that level. The initialization object is
 * DODSGlobalInit, which contains an initialization function, a termination
 * function (if one is provided), and a pointer to the next DODSGlobalInit
 * object. So there can be multiple initialization routines called per
 * level.
 *
 * @param argc number of arguments being passed in the argv parameter. This
 * is the same as the argc command line argument passed to main.
 * @param argv the actual arguments being passed. This is the same as the
 * command line arguments passed to main.
 * @return returns true if initialization is successful, false if
 * initialization fails and the application should terminate. Initialization
 * stops after the first initialization function returns false.
 * @see DODSGlobalInit
 */
bool
DODSGlobalIQ::DODSGlobalInit(int argc, char **argv)
{
    bool retVal = true;
    if( DODSGlobalIQ::is_initialized == false )
    {
	for(int i = 0; i < 5; i++)
	{
	    if(DODSGlobalInitList[i])
	    {
		retVal = DODSGlobalInitList[i]->initialize(argc, argv);
		if(retVal != true)
		{
		    break;
		}
	    }
	}
	DODSGlobalIQ::is_initialized = true ;
    }

    return retVal;
}

/** @brief Runs the termination functions in reverse order of
 * initialization, providing the application to clean up its global objects.
 *
 * This function traverses the levels in reverse order calling the first
 * termination function in that level. 
 *
 * @return Returns true at this time. If termination fails for some reason
 * termination continues on.
 * @see DODSGlobalInit
 */
bool
DODSGlobalIQ::DODSGlobalQuit()
{
    if( DODSGlobalIQ::is_initialized == true )
    {
	for(int i = 4; i >= 0; i--)
	{
	    if(DODSGlobalInitList[i])
	    {
		DODSGlobalInitList[i]->terminate();
	    }
	}
	DODSGlobalIQ::is_initialized = false ;
    }

    return true ;
}

// $Log: DODSGlobalIQ.cc,v $
// Revision 1.3  2004/12/15 17:39:03  pwest
// Added doxygen comments
//
// Revision 1.2  2004/09/09 17:17:12  pwest
// Added copywrite information
//
// Revision 1.1  2004/06/30 20:16:24  pwest
// dods dispatch code, can be used for apache modules or simple cgi script
// invocation or opendap daemon. Built during cedar server development.
//
