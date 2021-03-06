/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */
var ext = require('../');
var assert = require('assert');

describe('node-ddb extension', function() {
  it('should export a getVersion() method', function() {
    assert.equal(typeof ext.getVersion(), "string");
    assert(ext.getVersion().length > 0);
  });

  it('should export a parseFiles() method', function() {
    assert.equal(typeof ext.parseFiles, "function");
  });

  it('should export a parseFile() method', function() {
    assert.equal(typeof ext.parseFile, "function");
  });

  // TODO: test parseFile

  it('should be able to call parseFiles without hash', async function(){
    const res = await ext.parseFiles(__filename);
    assert.equal(res.length, 1);
    assert.equal(typeof res.hash, "undefined")
  });

  it('should be able to call parseFiles with hash', async function(){
    const res = await ext.parseFiles([__filename, __dirname], {withHash: true});
    assert.equal(res.length, 2);

    // Files have hash calculated
    assert.equal(typeof res[0].hash, "string");
    assert(res[0].hash.length > 0);

    // Directories do not
    assert.equal(typeof res[1].hash, "undefined");
  });

  it('should fail when parseFiles is called on bad files', async function(){
    await assert.rejects(ext.parseFiles("404", {withHash: true}));
  });

  it('should export a thumbs.getFromUserCache() method', function() {
    assert.equal(typeof ext.thumbs.getFromUserCache, "function");
  });

  it('should fail when thumbs.getFromUserCache() is called on bad files', async function(){
    await assert.rejects(ext.thumbs.getFromUserCache("nonexistant.jpg", 0, {thumbSize: 200}));
  });

  // TODO: test good getThumbFromUserCache path
  
});
