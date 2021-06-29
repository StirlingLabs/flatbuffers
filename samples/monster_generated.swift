// automatically generated by the FlatBuffers compiler, do not modify
// swiftlint:disable all
// swiftformat:disable all

import FlatBuffers

public enum MyGame_Sample_Color: Int8, Enum, Verifiable {
  public typealias T = Int8
  public static var byteSize: Int { return MemoryLayout<Int8>.size }
  public var value: Int8 { return self.rawValue }
  case red = 0
  case green = 1
  case blue = 2

  public static var max: MyGame_Sample_Color { return .blue }
  public static var min: MyGame_Sample_Color { return .red }
}

public enum MyGame_Sample_Equipment: UInt8, UnionEnum {
  public typealias T = UInt8

  public init?(value: T) {
    self.init(rawValue: value)
  }

  public static var byteSize: Int { return MemoryLayout<UInt8>.size }
  public var value: UInt8 { return self.rawValue }
  case none_ = 0
  case weapon = 1

  public static var max: MyGame_Sample_Equipment { return .weapon }
  public static var min: MyGame_Sample_Equipment { return .none_ }
}

public struct MyGame_Sample_Vec3: NativeStruct, Verifiable {

  static func validateVersion() { FlatBuffersVersion_2_0_0() }

  private var _x: Float32
  private var _y: Float32
  private var _z: Float32

  public init(x: Float32, y: Float32, z: Float32) {
    _x = x
    _y = y
    _z = z
  }

  public init() {
    _x = 0.0
    _y = 0.0
    _z = 0.0
  }

  public var x: Float32 { _x }
  public var y: Float32 { _y }
  public var z: Float32 { _z }

  public static func verify<T>(_ verifier: inout Verifier, at position: Int, of type: T.Type) throws where T: Verifiable {
    try verifier.inBuffer(position: position, of: MyGame_Sample_Vec3.self)
  }
}

public struct MyGame_Sample_Vec3_Mutable: FlatBufferObject {

  static func validateVersion() { FlatBuffersVersion_2_0_0() }
  public var __buffer: ByteBuffer! { return _accessor.bb }
  private var _accessor: Struct

  public init(_ bb: ByteBuffer, o: Int32) { _accessor = Struct(bb: bb, position: o) }

  public var x: Float32 { return _accessor.readBuffer(of: Float32.self, at: 0) }
  public var y: Float32 { return _accessor.readBuffer(of: Float32.self, at: 4) }
  public var z: Float32 { return _accessor.readBuffer(of: Float32.self, at: 8) }
}

public struct MyGame_Sample_Monster: FlatBufferObject, Verifiable {

  static func validateVersion() { FlatBuffersVersion_2_0_0() }
  public var __buffer: ByteBuffer! { return _accessor.bb }
  private var _accessor: Table

  public static func getRootAsMonster(bb: ByteBuffer) -> MyGame_Sample_Monster { return MyGame_Sample_Monster(Table(bb: bb, position: Int32(bb.read(def: UOffset.self, position: bb.reader)) + Int32(bb.reader))) }

  private init(_ t: Table) { _accessor = t }
  public init(_ bb: ByteBuffer, o: Int32) { _accessor = Table(bb: bb, position: o) }

  private enum VTOFFSET: VOffset {
    case pos = 4
    case mana = 6
    case hp = 8
    case name = 10
    case inventory = 14
    case color = 16
    case weapons = 18
    case equippedType = 20
    case equipped = 22
    case path = 24
    var v: Int32 { Int32(self.rawValue) }
    var p: VOffset { self.rawValue }
  }

