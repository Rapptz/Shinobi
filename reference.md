## Documentation

As you might have already noticed, the Shinobi file is actually just a JSON file. The Shinobi file has a following structure,
here in now called "objects". Every object in the Shinobi file specifies its behaviour. You can find basic example usage of 
how a JSON file is in [Wikipedia](http://en.wikipedia.org/wiki/JSON). Just like JSON files, properties or values in shinobi 
have a type as well. These will be specified in the documentation below. The following are valid types to shinobi:

|Type   |Examples                  |
|:-----:|:------------------------:|
|String |"hello"                   |
|Number |1010, -10.0, 0xDE         |
|Boolean|true, false               |
|Array  |[1, "hello", 1010, true]  |


## Table of Contents

- [project](#project)

## Objects

### project

The `project` object has two possible values. 

|Value   |Type   | Effect                                                                |
|:------:|:-----:|:---------------------------------------------------------------------:|
|name    |String |Specifies the final executable name when the project type is software. |
|type    |String |Specifies how to parse the Shinobi file.                               |


The `name` value is ignored if the `type` is library but would be useful to keep for identification purposes. While `name` 
can be specified as anything, `type` only has two recognisable values. `software` and `library`. `software` would
make the final result of parsing an executable while `library` would create library files (.lib on Windows, .a on Linux and OS X).

While you don't have to specify the `name` value for your project, as it would default to "untitled", you are **required** to specify
a `type` value so shinobi knows how to parse your file.

Example:

```js
{
    "project": {
        "name": "project_name",
        "type": "software"
    }
}
```


