# Photon

Photon is your pet light particle. 
It can move along the colored leds and color them. Photon can move forward or backward,
turn or change color. Use it to create awesome color animations!

## Reference

```cards
photon.forward(1)
photon.backward(1)
photon.flip()
photon.testForColor()
photon.setColor(100)
photon.all(100)
photon.setMode(PhotonMode.On)
photon.setPosition(0)
photon.position()
photon.setHeading(true)
photon.setVisible(true)
photon.clean()
```

## Advanced

```cards
photon.setStrip(lights.pixels)
```

## Photon and Logo

Photon is inspired from [LightLogo](http://joshburker.blogspot.com/2015/08/lightlogo-logo-programming-microworld.html) 
from Brian Silverman. It provides a microworld experience on top of any colored led strip.

```package
photon
```