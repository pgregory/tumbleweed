RawClass MetaSFML Class MetaObject sfmlFFI
Class SFML Object
Class SFMLObject Object handle
Class SFMLWindow SFMLObject 
Class SFMLTexture SFMLObject
Class SFMLSprite SFMLObject
Class SFMLFont SFMLObject
Class SFMLText SFMLObject
Class SFMLEvent ExternalData 

Methods MetaSFML 'initialisation'
  sfmlFFI
    (sfmlFFI isNil) ifTrue: [
      sfmlFFI <- FFI new: 'modules/sfml/sfml'
    ].
    ^ sfmlFFI
]
*
Methods SFMLObject 'general'
  handle
    ^ handle
|
  handle: h
    handle <- h
]
*
Methods MetaSFMLWindow 'creation'
  new
  | newWindow args result |
  newWindow <- super new.
  args <- Array new: 4.
  args at: 1 put: 800; at: 2 put: 600; at: 3 put: 32; at: 4 put: 'Default Window'.
  result <- (SFML sfmlFFI) call: 'windowCreate' returning: #cObject taking: #(int int int string) as: args.
  newWindow handle: (result at: 1). 
  ^ newWindow
]
*
Methods SFMLWindow 'display'
  display
  | args |
  args <- Array new: 1.
  args at: 1 put: handle.
  (SFML sfmlFFI) call: 'windowDisplay' returning: #void taking: #(cObject) as: args
|
  clear
  | args |
  args <- Array new: 1.
  args at: 1 put: handle.
  (SFML sfmlFFI) call: 'windowClear' returning: #void taking: #(cObject) as: args
|
  draw: aDrawable
  | args |
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: (aDrawable handle).
  (SFML sfmlFFI) call: 'windowDraw' returning: #void taking: #(cObject cObject) as: args
|
  close
  | args |
  args <- Array new: 1.
  args at: 1 put: handle.
  (SFML sfmlFFI) call: 'windowClose' returning: #void taking: #(cObject) as: args
]
*
Methods SFMLEvent 'all'
  fields
    ^ #((type int)
        (mouseX int)
        (mouseY int)
        (keyCode int))
]
*
Methods SFMLWindow 'events'
  pollEvent
  | args result event |
  event <- SFMLEvent new.
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: event.
  result <- (SFML sfmlFFI) call: 'windowPollEvent' returning: #void taking: #(cObject externalOut) as: args.
  ^result at: 3
]
*
Methods MetaSFMLTexture 'creation'
  new 
  | newTexture args result |
  newTexture <- super new.
  args <- Array new: 0.
  result <- (SFML sfmlFFI) call: 'textureCreate' returning: #cObject taking: #() as: args.
  newTexture handle: (result at: 1).
  ^ newTexture
]
*
Methods SFMLTexture 'loading'
  loadFromFile: aString
  | args result |
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: aString.
  result <- (SFML sfmlFFI) call: 'textureLoadFromFile' returning: #int taking: #(cObject string) as: args.
  ^ (result at: 1)
]
*
Methods MetaSFMLSprite 'creation'
  new 
  | newSprite args result |
  newSprite <- super new.
  args <- Array new: 0.
  result <- (SFML sfmlFFI) call: 'spriteCreate' returning: #cObject taking: #() as: args.
  newSprite handle: (result at: 1).
  ^ newSprite
]
*
Methods SFMLSprite 'modifiation'
  setTexture: aTexture
  | args result |
  "Need to do validation on the aTexture argument"
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: (aTexture handle).
  result <- (SFML sfmlFFI) call: 'spriteSetTexture' returning: #void taking: #(cObject cObject) as: args
|
  setPositionX: xpos Y: ypos
  | args |
  args <- Array new: 3.
  args at: 1 put: handle; at: 2 put: xpos; at: 3 put: ypos.
  (SFML sfmlFFI) call: 'spriteSetPosition' returning: #void taking: #(cObject double double) as: args
|
  moveX: xoffset Y: yoffset
  | args |
  args <- Array new: 3.
  args at: 1 put: handle; at: 2 put: xoffset; at: 3 put: yoffset.
  (SFML sfmlFFI) call: 'spriteMove' returning: #void taking: #(cObject double double) as: args
|
  rotate: angle
  | args |
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: angle.
  (SFML sfmlFFI) call: 'spriteRotate' returning: #void taking: #(cObject double) as: args
]
*
Methods MetaSFMLFont 'creation'
  new 
  | newFont args result |
  newFont <- super new.
  args <- Array new: 0.
  result <- (SFML sfmlFFI) call: 'fontCreate' returning: #cObject taking: #() as: args.
  newFont handle: (result at: 1).
  ^ newFont
]
*
Methods SFMLFont 'loading'
  loadFromFile: aString
  | args result |
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: aString.
  result <- (SFML sfmlFFI) call: 'fontLoadFromFile' returning: #int taking: #(cObject string) as: args.
  ^ (result at: 1)
]
*
Methods MetaSFMLText 'creation'
  new 
  | newText args result |
  newText <- super new.
  args <- Array new: 0.
  result <- (SFML sfmlFFI) call: 'textCreate' returning: #cObject taking: #() as: args.
  newText handle: (result at: 1).
  ^ newText
]
*
Methods SFMLText 'modification'
  setString: aString
  | args |
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: aString.
  (SFML sfmlFFI) call: 'textSetString' returning: #void taking: #(cObject string) as: args.
|
  setFont: aFont
  | args |
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: (aFont handle).
  (SFML sfmlFFI) call: 'textSetFont' returning: #void taking: #(cObject cObject) as: args.
|
  setCharacterSize: aNumber
  | args |
  args <- Array new: 2.
  args at: 1 put: handle; at: 2 put: (aNumber floor).
  (SFML sfmlFFI) call: 'textSetCharacterSize' returning: #void taking: #(cObject int) as: args.
]
