var addon = require('bindings')('cut');

console.log(addon.hello()); // 'world'
var st = addon.loadFile("carved_path_1.obj");
addon.visitMesh(0);
console.log(addon.exportBoundary());