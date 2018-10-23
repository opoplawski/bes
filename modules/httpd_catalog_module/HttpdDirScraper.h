/*
 * HttpdDirScraper.h
 *
 *  Created on: Oct 15, 2018
 *      Author: ndp
 */

#ifndef MODULES_CMR_MODULE_HTTPDDIRSCRAPER_H_
#define MODULES_CMR_MODULE_HTTPDDIRSCRAPER_H_

#include <set>
#include <string>
#include <CatalogNode.h>

namespace httpd_catalog {

class HttpdDirScraper {
private:

    int getNextElementText(const string &page_str, string element_name, int startIndex, string &resultText, bool trim=true) const;
    void createHttpdDirectoryPageMap(std::string url, std::set<bes::CatalogItem *> &items) const;

public:
    HttpdDirScraper();
    virtual ~HttpdDirScraper();

    void createHttpdDirectoryPageMap(std::string url, std::set<std::string> &pageNodes, std::set<std::string> &pageLeave) const;
    bes::CatalogNode *get_node(const string &url, const string &path) const;

};
} // namespace httpd_catalog

#endif /* MODULES_CMR_MODULE_HTTPDDIRSCRAPER_H_ */
