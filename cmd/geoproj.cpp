/* Copyright 2019 MasseranoLabs LLC

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License. */

#include <iostream>
#include "geoproj.h"
#include "../libs/ddb.h"
#include "../libs/geoproject.h"


namespace cmd {

void GeoProj::setOptions(cxxopts::Options &opts) {
    opts
    .positional_help("[args]")
    .custom_help("geoproj *.JPG -o output/")
    .add_options()
    ("i,images", "Images to project", cxxopts::value<std::vector<std::string>>())
    ("o,output", "Output path (file or directory)", cxxopts::value<std::string>())
    ("a,add", "Automatically add images to index if needed", cxxopts::value<bool>())
    ("d,directory", "Working directory", cxxopts::value<std::string>()->default_value("."));

    opts.parse_positional({"images"});
}

std::string GeoProj::description() {
    return "Project indexed images to georeferenced rasters";
}

void GeoProj::run(cxxopts::ParseResult &opts) {
    if (!opts.count("images") || !opts.count("output")) {
        printHelp();
    }

    auto images = opts["images"].as<std::vector<std::string>>();
    auto output = opts["output"].as<std::string>();
    auto db = ddb::open(opts["directory"].as<std::string>(), true);

    if (opts["add"].count()){
        ddb::addToIndex(db.get(), images);
    }

    ddb::geoProject(db.get(), images, output);
}

}

