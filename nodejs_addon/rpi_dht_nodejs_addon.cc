// Copyright (c) 2016 react9
// Author: react9

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <node.h>
#include <time.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <uv.h>
#include <v8.h>
#include "../source/Raspberry_Pi/pi_dht_read.h"

int pi_dht_read(int sensor, int pin, float* humidity, float* temperature);

// ========================================================================== //

using v8::Function;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Persistent;
using v8::Handle;
using v8::Int32;
using v8::Number;
using v8::Array;

struct Work
{
    uv_work_t request;                  
    Persistent<Function> callback;      

	int sensor_type;
	int gpio_pin;
	
	int result;
	float temperature;
	float humidity;
	
};

static void WorkAsync(uv_work_t *req) {
	
	Work *work = static_cast<Work *>(req->data);
	
    float humidity = 0.0, temperature = 0.0;
	int result = pi_dht_read(work->sensor_type, work->gpio_pin, &humidity, &temperature);	

	work->result = result;
	work->temperature = temperature;
	work->humidity = humidity;	
}

static void WorkAsyncComplete(uv_work_t *req,int status)
{
	Isolate * isolate = Isolate::GetCurrent();
    v8::HandleScope handleScope(isolate); // Required for Node 4.x	
	
    Work *work = static_cast<Work *>(req->data);

	Local<Object> obj = Object::New(isolate);
	
	obj->Set(String::NewFromUtf8(isolate, "temperature"), Number::New(isolate, work->temperature));
	obj->Set(String::NewFromUtf8(isolate, "humidity"), Number::New(isolate, work->humidity));
	obj->Set(String::NewFromUtf8(isolate, "result"), Number::New(isolate, work->result));  
  
    Handle<Value> argv[] = { obj };

    Local<Function>::New(isolate, work->callback)->Call(isolate->GetCurrentContext()->Global(), 1, argv);

    work->callback.Reset();

    delete work;
}

void GetDHT(const v8::FunctionCallbackInfo<v8::Value>&args) {	
	
	Isolate* isolate = args.GetIsolate();

    Work *work = new Work();
	work->request.data = work;
	
	work->sensor_type = args[0]->IntegerValue();
	work->gpio_pin = args[1]->IntegerValue();

	Local<Function> callback = Local<Function>::Cast(args[2]);
	work->callback.Reset(isolate, callback);
	 
    // queue the async function to the event loop
    // the uv default loop is the node.js event loop
    uv_queue_work(uv_default_loop(),&work->request,WorkAsync,WorkAsyncComplete);

    args.GetReturnValue().Set(Undefined(isolate));
}

void GetDHTSync(const FunctionCallbackInfo<Value>& args) {	
    Isolate* isolate = args.GetIsolate();
	Local<Object> obj = Object::New(isolate);
	
	float humidity = 0.0, temperature = 0.0;	
	int result;
    
	result = pi_dht_read(args[0]->IntegerValue(), args[1]->IntegerValue(), &humidity, &temperature);	
		
	obj->Set(String::NewFromUtf8(isolate, "temperature"), Number::New(isolate, temperature));
	obj->Set(String::NewFromUtf8(isolate, "humidity"), Number::New(isolate, humidity));
	obj->Set(String::NewFromUtf8(isolate, "result"), Number::New(isolate, result));
	
	args.GetReturnValue().Set(obj);	
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "getDHTSync", GetDHTSync);
  NODE_SET_METHOD(exports, "getDHT", GetDHT);
}

NODE_MODULE(addon, init)

