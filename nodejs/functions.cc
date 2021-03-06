/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
#include <sstream>
#include "functions.h"
#include "../libs/ddb.h"
#include "../libs/info.h"
#include "../libs/thumbs.h"
#include "../libs/entry.h"

NAN_METHOD(getVersion) {
    info.GetReturnValue().Set(Nan::New(ddb::getVersion()).ToLocalChecked());
}

NAN_METHOD(typeToHuman) {
    if (info.Length() != 1){
        Nan::ThrowError("Invalid number of arguments");
        return;
    }
    if (!info[0]->IsNumber()){
        Nan::ThrowError("Argument 0 must be a number");
        return;
    }

    ddb::EntryType t = static_cast<ddb::EntryType>(Nan::To<int>(info[0]).FromJust());
    info.GetReturnValue().Set(Nan::New(ddb::typeToHuman(t)).ToLocalChecked());
}

class ParseFilesWorker : public Nan::AsyncWorker {
 public:
  ParseFilesWorker(Nan::Callback *callback, const std::vector<std::string> &input, ddb::ParseFilesOpts &pfOpts)
    : AsyncWorker(callback, "nan:ParseFilesWorker"),
      input(input), pfOpts(pfOpts){}
  ~ParseFilesWorker() {}

  void Execute () {
    try{
        ddb::parseFiles(input, s, pfOpts);
    }catch(ddb::AppException &e){
        SetErrorMessage(e.what());
    }
  }

  void HandleOKCallback () {
     Nan::HandleScope scope;

     Nan::JSON json;
     Nan::MaybeLocal<v8::Value> result = json.Parse(v8::String::NewFromUtf8(v8::Isolate::GetCurrent(), s.str().c_str()).ToLocalChecked());

     v8::Local<v8::Value> argv[] = {
         Nan::Null(),
         result.ToLocalChecked()
     };
     callback->Call(2, argv, async_resource);
   }

 private:
    std::vector<std::string> input;
    std::ostringstream s;
    ddb::ParseFilesOpts pfOpts;
};


NAN_METHOD(parseFiles) {
    if (info.Length() != 3){
        Nan::ThrowError("Invalid number of arguments");
        return;
	}
    if (!info[0]->IsArray()){
        Nan::ThrowError("Argument 0 must be an array");
        return;
    }
    if (!info[1]->IsObject()){
        Nan::ThrowError("Argument 1 must be an object");
        return;
    }
    if (!info[2]->IsFunction()){
        Nan::ThrowError("Argument 2 must be a function");
        return;
    }

    // v8 array --> c++ std vector
    auto input = info[0].As<v8::Array>();
    auto ctx = info.GetIsolate()->GetCurrentContext();
    std::vector<std::string> in;
    for (unsigned int i = 0; i < input->Length(); i++){
        Nan::Utf8String str(input->Get(ctx, i).ToLocalChecked());
        in.push_back(std::string(*str));
    }

    // Parse options
    ddb::ParseEntryOpts peOpts;
    v8::Local<v8::String> k = Nan::New<v8::String>("withHash").ToLocalChecked();
    v8::Local<v8::Object> obj = info[1].As<v8::Object>();

    if (Nan::HasRealNamedProperty(obj, k).FromJust()){
        peOpts.withHash = Nan::To<bool>(Nan::GetRealNamedProperty(obj, k).ToLocalChecked()).FromJust();
    }

    k = Nan::New<v8::String>("stopOnError").ToLocalChecked();
    if (Nan::HasRealNamedProperty(obj, k).FromJust()){
        peOpts.stopOnError = Nan::To<bool>(Nan::GetRealNamedProperty(obj, k).ToLocalChecked()).FromJust();
    }

    ddb::ParseFilesOpts pfOpts;
    pfOpts.format = "json";

    k = Nan::New<v8::String>("recursive").ToLocalChecked();
    if (Nan::HasRealNamedProperty(obj, k).FromJust()){
        pfOpts.recursive = Nan::To<bool>(Nan::GetRealNamedProperty(obj, k).ToLocalChecked()).FromJust();
    }

    k = Nan::New<v8::String>("maxRecursionDepth").ToLocalChecked();
    if (Nan::HasRealNamedProperty(obj, k).FromJust()){
        pfOpts.maxRecursionDepth = Nan::To<int>(Nan::GetRealNamedProperty(obj, k).ToLocalChecked()).FromJust();
    }

    pfOpts.peOpts = peOpts;

    // Execute
    Nan::Callback *callback = new Nan::Callback(Nan::To<v8::Function>(info[2]).ToLocalChecked());
    Nan::AsyncQueueWorker(new ParseFilesWorker(callback, in, pfOpts));
}

//(const fs::path &imagePath, time_t modifiedTime, int thumbSize, bool forceRecreate)
class GetThumbFromUserCacheWorker : public Nan::AsyncWorker {
 public:
  GetThumbFromUserCacheWorker(Nan::Callback *callback, const fs::path &imagePath, time_t modifiedTime, int thumbSize, bool forceRecreate)
    : AsyncWorker(callback, "nan:GetThumbFromUserCacheWorker"),
      imagePath(imagePath), modifiedTime(modifiedTime), thumbSize(thumbSize), forceRecreate(forceRecreate) {}
  ~GetThumbFromUserCacheWorker() {}

  void Execute () {
    try{
        thumbPath = ddb::getThumbFromUserCache(imagePath, modifiedTime, thumbSize, forceRecreate);
    }catch(ddb::AppException &e){
        SetErrorMessage(e.what());
    }
  }

