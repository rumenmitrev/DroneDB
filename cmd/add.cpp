/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#include <iostream>
#include <filesystem>
#include "add.h"
#include "../libs/ddb.h"

namespace fs = std::filesystem;

namespace cmd {

void Add::setOptions(cxxopts::Options &opts) {
    opts
    .positional_help("[args] [PATHS]")
    .custom_help("add *.JPG")
    .add_options()
    ("d,directory", "Working directory", cxxopts::value<std::string>()->default_value("."))
    ("p,paths", "Paths to add to index (files or directories)", cxxopts::value<std::vector<std::string>>());

    opts.parse_positional({"paths"});
}

std::string Add::description() {
    return "Add files and directories to an index.";
}

void Add::run(cxxopts::ParseResult &opts) {
    if (!opts.count("paths")) {
        printHelp();
    }

    auto db = ddb::open(opts["directory"].as<std::string>(), true);
    ddb::addToIndex(db.get(), opts["paths"].as<std::vector<std::string>>());
}

}


