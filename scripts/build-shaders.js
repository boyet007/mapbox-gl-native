#!/usr/bin/env node

var path = require('path');
var fs = require('fs');
var mkdirp = require('mkdirp');

var vertex_file = process.argv[2];
var fragment_file = process.argv[3];
var output_file = process.argv[4];

if (!vertex_file || !fragment_file || !output_file) {
    console.warn('No input or output file given.');
    console.warn('Usage: %s input.vertex.glsl input.fragment.glsl output.hpp', path.basename(process.argv[1]));
    process.exit(1);
}

var components = path.basename(vertex_file).split('.');
var shader_name = components[0];

function applyPragmas(source, pragmas) {
    return source.replace(/#pragma mapbox: ([\w]+) ([\w]+) ([\w]+) ([\w]+)/g, (match, operation, precision, type, name) => {
        return pragmas[operation]
            .replace(/\{type\}/g, type)
            .replace(/\{precision\}/g, precision)
            .replace(/\{name\}/g, name);
    });
}

function vertexSource() {
    return '\n' + applyPragmas(fs.readFileSync(vertex_file, 'utf8'), {
        define: "attribute {precision} {type} a_{name};\nvarying {precision} {type} {name};",
        initialize: "{name} = a_{name};"
    });
}

function fragmentSource() {
    return '\n' + applyPragmas(fs.readFileSync(fragment_file, 'utf8'), {
        define: "varying {precision} {type} {name};",
        initialize: ""
    });
}

var content = "#pragma once\n" +
"\n" +
"// NOTE: DO NOT CHANGE THIS FILE. IT IS AUTOMATICALLY GENERATED.\n" +
"\n" +
"#include <mbgl/gl/gl.hpp>\n" +
"\n" +
"namespace mbgl {\n" +
"namespace shaders {\n" +
"\n" +
"class " + shader_name + " {\n" +
"public:\n" +
"    static constexpr const char* name = \"" + shader_name + "\";" +
"    static constexpr const char* vertexSource = R\"MBGL_SHADER(" + vertexSource() + ")MBGL_SHADER\";\n" +
"    static constexpr const char* fragmentSource = R\"MBGL_SHADER(" + fragmentSource() + ")MBGL_SHADER\";\n" +
"};\n" +
"\n" +
"} // namespace shaders\n" +
"} // namespace mbgl\n";

mkdirp.sync(path.dirname(output_file));
fs.writeFileSync(output_file, content);
