/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H

#include <stdexcept>
namespace ddb{

class AppException : public std::runtime_error{
    using std::runtime_error::runtime_error;
};
class DBException : public AppException{
    using AppException::AppException;
};
class SQLException : public DBException{
    using DBException::DBException;
};
class FSException : public AppException{
    using AppException::AppException;
};
class TimezoneException : public AppException{
    using AppException::AppException;
};
class IndexException : public AppException{
    using AppException::AppException;
};
class InvalidArgsException : public AppException{
    using AppException::AppException;
};
class GDALException : public AppException{
    using AppException::AppException;
};
class CURLException : public AppException{
    using AppException::AppException;
};

}
#endif // EXCEPTIONS_H