  void HandleOKCallback () {
     Nan::HandleScope scope;

     v8::Local<v8::Value> argv[] = {
         Nan::Null(),
         Nan::New(thumbPath.string()).ToLocalChecked()
     };
     callback->Call(2, argv, async_resource);
   }

 private:
    fs::path imagePath;
    time_t modifiedTime;
    int thumbSize;
    bool forceRecreate;

    fs::path thumbPath;
};


NAN_METHOD(_thumbs_getFromUserCache) {
    if (info.Length() != 4){
        Nan::ThrowError("Invalid number of arguments");
        return;
    }
    if (!info[0]->IsString()){
        Nan::ThrowError("Argument 0 must be a string");
        return;
    }
    if (!info[1]->IsNumber()){
        Nan::ThrowError("Argument 1 must be a number");
        return;
    }
    if (!info[2]->IsObject()){
        Nan::ThrowError("Argument 2 must be an object");
        return;
    }
    if (!info[3]->IsFunction()){
        Nan::ThrowError("Argument 3 must be a function");
        return;
    }

    // Parse first two args
    fs::path imagePath = fs::path(*Nan::Utf8String(info[0].As<v8::String>()));
    time_t modifiedTime = Nan::To<time_t>(info[1].As<v8::Uint32>()).FromJust();

    // Parse options
    v8::Local<v8::String> k = Nan::New<v8::String>("thumbSize").ToLocalChecked();
    v8::Local<v8::Object> obj = info[2].As<v8::Object>();
    int thumbSize = 512;

    if (Nan::HasRealNamedProperty(obj, k).FromJust()){
        thumbSize = Nan::To<time_t>(Nan::GetRealNamedProperty(obj, k).ToLocalChecked()).FromJust();
    }

    bool forceRecreate = false;
    k = Nan::New<v8::String>("forceRecreate").ToLocalChecked();
    if (Nan::HasRealNamedProperty(obj, k).FromJust()){
        forceRecreate = Nan::To<bool>(Nan::GetRealNamedProperty(obj, k).ToLocalChecked()).FromJust();
    }

    // Execute
    Nan::Callback *callback = new Nan::Callback(Nan::To<v8::Function>(info[3]).ToLocalChecked());
    Nan::AsyncQueueWorker(new GetThumbFromUserCacheWorker(callback, imagePath, modifiedTime, thumbSize, forceRecreate));
}

// NAN_METHOD(aBoolean) {
//     info.GetReturnValue().Set(false);
// }

// NAN_METHOD(aNumber) {
//     info.GetReturnValue().Set(1.75);
// }

// NAN_METHOD(anObject) {
//     v8::Local<v8::Object> obj = Nan::New<v8::Object>();
//     Nan::Set(obj, Nan::New("key").ToLocalChecked(), Nan::New("value").ToLocalChecked());
//     info.GetReturnValue().Set(obj);
// }
// NAN_METHOD(callback) {
//     v8::Local<v8::Function> callbackHandle = info[0].As<v8::Function>();
//     Nan::AsyncResource* resource = new Nan::AsyncResource(Nan::New<v8::String>("MyObject:CallCallback").ToLocalChecked());
//     resource->runInAsyncScope(Nan::GetCurrentContext()->Global(), callbackHandle, 0, 0);
// }

// NAN_METHOD(callbackWithParameter) {
//     v8::Local<v8::Function> callbackHandle = info[0].As<v8::Function>();
//     Nan::AsyncResource* resource = new Nan::AsyncResource(Nan::New<v8::String>("MyObject:CallCallbackWithParam").ToLocalChecked());
//     int argc = 1;
//     v8::Local<v8::Value> argv[] = {
//         Nan::New("parameter test").ToLocalChecked()
//     };
//     resource->runInAsyncScope(Nan::GetCurrentContext()->Global(), callbackHandle, argc, argv);
// }

// // Wrapper Impl

// Nan::Persistent<v8::Function> MyObject::constructor;

// NAN_MODULE_INIT(MyObject::Init) {
//   v8::Local<v8::FunctionTemplate> tpl = Nan::New<v8::FunctionTemplate>(New);
//   tpl->SetClassName(Nan::New("MyObject").ToLocalChecked());
//   tpl->InstanceTemplate()->SetInternalFieldCount(1);

//   Nan::SetPrototypeMethod(tpl, "plusOne", PlusOne);

//   constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
//   Nan::Set(target, Nan::New("MyObject").ToLocalChecked(), Nan::GetFunction(tpl).ToLocalChecked());
// }

// MyObject::MyObject(double value) : value_(value) {
// }

// MyObject::~MyObject() {
// }

// NAN_METHOD(MyObject::New) {
//   if (info.IsConstructCall()) {
//     double value = info[0]->IsUndefined() ? 0 : Nan::To<double>(info[0]).FromJust();
//     MyObject *obj = new MyObject(value);
//     obj->Wrap(info.This());
//     info.GetReturnValue().Set(info.This());
//   } else {
//     const int argc = 1;
//     v8::Local<v8::Value> argv[argc] = {info[0]};
//     v8::Local<v8::Function> cons = Nan::New(constructor);
//     info.GetReturnValue().Set(Nan::NewInstance(cons, argc, argv).ToLocalChecked());
//   }
// }

// NAN_METHOD(MyObject::PlusOne) {
//   MyObject* obj = Nan::ObjectWrap::Unwrap<MyObject>(info.This());
//   obj->value_ += 1;
//   info.GetReturnValue().Set(obj->value_);
// }
