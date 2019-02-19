var addon = require('bindings')('cut');

module.exports.hello = () => addon.hello();
module.exports.loadMesh = (filePath) => addon.loadMesh(filePath);
module.exports.visitGraph = (startFace) => addon.visitGraph(startFace);
module.exports.extractBoundary = () => addon.extractBoundary();