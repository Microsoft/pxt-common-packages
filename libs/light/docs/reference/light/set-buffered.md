# set Buffered

Turn on or turn off saving (buffering) changes to the pixels.

```sig
light.pixels.setBuffered(false)
```
When you set pixel buffering on, changes to the pixels are saved and not shown until a **[show](/reference/light/show)** is used in the program.

Pixel buffering stays on until you turn it off again by using **[setBuffered]()** with `false` for the **on** parameter.

## Parameters

* **on**: a [boolean](/types/boolean) value saying whether pixel change buffering is on (`true`) or off (`false`).
>`true`: pixel buffering is on. Pixel changes don't appear until [show](reference/light/show) is used.<br/>
`false`: pixel buffering is off. Pixel changes show automatically when each change happens.

## Example

Set buffering of pixel changes so that the new colors all show at once.

```blocks
light.pixels.setBuffered(true)
light.pixels.setPixelColor(0, Colors.Blue)
light.pixels.setPixelColor(2, Colors.Red)
light.pixels.setPixelColor(4, Colors.Yellow)
light.pixels.show()
```
## See also

[show](/reference/light/show)

```package
light
```


