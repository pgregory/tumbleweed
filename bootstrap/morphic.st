*
* Common morpho functionality for the user interface
*
Class NodeException Exception
RawClass MetaNode Class MetaObject errorException
Class Node Object parent children name
Class Morph Node bounds color alpha is_visible is_draggable last_time shape
Class Frame Morph
Class World Frame
*
Methods MetaNode 'creation'
  initialize
    errorException <- NodeException new
|
  new
    smalltalk error: 'Cannot create Node without a name'
|
  new: aString
    | newObj |
    newObj <- super new.
    newObj init: aString.
    ^ newObj
]
*
Methods Node 'all'
  init: aString
    children <- Dictionary new.
    parent <- nil.
    name <- aString
|
  errorException
    ^ errorException
|
  name
    ^ name
|
  parent
    ^ parent
|
  setParent: aNode
    (parent isNil) ifFalse: [
      parent removeChild: self
    ].
    parent <- aNode
|
  addChild: aNode  
    children at: (aNode name) put: aNode.
    aNode setParent: self
|
  removeChild: aNode
    ( children includesKey: (aNode name) ) ifFalse: [
      errorException signalWith: ('Child "', (aNode name), '" is not a member')
    ].
    children removeKey: (aNode name)
|
  children
    ^ children
]
*
Methods MetaMorph 'creation'
  new: aString
    | newObj |
    newObj <- super new: aString.
    ^ newObj
]
*
Methods Morph 'initialisation'
  init: aString
    super init: aString.
    bounds <- Rectangle new.
    self setPositionX: 0 Y: 0; setWidth: 0 height: 0
]
*
Methods Morph 'geometry'
  left
    ^ bounds left
|
  right
    ^ bounds right
|
  top
    ^ bounds top
|
  bottom
    ^ bounds bottom
|
  setPositionX: xpos Y: ypos
    bounds upperLeft: (xpos@ypos) 
|
  setWidth: width height: height
    bounds bottomRight: ((self left+width)@(self top+height)) 
]
*
Methods Morph 'display'
  drawOn: aSurface
  (shape isNil) ifTrue: [
    shape <- SFMLRectangleShape new
  ].
  shape setPositionX: (bounds left) Y: (bounds top).
  shape setSizeX: (bounds width) Y: (bounds height).
  aSurface draw: shape
]
*
* Initialize Node to make sure the errorException is built.
*
! Node initialize