  public var pos: MyGame_Sample_Vec3? { let o = _accessor.offset(VTOFFSET.pos.v); return o == 0 ? nil : _accessor.readBuffer(of: MyGame_Sample_Vec3.self, at: o) }
  public var mutablePos: MyGame_Sample_Vec3_Mutable? { let o = _accessor.offset(VTOFFSET.pos.v); return o == 0 ? nil : MyGame_Sample_Vec3_Mutable(_accessor.bb, o: o + _accessor.postion) }
  public var mana: Int16 { let o = _accessor.offset(VTOFFSET.mana.v); return o == 0 ? 150 : _accessor.readBuffer(of: Int16.self, at: o) }
  public var hp: Int16 { let o = _accessor.offset(VTOFFSET.hp.v); return o == 0 ? 100 : _accessor.readBuffer(of: Int16.self, at: o) }
  public var name: String? { let o = _accessor.offset(VTOFFSET.name.v); return o == 0 ? nil : _accessor.string(at: o) }
  public var nameSegmentArray: [UInt8]? { return _accessor.getVector(at: VTOFFSET.name.v) }
  public var inventoryCount: Int32 { let o = _accessor.offset(VTOFFSET.inventory.v); return o == 0 ? 0 : _accessor.vector(count: o) }
  public func inventory(at index: Int32) -> UInt8 { let o = _accessor.offset(VTOFFSET.inventory.v); return o == 0 ? 0 : _accessor.directRead(of: UInt8.self, offset: _accessor.vector(at: o) + index * 1) }
  public var inventory: [UInt8] { return _accessor.getVector(at: VTOFFSET.inventory.v) ?? [] }
  public var color: MyGame_Sample_Color { let o = _accessor.offset(VTOFFSET.color.v); return o == 0 ? .blue : MyGame_Sample_Color(rawValue: _accessor.readBuffer(of: Int8.self, at: o)) ?? .blue }
  public var weaponsCount: Int32 { let o = _accessor.offset(VTOFFSET.weapons.v); return o == 0 ? 0 : _accessor.vector(count: o) }
  public func weapons(at index: Int32) -> MyGame_Sample_Weapon? { let o = _accessor.offset(VTOFFSET.weapons.v); return o == 0 ? nil : MyGame_Sample_Weapon(_accessor.bb, o: _accessor.indirect(_accessor.vector(at: o) + index * 8)) }
  public var equippedType: MyGame_Sample_Equipment { let o = _accessor.offset(VTOFFSET.equippedType.v); return o == 0 ? .none_ : MyGame_Sample_Equipment(rawValue: _accessor.readBuffer(of: UInt8.self, at: o)) ?? .none_ }
  public func equipped<T: FlatbuffersInitializable>(type: T.Type) -> T? { let o = _accessor.offset(VTOFFSET.equipped.v); return o == 0 ? nil : _accessor.union(o) }
  public var pathCount: Int32 { let o = _accessor.offset(VTOFFSET.path.v); return o == 0 ? 0 : _accessor.vector(count: o) }
  public func path(at index: Int32) -> MyGame_Sample_Vec3? { let o = _accessor.offset(VTOFFSET.path.v); return o == 0 ? nil : _accessor.directRead(of: MyGame_Sample_Vec3.self, offset: _accessor.vector(at: o) + index * 12) }
  public func mutablePath(at index: Int32) -> MyGame_Sample_Vec3_Mutable? { let o = _accessor.offset(VTOFFSET.path.v); return o == 0 ? nil : MyGame_Sample_Vec3_Mutable(_accessor.bb, o: _accessor.vector(at: o) + index * 12) }
  public static func startMonster(_ fbb: inout FlatBufferBuilder) -> UOffset { fbb.startTable(with: 11) }
  public static func add(pos: MyGame_Sample_Vec3?, _ fbb: inout FlatBufferBuilder) { guard let pos = pos else { return }; fbb.create(struct: pos, position: VTOFFSET.pos.p) }
  public static func add(mana: Int16, _ fbb: inout FlatBufferBuilder) { fbb.add(element: mana, def: 150, at: VTOFFSET.mana.p) }
  public static func add(hp: Int16, _ fbb: inout FlatBufferBuilder) { fbb.add(element: hp, def: 100, at: VTOFFSET.hp.p) }
  public static func add(name: Offset, _ fbb: inout FlatBufferBuilder) { fbb.add(offset: name, at: VTOFFSET.name.p) }
  public static func addVectorOf(inventory: Offset, _ fbb: inout FlatBufferBuilder) { fbb.add(offset: inventory, at: VTOFFSET.inventory.p) }
  public static func add(color: MyGame_Sample_Color, _ fbb: inout FlatBufferBuilder) { fbb.add(element: color.rawValue, def: 2, at: VTOFFSET.color.p) }
  public static func addVectorOf(weapons: Offset, _ fbb: inout FlatBufferBuilder) { fbb.add(offset: weapons, at: VTOFFSET.weapons.p) }
  public static func add(equippedType: MyGame_Sample_Equipment, _ fbb: inout FlatBufferBuilder) { fbb.add(element: equippedType.rawValue, def: 0, at: VTOFFSET.equippedType.p) }
  public static func add(equipped: Offset, _ fbb: inout FlatBufferBuilder) { fbb.add(offset: equipped, at: VTOFFSET.equipped.p) }
  public static func addVectorOf(path: Offset, _ fbb: inout FlatBufferBuilder) { fbb.add(offset: path, at: VTOFFSET.path.p) }
  public static func startVectorOfPath(_ size: Int, in builder: inout FlatBufferBuilder) {
    builder.startVector(size * MemoryLayout<MyGame_Sample_Vec3>.size, elementSize: MemoryLayout<MyGame_Sample_Vec3>.alignment)
  }
  public static func endMonster(_ fbb: inout FlatBufferBuilder, start: UOffset) -> Offset { let end = Offset(offset: fbb.endTable(at: start)); return end }
  public static func createMonster(
    _ fbb: inout FlatBufferBuilder,
    pos: MyGame_Sample_Vec3? = nil,
    mana: Int16 = 150,
    hp: Int16 = 100,
    nameOffset name: Offset = Offset(),
    inventoryVectorOffset inventory: Offset = Offset(),
    color: MyGame_Sample_Color = .blue,
    weaponsVectorOffset weapons: Offset = Offset(),
    equippedType: MyGame_Sample_Equipment = .none_,
    equippedOffset equipped: Offset = Offset(),
    pathVectorOffset path: Offset = Offset()
  ) -> Offset {
    let __start = MyGame_Sample_Monster.startMonster(&fbb)
    MyGame_Sample_Monster.add(pos: pos, &fbb)
    MyGame_Sample_Monster.add(mana: mana, &fbb)
    MyGame_Sample_Monster.add(hp: hp, &fbb)
    MyGame_Sample_Monster.add(name: name, &fbb)
    MyGame_Sample_Monster.addVectorOf(inventory: inventory, &fbb)
    MyGame_Sample_Monster.add(color: color, &fbb)
    MyGame_Sample_Monster.addVectorOf(weapons: weapons, &fbb)
    MyGame_Sample_Monster.add(equippedType: equippedType, &fbb)
    MyGame_Sample_Monster.add(equipped: equipped, &fbb)
    MyGame_Sample_Monster.addVectorOf(path: path, &fbb)
    return MyGame_Sample_Monster.endMonster(&fbb, start: __start)
  }

