# JSON++

[![Build Status](https://travis-ci.org/hjiang/jsonxx.svg?branch=master)](https://travis-ci.org/hjiang/jsonxx)

## Introduction

JSON++ is a light-weight JSON parser, writer and reader written in C++.
JSON++ can also convert JSON documents into lossless XML documents.

## Contributors

* http://github.com/hjiang
* http://github.com/elanthis
* http://github.com/r-lyeh

If you've made substantial contribution, please add your link here. 

## Why another JSON parser?

Perhaps because web service clients are usually written in dynamic languages these days, none of the existing C++ JSON parsers fitted my needs very well, so I wrote one that I used in another project. My goals for JSON++ were:

* Efficient in both memory and speed.
* No third party dependencies. JSON++ only depends on the standard C++ library.
* Cross platform.
* Robust.
* Small and convenient API. Most of the time, you only need to call one function and two function templates.
* Easy to integrate. JSON++ only has one source file and one header file. Just compile the source file and link with your program.
* Able to construct documents dynamically.
* JSON writer: write documents in JSON format.

Other contributors have sinced added more functionalities:

* XML writer: convert documents to JSONx format. See http://goo.gl/I3cxs for details.
* XML writer: convert documents to JXML format. See https://github.com/r-lyeh/JXML for details.
* XML writer: convert documents to JXMLex format. See https://github.com/r-lyeh/JXMLex for details.
* XML writer: convert documents to tagged XML format. See https://github.com/hjiang/jsonxx/issues/12 for details.

## Compiler version

You need a modern C++ compiler. For older compilers, please try [legacy branch](https://github.com/hjiang/jsonxx/tree/legacy).

## Configuration

### Strict/permissive parsing

JSONxx can parse JSON documents both in strict or permissive mode.

When `jsonxx::Settings::Parser` is set to `Strict`, JSONxx parser will accept:
* Fully conformant JSON documents *only*.

When `jsonxx::Settings::Parser` is set to `Permissive`, JSONxx parser will accept:
* Fully conformant JSON documents
* Ending commas in arrays and objects: `{ "array": [0,1,2,], }`
* Single quoted strings: `['hello', "world"]`
* C++ style comments: `{ "width": 320, "height": 240 } //Picture details`

Default value is `Permissive`.

When `jsonxx::Settings::UnquotedKeys` is set to `Enabled`, JSONxx parser will accept:
* Unquoted keys: `{name: "world"}`

Default value is `Disabled`.

### Assertions

JSONxx uses internally `JSONXX_ASSERT(...)` macro that works both in debug and release mode. Set `jsonxx::Settings::Assertions` value to `Disabled` to disable assertions.

Default value is `Enabled`.

## Usage

The following snippets are from one of the unit tests. They are quite self-descriptive.

~~~C++
using namespace std;
using namespace jsonxx;

string teststr(
        "{"
        "  \"foo\" : 1,"
        "  \"bar\" : false,"
        "  \"person\" : {\"name\" : \"GWB\", \"age\" : 60,},"
        "  \"data\": [\"abcd\", 42],"
        "}"
);

// Parse string or stream
Object o;
assert(o.parse(teststr));

// Validation. Checking for JSON types and values as well
assert(1 == o.get<Number>("foo"));
assert(o.has<Boolean>("bar"));
assert(o.has<Object>("person"));
assert(o.get<Object>("person").has<Number>("age"));
assert(!o.get<Object>("person").has<Boolean>("old"));
assert(o.get<Object>("person").get<Boolean>("old", false));
assert(o.has<Array>("data"));
assert(o.get<Array>("data").get<Number>(1) == 42);
assert(o.get<Array>("data").get<String>(0) == "abcd");
assert(o.get<Array>("data").get<String>(2, "hello") == "hello");
assert(!o.has<Number>("data"));
cout << o.json() << endl;                     // JSON output
cout << o.xml(JSONx) << endl;                 // JSON to XML conversion (JSONx subtype)
cout << o.xml(JXML) << endl;                  // JSON to XML conversion (JXML subtype)
cout << o.xml(JXMLex) << endl;                // JSON to XML conversion (JXMLex subtype)
~~~

~~~C++
// Generate JSON document dynamically
using namespace std;
using namespace jsonxx;
Array a;
a << 123;
a << "hello world";
a << 3.1415;
a << 99.95f;
a << 'h';
a << Object("key", "value");
Object o;
o << "key1" << "value";
o << "key2" << 123;
o << "key3" << a;
cout << o.json() << endl;
~~~

## To do

* Custom JSON comments (C style /**/) when permissive parsing is enabled.
