// automatically generated by the FlatBuffers compiler, do not modify

/**
 * @const
*/
var MyGame = MyGame || {};

/**
 * @const
*/
MyGame.Example = MyGame.Example || {};

/**
 * @const
*/
MyGame.OtherNameSpace = MyGame.OtherNameSpace || {};

/**
 * @enum
 */
MyGame.Example.Color = {
  Red: 1,
  Green: 2,
  Blue: 8
};

/**
 * @enum
 */
MyGame.Example.Any = {
  NONE: 0,
  Monster: 1,
  TestSimpleTableWithEnum: 2
};

/**
 * @constructor
 */
MyGame.Example.Test = function() {
  /**
   * @type {flatbuffers.ByteBuffer}
   */
  this.bb = null;

  /**
   * @type {number}
   */
  this.bb_pos = 0;
};

/**
 * @param {number} i
 * @param {flatbuffers.ByteBuffer} bb
 * @returns {MyGame.Example.Test}
 */
MyGame.Example.Test.prototype.__init = function(i, bb) {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @returns {number}
 */
MyGame.Example.Test.prototype.a = function() {
  return this.bb.readInt16(this.bb_pos);
};

/**
 * @returns {number}
 */
MyGame.Example.Test.prototype.b = function() {
  return this.bb.readInt8(this.bb_pos + 2);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} a
 * @param {number} b
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Test.createTest = function(builder, a, b) {
  builder.prep(2, 4);
  builder.pad(1);
  builder.writeInt8(b);
  builder.writeInt16(a);
  return builder.offset();
};

/**
 * @constructor
 */
MyGame.Example.TestSimpleTableWithEnum = function() {
  /**
   * @type {flatbuffers.ByteBuffer}
   */
  this.bb = null;

  /**
   * @type {number}
   */
  this.bb_pos = 0;
};

/**
 * @param {number} i
 * @param {flatbuffers.ByteBuffer} bb
 * @returns {MyGame.Example.TestSimpleTableWithEnum}
 */
MyGame.Example.TestSimpleTableWithEnum.prototype.__init = function(i, bb) {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @param {flatbuffers.ByteBuffer} bb
 * @param {MyGame.Example.TestSimpleTableWithEnum=} obj
 * @returns {MyGame.Example.TestSimpleTableWithEnum}
 */
MyGame.Example.TestSimpleTableWithEnum.getRootAsTestSimpleTableWithEnum = function(bb, obj) {
  return (obj || new MyGame.Example.TestSimpleTableWithEnum).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @returns {MyGame.Example.Color}
 */
MyGame.Example.TestSimpleTableWithEnum.prototype.color = function() {
  var offset = this.bb.__offset(this.bb_pos, 4);
  return offset ? /** @type {MyGame.Example.Color} */ (this.bb.readInt8(this.bb_pos + offset)) : MyGame.Example.Color.Green;
};

/**
 * @param {flatbuffers.Builder} builder
 */
MyGame.Example.TestSimpleTableWithEnum.startTestSimpleTableWithEnum = function(builder) {
  builder.startObject(1);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {MyGame.Example.Color} color
 */
MyGame.Example.TestSimpleTableWithEnum.addColor = function(builder, color) {
  builder.addFieldInt8(0, color, MyGame.Example.Color.Green);
};

/**
 * @param {flatbuffers.Builder} builder
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.TestSimpleTableWithEnum.endTestSimpleTableWithEnum = function(builder) {
  var offset = builder.endObject();
  return offset;
};

/**
 * @constructor
 */
MyGame.Example.Vec3 = function() {
  /**
   * @type {flatbuffers.ByteBuffer}
   */
  this.bb = null;

  /**
   * @type {number}
   */
  this.bb_pos = 0;
};

/**
 * @param {number} i
 * @param {flatbuffers.ByteBuffer} bb
 * @returns {MyGame.Example.Vec3}
 */
MyGame.Example.Vec3.prototype.__init = function(i, bb) {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @returns {number}
 */
MyGame.Example.Vec3.prototype.x = function() {
  return this.bb.readFloat32(this.bb_pos);
};

/**
 * @returns {number}
 */
MyGame.Example.Vec3.prototype.y = function() {
  return this.bb.readFloat32(this.bb_pos + 4);
};

/**
 * @returns {number}
 */
MyGame.Example.Vec3.prototype.z = function() {
  return this.bb.readFloat32(this.bb_pos + 8);
};

/**
 * @returns {number}
 */
MyGame.Example.Vec3.prototype.test1 = function() {
  return this.bb.readFloat64(this.bb_pos + 16);
};

/**
 * @returns {MyGame.Example.Color}
 */
MyGame.Example.Vec3.prototype.test2 = function() {
  return /** @type {MyGame.Example.Color} */ (this.bb.readInt8(this.bb_pos + 24));
};

/**
 * @param {MyGame.Example.Test=} obj
 * @returns {MyGame.Example.Test}
 */
MyGame.Example.Vec3.prototype.test3 = function(obj) {
  return (obj || new MyGame.Example.Test).__init(this.bb_pos + 26, this.bb);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} x
 * @param {number} y
 * @param {number} z
 * @param {number} test1
 * @param {MyGame.Example.Color} test2
 * @param {number} test3_a
 * @param {number} test3_b
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Vec3.createVec3 = function(builder, x, y, z, test1, test2, test3_a, test3_b) {
  builder.prep(16, 32);
  builder.pad(2);
  builder.prep(2, 4);
  builder.pad(1);
  builder.writeInt8(test3_b);
  builder.writeInt16(test3_a);
  builder.pad(1);
  builder.writeInt8(test2);
  builder.writeFloat64(test1);
  builder.pad(4);
  builder.writeFloat32(z);
  builder.writeFloat32(y);
  builder.writeFloat32(x);
  return builder.offset();
};

/**
 * @constructor
 */
MyGame.Example.Stat = function() {
  /**
   * @type {flatbuffers.ByteBuffer}
   */
  this.bb = null;

  /**
   * @type {number}
   */
  this.bb_pos = 0;
};

/**
 * @param {number} i
 * @param {flatbuffers.ByteBuffer} bb
 * @returns {MyGame.Example.Stat}
 */
MyGame.Example.Stat.prototype.__init = function(i, bb) {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @param {flatbuffers.ByteBuffer} bb
 * @param {MyGame.Example.Stat=} obj
 * @returns {MyGame.Example.Stat}
 */
MyGame.Example.Stat.getRootAsStat = function(bb, obj) {
  return (obj || new MyGame.Example.Stat).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @param {flatbuffers.Encoding=} optionalEncoding
 * @returns {string|Uint8Array}
 */
MyGame.Example.Stat.prototype.id = function(optionalEncoding) {
  var offset = this.bb.__offset(this.bb_pos, 4);
  return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
};

/**
 * @returns {flatbuffers.Long}
 */
MyGame.Example.Stat.prototype.val = function() {
  var offset = this.bb.__offset(this.bb_pos, 6);
  return offset ? this.bb.readInt64(this.bb_pos + offset) : flatbuffers.Long.ZERO;
};

/**
 * @returns {number}
 */
MyGame.Example.Stat.prototype.count = function() {
  var offset = this.bb.__offset(this.bb_pos, 8);
  return offset ? this.bb.readUint16(this.bb_pos + offset) : 0;
};

/**
 * @param {flatbuffers.Builder} builder
 */
MyGame.Example.Stat.startStat = function(builder) {
  builder.startObject(3);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} idOffset
 */
MyGame.Example.Stat.addId = function(builder, idOffset) {
  builder.addFieldOffset(0, idOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Long} val
 */
MyGame.Example.Stat.addVal = function(builder, val) {
  builder.addFieldInt64(1, val, flatbuffers.Long.ZERO);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} count
 */
MyGame.Example.Stat.addCount = function(builder, count) {
  builder.addFieldInt16(2, count, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Stat.endStat = function(builder) {
  var offset = builder.endObject();
  return offset;
};

/**
 * an example documentation comment: monster object
 *
 * @constructor
 */
MyGame.Example.Monster = function() {
  /**
   * @type {flatbuffers.ByteBuffer}
   */
  this.bb = null;

  /**
   * @type {number}
   */
  this.bb_pos = 0;
};

/**
 * @param {number} i
 * @param {flatbuffers.ByteBuffer} bb
 * @returns {MyGame.Example.Monster}
 */
MyGame.Example.Monster.prototype.__init = function(i, bb) {
  this.bb_pos = i;
  this.bb = bb;
  return this;
};

/**
 * @param {flatbuffers.ByteBuffer} bb
 * @param {MyGame.Example.Monster=} obj
 * @returns {MyGame.Example.Monster}
 */
MyGame.Example.Monster.getRootAsMonster = function(bb, obj) {
  return (obj || new MyGame.Example.Monster).__init(bb.readInt32(bb.position()) + bb.position(), bb);
};

/**
 * @param {flatbuffers.ByteBuffer} bb
 * @returns {boolean}
 */
MyGame.Example.Monster.bufferHasIdentifier = function(bb) {
  return bb.__has_identifier('MONS');
};

/**
 * @param {MyGame.Example.Vec3=} obj
 * @returns {MyGame.Example.Vec3}
 */
MyGame.Example.Monster.prototype.pos = function(obj) {
  var offset = this.bb.__offset(this.bb_pos, 4);
  return offset ? (obj || new MyGame.Example.Vec3).__init(this.bb_pos + offset, this.bb) : null;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.mana = function() {
  var offset = this.bb.__offset(this.bb_pos, 6);
  return offset ? this.bb.readInt16(this.bb_pos + offset) : 150;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.hp = function() {
  var offset = this.bb.__offset(this.bb_pos, 8);
  return offset ? this.bb.readInt16(this.bb_pos + offset) : 100;
};

/**
 * @param {flatbuffers.Encoding=} optionalEncoding
 * @returns {string|Uint8Array}
 */
MyGame.Example.Monster.prototype.name = function(optionalEncoding) {
  var offset = this.bb.__offset(this.bb_pos, 10);
  return offset ? this.bb.__string(this.bb_pos + offset, optionalEncoding) : null;
};

/**
 * @param {number} index
 * @returns {number}
 */
MyGame.Example.Monster.prototype.inventory = function(index) {
  var offset = this.bb.__offset(this.bb_pos, 14);
  return offset ? this.bb.readUint8(this.bb.__vector(this.bb_pos + offset) + index) : 0;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.inventoryLength = function() {
  var offset = this.bb.__offset(this.bb_pos, 14);
  return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * @returns {Uint8Array}
 */
MyGame.Example.Monster.prototype.inventoryArray = function() {
  var offset = this.bb.__offset(this.bb_pos, 14);
  return offset ? new Uint8Array(this.bb.bytes().buffer, this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
};

/**
 * @returns {MyGame.Example.Color}
 */
MyGame.Example.Monster.prototype.color = function() {
  var offset = this.bb.__offset(this.bb_pos, 16);
  return offset ? /** @type {MyGame.Example.Color} */ (this.bb.readInt8(this.bb_pos + offset)) : MyGame.Example.Color.Blue;
};

/**
 * @returns {MyGame.Example.Any}
 */
MyGame.Example.Monster.prototype.testType = function() {
  var offset = this.bb.__offset(this.bb_pos, 18);
  return offset ? /** @type {MyGame.Example.Any} */ (this.bb.readUint8(this.bb_pos + offset)) : MyGame.Example.Any.NONE;
};

/**
 * @param {flatbuffers.Table} obj
 * @returns {?flatbuffers.Table}
 */
MyGame.Example.Monster.prototype.test = function(obj) {
  var offset = this.bb.__offset(this.bb_pos, 20);
  return offset ? this.bb.__union(obj, this.bb_pos + offset) : null;
};

/**
 * @param {number} index
 * @param {MyGame.Example.Test=} obj
 * @returns {MyGame.Example.Test}
 */
MyGame.Example.Monster.prototype.test4 = function(index, obj) {
  var offset = this.bb.__offset(this.bb_pos, 22);
  return offset ? (obj || new MyGame.Example.Test).__init(this.bb.__vector(this.bb_pos + offset) + index * 4, this.bb) : null;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.test4Length = function() {
  var offset = this.bb.__offset(this.bb_pos, 22);
  return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * @param {number} index
 * @param {flatbuffers.Encoding=} optionalEncoding
 * @returns {string|Uint8Array}
 */
MyGame.Example.Monster.prototype.testarrayofstring = function(index, optionalEncoding) {
  var offset = this.bb.__offset(this.bb_pos, 24);
  return offset ? this.bb.__string(this.bb.__vector(this.bb_pos + offset) + index * 4, optionalEncoding) : null;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testarrayofstringLength = function() {
  var offset = this.bb.__offset(this.bb_pos, 24);
  return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * an example documentation comment: this will end up in the generated code
 * multiline too
 *
 * @param {number} index
 * @param {MyGame.Example.Monster=} obj
 * @returns {MyGame.Example.Monster}
 */
MyGame.Example.Monster.prototype.testarrayoftables = function(index, obj) {
  var offset = this.bb.__offset(this.bb_pos, 26);
  return offset ? (obj || new MyGame.Example.Monster).__init(this.bb.__indirect(this.bb.__vector(this.bb_pos + offset) + index * 4), this.bb) : null;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testarrayoftablesLength = function() {
  var offset = this.bb.__offset(this.bb_pos, 26);
  return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * @param {MyGame.Example.Monster=} obj
 * @returns {MyGame.Example.Monster}
 */
MyGame.Example.Monster.prototype.enemy = function(obj) {
  var offset = this.bb.__offset(this.bb_pos, 28);
  return offset ? (obj || new MyGame.Example.Monster).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
};

/**
 * @param {number} index
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testnestedflatbuffer = function(index) {
  var offset = this.bb.__offset(this.bb_pos, 30);
  return offset ? this.bb.readUint8(this.bb.__vector(this.bb_pos + offset) + index) : 0;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testnestedflatbufferLength = function() {
  var offset = this.bb.__offset(this.bb_pos, 30);
  return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * @returns {Uint8Array}
 */
MyGame.Example.Monster.prototype.testnestedflatbufferArray = function() {
  var offset = this.bb.__offset(this.bb_pos, 30);
  return offset ? new Uint8Array(this.bb.bytes().buffer, this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
};

/**
 * @param {MyGame.Example.Stat=} obj
 * @returns {MyGame.Example.Stat}
 */
MyGame.Example.Monster.prototype.testempty = function(obj) {
  var offset = this.bb.__offset(this.bb_pos, 32);
  return offset ? (obj || new MyGame.Example.Stat).__init(this.bb.__indirect(this.bb_pos + offset), this.bb) : null;
};

/**
 * @returns {boolean}
 */
MyGame.Example.Monster.prototype.testbool = function() {
  var offset = this.bb.__offset(this.bb_pos, 34);
  return offset ? !!this.bb.readInt8(this.bb_pos + offset) : false;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testhashs32Fnv1 = function() {
  var offset = this.bb.__offset(this.bb_pos, 36);
  return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testhashu32Fnv1 = function() {
  var offset = this.bb.__offset(this.bb_pos, 38);
  return offset ? this.bb.readUint32(this.bb_pos + offset) : 0;
};

/**
 * @returns {flatbuffers.Long}
 */
MyGame.Example.Monster.prototype.testhashs64Fnv1 = function() {
  var offset = this.bb.__offset(this.bb_pos, 40);
  return offset ? this.bb.readInt64(this.bb_pos + offset) : flatbuffers.Long.ZERO;
};

/**
 * @returns {flatbuffers.Long}
 */
MyGame.Example.Monster.prototype.testhashu64Fnv1 = function() {
  var offset = this.bb.__offset(this.bb_pos, 42);
  return offset ? this.bb.readUint64(this.bb_pos + offset) : flatbuffers.Long.ZERO;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testhashs32Fnv1a = function() {
  var offset = this.bb.__offset(this.bb_pos, 44);
  return offset ? this.bb.readInt32(this.bb_pos + offset) : 0;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testhashu32Fnv1a = function() {
  var offset = this.bb.__offset(this.bb_pos, 46);
  return offset ? this.bb.readUint32(this.bb_pos + offset) : 0;
};

/**
 * @returns {flatbuffers.Long}
 */
MyGame.Example.Monster.prototype.testhashs64Fnv1a = function() {
  var offset = this.bb.__offset(this.bb_pos, 48);
  return offset ? this.bb.readInt64(this.bb_pos + offset) : flatbuffers.Long.ZERO;
};

/**
 * @returns {flatbuffers.Long}
 */
MyGame.Example.Monster.prototype.testhashu64Fnv1a = function() {
  var offset = this.bb.__offset(this.bb_pos, 50);
  return offset ? this.bb.readUint64(this.bb_pos + offset) : flatbuffers.Long.ZERO;
};

/**
 * @param {number} index
 * @returns {boolean}
 */
MyGame.Example.Monster.prototype.testarrayofbools = function(index) {
  var offset = this.bb.__offset(this.bb_pos, 52);
  return offset ? !!this.bb.readInt8(this.bb.__vector(this.bb_pos + offset) + index) : false;
};

/**
 * @returns {number}
 */
MyGame.Example.Monster.prototype.testarrayofboolsLength = function() {
  var offset = this.bb.__offset(this.bb_pos, 52);
  return offset ? this.bb.__vector_len(this.bb_pos + offset) : 0;
};

/**
 * @returns {Int8Array}
 */
MyGame.Example.Monster.prototype.testarrayofboolsArray = function() {
  var offset = this.bb.__offset(this.bb_pos, 52);
  return offset ? new Int8Array(this.bb.bytes().buffer, this.bb.__vector(this.bb_pos + offset), this.bb.__vector_len(this.bb_pos + offset)) : null;
};

/**
 * @param {flatbuffers.Builder} builder
 */
MyGame.Example.Monster.startMonster = function(builder) {
  builder.startObject(25);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} posOffset
 */
MyGame.Example.Monster.addPos = function(builder, posOffset) {
  builder.addFieldStruct(0, posOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} mana
 */
MyGame.Example.Monster.addMana = function(builder, mana) {
  builder.addFieldInt16(1, mana, 150);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} hp
 */
MyGame.Example.Monster.addHp = function(builder, hp) {
  builder.addFieldInt16(2, hp, 100);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} nameOffset
 */
MyGame.Example.Monster.addName = function(builder, nameOffset) {
  builder.addFieldOffset(3, nameOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} inventoryOffset
 */
MyGame.Example.Monster.addInventory = function(builder, inventoryOffset) {
  builder.addFieldOffset(5, inventoryOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {Array.<number>} data
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Monster.createInventoryVector = function(builder, data) {
  builder.startVector(1, data.length, 1);
  for (var i = data.length - 1; i >= 0; i--) {
    builder.addInt8(data[i]);
  }
  return builder.endVector();
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} numElems
 */
MyGame.Example.Monster.startInventoryVector = function(builder, numElems) {
  builder.startVector(1, numElems, 1);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {MyGame.Example.Color} color
 */
MyGame.Example.Monster.addColor = function(builder, color) {
  builder.addFieldInt8(6, color, MyGame.Example.Color.Blue);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {MyGame.Example.Any} testType
 */
MyGame.Example.Monster.addTestType = function(builder, testType) {
  builder.addFieldInt8(7, testType, MyGame.Example.Any.NONE);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} testOffset
 */
MyGame.Example.Monster.addTest = function(builder, testOffset) {
  builder.addFieldOffset(8, testOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} test4Offset
 */
MyGame.Example.Monster.addTest4 = function(builder, test4Offset) {
  builder.addFieldOffset(9, test4Offset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} numElems
 */
MyGame.Example.Monster.startTest4Vector = function(builder, numElems) {
  builder.startVector(4, numElems, 2);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} testarrayofstringOffset
 */
MyGame.Example.Monster.addTestarrayofstring = function(builder, testarrayofstringOffset) {
  builder.addFieldOffset(10, testarrayofstringOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {Array.<flatbuffers.Offset>} data
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Monster.createTestarrayofstringVector = function(builder, data) {
  builder.startVector(4, data.length, 4);
  for (var i = data.length - 1; i >= 0; i--) {
    builder.addOffset(data[i]);
  }
  return builder.endVector();
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} numElems
 */
MyGame.Example.Monster.startTestarrayofstringVector = function(builder, numElems) {
  builder.startVector(4, numElems, 4);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} testarrayoftablesOffset
 */
MyGame.Example.Monster.addTestarrayoftables = function(builder, testarrayoftablesOffset) {
  builder.addFieldOffset(11, testarrayoftablesOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {Array.<flatbuffers.Offset>} data
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Monster.createTestarrayoftablesVector = function(builder, data) {
  builder.startVector(4, data.length, 4);
  for (var i = data.length - 1; i >= 0; i--) {
    builder.addOffset(data[i]);
  }
  return builder.endVector();
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} numElems
 */
MyGame.Example.Monster.startTestarrayoftablesVector = function(builder, numElems) {
  builder.startVector(4, numElems, 4);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} enemyOffset
 */
MyGame.Example.Monster.addEnemy = function(builder, enemyOffset) {
  builder.addFieldOffset(12, enemyOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} testnestedflatbufferOffset
 */
MyGame.Example.Monster.addTestnestedflatbuffer = function(builder, testnestedflatbufferOffset) {
  builder.addFieldOffset(13, testnestedflatbufferOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {Array.<number>} data
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Monster.createTestnestedflatbufferVector = function(builder, data) {
  builder.startVector(1, data.length, 1);
  for (var i = data.length - 1; i >= 0; i--) {
    builder.addInt8(data[i]);
  }
  return builder.endVector();
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} numElems
 */
MyGame.Example.Monster.startTestnestedflatbufferVector = function(builder, numElems) {
  builder.startVector(1, numElems, 1);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} testemptyOffset
 */
MyGame.Example.Monster.addTestempty = function(builder, testemptyOffset) {
  builder.addFieldOffset(14, testemptyOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {boolean} testbool
 */
MyGame.Example.Monster.addTestbool = function(builder, testbool) {
  builder.addFieldInt8(15, +testbool, +false);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} testhashs32Fnv1
 */
MyGame.Example.Monster.addTesthashs32Fnv1 = function(builder, testhashs32Fnv1) {
  builder.addFieldInt32(16, testhashs32Fnv1, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} testhashu32Fnv1
 */
MyGame.Example.Monster.addTesthashu32Fnv1 = function(builder, testhashu32Fnv1) {
  builder.addFieldInt32(17, testhashu32Fnv1, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Long} testhashs64Fnv1
 */
MyGame.Example.Monster.addTesthashs64Fnv1 = function(builder, testhashs64Fnv1) {
  builder.addFieldInt64(18, testhashs64Fnv1, flatbuffers.Long.ZERO);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Long} testhashu64Fnv1
 */
MyGame.Example.Monster.addTesthashu64Fnv1 = function(builder, testhashu64Fnv1) {
  builder.addFieldInt64(19, testhashu64Fnv1, flatbuffers.Long.ZERO);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} testhashs32Fnv1a
 */
MyGame.Example.Monster.addTesthashs32Fnv1a = function(builder, testhashs32Fnv1a) {
  builder.addFieldInt32(20, testhashs32Fnv1a, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} testhashu32Fnv1a
 */
MyGame.Example.Monster.addTesthashu32Fnv1a = function(builder, testhashu32Fnv1a) {
  builder.addFieldInt32(21, testhashu32Fnv1a, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Long} testhashs64Fnv1a
 */
MyGame.Example.Monster.addTesthashs64Fnv1a = function(builder, testhashs64Fnv1a) {
  builder.addFieldInt64(22, testhashs64Fnv1a, flatbuffers.Long.ZERO);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Long} testhashu64Fnv1a
 */
MyGame.Example.Monster.addTesthashu64Fnv1a = function(builder, testhashu64Fnv1a) {
  builder.addFieldInt64(23, testhashu64Fnv1a, flatbuffers.Long.ZERO);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} testarrayofboolsOffset
 */
MyGame.Example.Monster.addTestarrayofbools = function(builder, testarrayofboolsOffset) {
  builder.addFieldOffset(24, testarrayofboolsOffset, 0);
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {Array.<boolean>} data
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Monster.createTestarrayofboolsVector = function(builder, data) {
  builder.startVector(1, data.length, 1);
  for (var i = data.length - 1; i >= 0; i--) {
    builder.addInt8(+data[i]);
  }
  return builder.endVector();
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {number} numElems
 */
MyGame.Example.Monster.startTestarrayofboolsVector = function(builder, numElems) {
  builder.startVector(1, numElems, 1);
};

/**
 * @param {flatbuffers.Builder} builder
 * @returns {flatbuffers.Offset}
 */
MyGame.Example.Monster.endMonster = function(builder) {
  var offset = builder.endObject();
  builder.requiredField(offset, 10); // name
  return offset;
};

/**
 * @param {flatbuffers.Builder} builder
 * @param {flatbuffers.Offset} offset
 */
MyGame.Example.Monster.finishMonsterBuffer = function(builder, offset) {
  builder.finish(offset, 'MONS');
};

// Exports for Node.js and RequireJS
this.MyGame = MyGame;