  public static func verify<T>(_ verifier: inout Verifier, at position: Int, of type: T.Type) throws where T: Verifiable {
    var _v = try verifier.visitTable(at: position)
    try _v.visit(field: VTOFFSET.pos.p, fieldName: "pos", required: false, type: MyGame_Sample_Vec3.self)
    try _v.visit(field: VTOFFSET.mana.p, fieldName: "mana", required: false, type: Int16.self)
    try _v.visit(field: VTOFFSET.hp.p, fieldName: "hp", required: false, type: Int16.self)
    try _v.visit(field: VTOFFSET.name.p, fieldName: "name", required: false, type: ForwardOffset<String>.self)
    try _v.visit(field: VTOFFSET.inventory.p, fieldName: "inventory", required: false, type: ForwardOffset<Vector<UInt8, UInt8>>.self)
    try _v.visit(field: VTOFFSET.color.p, fieldName: "color", required: false, type: MyGame_Sample_Color.self)
    try _v.visit(field: VTOFFSET.weapons.p, fieldName: "weapons", required: false, type: ForwardOffset<Vector<ForwardOffset<MyGame_Sample_Weapon>, MyGame_Sample_Weapon>>.self)
    try _v.visit(unionKey: VTOFFSET.equippedType.p, unionField: VTOFFSET.equipped.p, unionKeyName: "equippedType", fieldName: "equipped", required: false, completion: { (verifier, key: MyGame_Sample_Equipment, pos) in
      switch key {
      case .none_:
        break // NOTE - SWIFT doesnt support none
      case .weapon:
        try ForwardOffset<MyGame_Sample_Weapon>.verify(&verifier, at: pos, of: MyGame_Sample_Weapon.self)
      }
    })
    try _v.visit(field: VTOFFSET.path.p, fieldName: "path", required: false, type: ForwardOffset<Vector<MyGame_Sample_Vec3, MyGame_Sample_Vec3>>.self)
    _v.finish()
  }
}

