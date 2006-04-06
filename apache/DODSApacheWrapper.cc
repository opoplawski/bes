// DODSApacheWrapper.cc

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

#include <string>

using std::string ;

#include "DODSApacheWrapper.h"
#include "DODSApacheRequests.h"
#include "DODSApache.h"
#include "DODSProcessEncodedString.h"
#include "DODSGlobalIQ.h"

DODSApacheWrapper::DODSApacheWrapper()
{
    _data_request = 0 ;
    _user_name = 0 ;
    _requests = 0 ;
    DODSGlobalIQ::DODSGlobalInit( 0, 0 ) ;
}

DODSApacheWrapper::~DODSApacheWrapper()
{
    if( _data_request )
    {
	delete [] _data_request ;
	_data_request = 0 ;
    }
    if( _user_name )
    {
	delete [] _user_name ;
	_user_name = 0 ;
    }
    DODSGlobalIQ::DODSGlobalQuit() ;
}

/** @brief Execute the given request using DODSApache interface

    @param re DODSDataRequestInterface filled in by the apache module holding
    the request information.
    @return Whether the request was successful or not
    @see _DODSDataRequestInterface
 */
int
DODSApacheWrapper::call_DODS( const DODSDataRequestInterface & re )
{
    DODSApache dods( re ) ;
    int ret = dods.execute_request() ;
    return ret ;
}

/** @brief Find the request from the URL and convert it to readable format

    @param s URL to convert into an OpenDAP request
 */
void
DODSApacheWrapper::process_request(const char*s)
{
    DODSProcessEncodedString h( s ) ;
    string str = h.get_key( "request" ) ;
    _requests = new DODSApacheRequests( str ) ;
}

const char *
DODSApacheWrapper::get_first_request()
{
    if( _requests )
    {
	DODSApacheRequests::requests_citer rcurr = _requests->get_first_request() ;
	DODSApacheRequests::requests_citer rend = _requests->get_end_request() ;
	if( rcurr == rend )
	    return 0 ;
	return (*rcurr).c_str() ;
    }
    return 0 ;
}

const char *
DODSApacheWrapper::get_next_request()
{
    if( _requests )
    {
	static DODSApacheRequests::requests_citer rcurr = _requests->get_first_request() ;
	static DODSApacheRequests::requests_citer rend = _requests->get_end_request() ;
	if( rcurr == rend )
	    return 0 ;
	rcurr++ ;
	if( rcurr == rend )
	    return 0 ;
	return (*rcurr).c_str() ;
    }
    return 0 ;
}

/** @brief Find the username from the URL and convert it to readable format

    @param s URL to convert into an OpenDAP request
    @return Resulting OpenDAP user name
 */
const char *
DODSApacheWrapper::process_user(const char*s)
{
    DODSProcessEncodedString h( s ) ;
    string str = h.get_key( "username" ) ;
    if( str == "" )
    {
	_user_name = new char[strlen( str.c_str() ) + 1] ;
	strcpy( _user_name, str.c_str() ) ;
    }
    else
    {
	_user_name = new char[strlen( str.c_str() ) + 20] ;
	sprintf( _user_name, "OpenDAP.remoteuser=%s", str.c_str() ) ;
    }
    return _user_name ;
}

// $Log: DODSApacheWrapper.cc,v $