public struct MyGame_Sample_Weapon: FlatBufferObject, Verifiable {

  static func validateVersion() { FlatBuffersVersion_2_0_0() }
  public var __buffer: ByteBuffer! { return _accessor.bb }
  private var _accessor: Table

  public static func getRootAsWeapon(bb: ByteBuffer) -> MyGame_Sample_Weapon { return MyGame_Sample_Weapon(Table(bb: bb, position: Int32(bb.read(def: UOffset.self, position: bb.reader)) + Int32(bb.reader))) }

  private init(_ t: Table) { _accessor = t }
  public init(_ bb: ByteBuffer, o: Int32) { _accessor = Table(bb: bb, position: o) }

  private enum VTOFFSET: VOffset {
    case name = 4
    case damage = 6
    var v: Int32 { Int32(self.rawValue) }
    var p: VOffset { self.rawValue }
  }

  public var name: String? { let o = _accessor.offset(VTOFFSET.name.v); return o == 0 ? nil : _accessor.string(at: o) }
  public var nameSegmentArray: [UInt8]? { return _accessor.getVector(at: VTOFFSET.name.v) }
  public var damage: Int16 { let o = _accessor.offset(VTOFFSET.damage.v); return o == 0 ? 0 : _accessor.readBuffer(of: Int16.self, at: o) }
  public static func startWeapon(_ fbb: inout FlatBufferBuilder) -> UOffset { fbb.startTable(with: 2) }
  public static func add(name: Offset, _ fbb: inout FlatBufferBuilder) { fbb.add(offset: name, at: VTOFFSET.name.p) }
  public static func add(damage: Int16, _ fbb: inout FlatBufferBuilder) { fbb.add(element: damage, def: 0, at: VTOFFSET.damage.p) }
  public static func endWeapon(_ fbb: inout FlatBufferBuilder, start: UOffset) -> Offset { let end = Offset(offset: fbb.endTable(at: start)); return end }
  public static func createWeapon(
    _ fbb: inout FlatBufferBuilder,
    nameOffset name: Offset = Offset(),
    damage: Int16 = 0
  ) -> Offset {
    let __start = MyGame_Sample_Weapon.startWeapon(&fbb)
    MyGame_Sample_Weapon.add(name: name, &fbb)
    MyGame_Sample_Weapon.add(damage: damage, &fbb)
    return MyGame_Sample_Weapon.endWeapon(&fbb, start: __start)
  }

  public static func verify<T>(_ verifier: inout Verifier, at position: Int, of type: T.Type) throws where T: Verifiable {
    var _v = try verifier.visitTable(at: position)
    try _v.visit(field: VTOFFSET.name.p, fieldName: "name", required: false, type: ForwardOffset<String>.self)
    try _v.visit(field: VTOFFSET.damage.p, fieldName: "damage", required: false, type: Int16.self)
    _v.finish()
  }
}

